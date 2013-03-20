/* 
 * Copyright © Elijah Insua & Ben Sgro, 2009, 2010.
 */


#import "m42State.h"

@implementation m42State

/**
 Abstract init method. 
 Derived classes need to implement this method.
 
 @returns nil Not to be called
 */
- (id)init
{
    [self doesNotRecognizeSelector:_cmd];
    [self release];
    return nil;
}


/**
 Abstract enter method.
 
 This method is called when we first enter a state. It is never called
 again within the state.
 
 @param  agent An instance of the agent derived from m42Agent.
 */
- (void) enter:(id) agent
{
    [self doesNotRecognizeSelector:_cmd];
}


/**
 Abstract execute method.
 
 If the agent is in this state and update is called on the state machine, this
 state will kick off.
 
 @param  agent An instance of the agent derived from m42Agent.
 */
- (void) execute:(id) agent
{
    [self doesNotRecognizeSelector:_cmd];
}


/**
 Abstract exit method.
 
 This method is called when we leave a state. It is never called again
 within the state (Unless we enter, execute, exit it again).
 
 @param  agent An instance of the agent derived from m42Agent.
 */
- (void) exit:(id) agent
{
    [self doesNotRecognizeSelector:_cmd];
}


/**
 Abstract state machine handle message method.
 
 When the state machine is "updated" this method is called on any state
 that has been sent a message. Messages are dispatched from the m42MessageDispatcher.
 
 @param agent       An instance of the agent derived from m42Agent.
 @param m42Telegram See m42Telegram for additional details.
 @return boolean    To keep the compiler from throwing a warning.
 */
- (Boolean) onMessage:(id) agent telegram:(struct m42Telegram) tGram
{
    [self doesNotRecognizeSelector:_cmd];
    return NO; // Shut the compiler up!
}

/**
 Currently, derived classes don't implement their own dealloc. However, 
 they might want to if they have member properties. 
 
 @todo Look into making this abstract as well, to force a clear implementation.
 */
- (void)dealloc {
    [super dealloc];
}


@end
