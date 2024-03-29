/* 
 * Copyright © Elijah Insua & Ben Sgro, 2009, 2010.
 */


#import "m42MessageDispatcher.h"

static m42MessageDispatcher *mSharedInstance = nil;
static m42Time              *mTimer          = nil;

@implementation m42MessageDispatcher

/**
 The m42MessageDispatcher class is a singleton.
 
 @return self An instance of the class.
 */
+ (m42MessageDispatcher *)mSharedInstance
{
    @synchronized(self)
    {
        if (mSharedInstance == nil)
        {
            mSharedInstance = [[m42MessageDispatcher alloc] init];
            mTimer = [[m42Time alloc] initWithCurrentTime];
        }
    }
    return mSharedInstance;
}


/**
 @private
 
 This method attempts to see if the agent provided is willing to handle the Telegram.
 If so, it will be dispatched to them via the state machine.
 
 @param m42Agent    An m42Agent or derived agent type.
 @param m42Telegram An m42Telegram containing a bunch of data for the agent.
 */
- (void) sendToHandler:(m42Agent *)agent telegram:(struct m42Telegram)tGram
{
    if (![agent handleMessage: tGram])
    {
        _m42log(CHAN_MESSAGING, @"Agent cannot handle message");
    }
}


/**
 @public
 
 Called when we are attempting to dispatch a message. This is typically called directly by game
 specific code.
 
 @param uint64_t     The amount of time to wait until dispatching the message. SEND_NOW is the same as 0.0 seconds
                     and will dispatch the message immediately.
 @param uint_fast8_t The ID of the sender - this is the agent dispatching the message.
 @param uint_fast8_t The ID of the receiver.
 @param uint_fast8_t A numberical representation of a message. These messages are typically implemented as an enumeration
                     or list of defines. Agents typically handle the messages.
 @param void         A void pointer to supply any additional data.
 */
- (void) dispatchMessage:(uint64_t)delay 
                  sender:(uint_fast8_t)s
                receiver:(uint_fast8_t)r
                 message:(uint_fast8_t)msg
                   extra:(void *)extraDS
{
    m42EntityManager *eManager = [m42EntityManager mSharedInstance];
    id receivingEntity = [eManager getEntityFromId:r];
    
    if (receivingEntity == nil)
    {
        _m42log(CHAN_MESSAGING, @"No receiver with id %d", r);
    }
    
    struct m42Telegram telegram = {r, s, msg, delay, extraDS};

    // No delay, dispatch the message immediately. 
    if (delay <= 0.0)
    {
        _m42log(CHAN_MESSAGING, @"Telegram dispatched by %d to %d of message %d", s,r,msg);
        [self sendToHandler:receivingEntity telegram:telegram];
    }
    else 
    {            
        uint64_t currentTime = [m42Time getCurrentTime];
        
        telegram.mDispatchTime = currentTime + delay;
        
        mPriorityQueue.insert(telegram);

        _m42log(CHAN_MESSAGING, 
                @"Delayed Telegram added to PQ by %d to %d of message %d and dispatchTime of %llu (currentTime: %llu)",
                s,r,msg, telegram.mDispatchTime, currentTime);
    }
    
}


/**
 @public 
 
 Attempt to dispatch any delayed messages. We look into the priority queue for any messages
 that have a dispatch time less than the current time and that have a dispatch time greater than zero.
 
 If so, we grab the first message (the highest priority) and send it on its way, removing it after
 we've dispatched it.
 
 */
- (void) dispatchDelayedMessages;
{
    uint64_t currentTime = [m42Time getCurrentTime]; 
    _m42log(CHAN_MESSAGING, @"dispatchDelayedMessages: currentTime = %llu", currentTime);
    
    // *Peak* into the PQ and see if there are any Telegrams we need to handle.
    while (!mPriorityQueue.empty()
           && (mPriorityQueue.begin()->mDispatchTime < currentTime)
           && (mPriorityQueue.begin()->mDispatchTime > 0))
    {
        // The PQ is sorted (DUH niga, its a PQ) so grab the first
        // Telegram and process it.
        const m42Telegram& telegram = *mPriorityQueue.begin();
        
        // Find out WHOM this telegram is for.
        m42EntityManager *eManager = [m42EntityManager mSharedInstance];
        id receivingEntity = [eManager getEntityFromId:telegram.mReceiverId];
        
        // Dispatch the Telegram.
        [self sendToHandler:receivingEntity telegram:telegram];
        
        // Remove this Telegram from the PQ.
        mPriorityQueue.erase(mPriorityQueue.begin());
        
        _m42log(CHAN_MESSAGING, @"Delayed Message Dispatched");
    }
}

@end