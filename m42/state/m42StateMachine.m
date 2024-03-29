/* 
 * Copyright © Elijah Insua & Ben Sgro, 2009, 2010.
 */


#import "m42StateMachine.h"

@implementation m42StateMachine

@synthesize mCurrentState, mPreviousState, mGlobalState;
@synthesize mOwner;


/**
 init override - set the state properties to nil.
 
 @return The class instance. 
 */
- (id) init
{
    if (self == [super init])
    {
        self.mCurrentState  = nil;
        self.mPreviousState = nil;
        self.mGlobalState   = nil;
    }
    
    return self;
}


/**
 Create a state machine and set to the owner to the creator.
 We use id because it can be any user define class that owns
 this instance.
 
 @param  agent An agent is derived from the m42Agent class.
 @return       The class instance.
 */
- (m42StateMachine *) initWithOwner:(id)agent
{
    if (self == [super init])
    {
        self.mOwner = agent;
    }
    
    return self;
}


/**
 Call when we want to update the State Machine. We first check to see
 if a globalState is setup and if so, we call its execute method. We
 then check the currentState and if its setup, we call its execute method.
 */
- (void) update;
{
    // If a global state exists, call its execute method.
    if (self.mGlobalState)
    {
        [self.mGlobalState execute:self.mOwner];
    }
    
    // Call the current states execute
    if (self.mCurrentState)
    {
        [self.mCurrentState execute:self.mOwner];
    }
}


/**
 Checks to see if the currentState has the ability to handle the message being sent. 
 We check the currentState first and then the globalState.
 
 @param m42Telegram A packed telegram.
 @return            TRUE if the global or local state can handle the message.
                    FALSE if either cannot.
 */
- (Boolean) handleMessage:(struct m42Telegram)tGram
{
    // Can we handle this in the current state?
    if (mCurrentState && [mCurrentState onMessage:mOwner telegram:tGram])
    {
        return YES;
    }
    
    // Can the global state handle this?
    if (mGlobalState && [mGlobalState onMessage:mOwner telegram:tGram])
    {
        return YES;
    }
    
    return NO;
}



/**
 When changing a state, we:
  -# Exit the current state
  -# Enter the new state
 
 We don't call execute: - that get's called when the state machine gets updated.

 @param m42State The state to enter.
 */
- (void) changeState:(m42State *) newState
{
    // If the new state we transition to is nil/false, throw an assertion.
    _m42assert(newState);    

    self.mPreviousState = self.mCurrentState;
    
    // Exit the current state.
    [self.mCurrentState exit:mOwner];
    
    // Update the state
    self.mCurrentState = newState;

    // Call the entry method for the new state
    [self.mCurrentState enter:mOwner];
}


/**
 Rollback the current state to the previous one. 
 
 */
- (void) revertToPreviousState
{
    [self changeState:self.mPreviousState];
}


/**
 Returns true if 'st' is the state we are currently in. 
 
 @param m42State A state to compare against the current one.
 */
- (Boolean) isInState:(m42State *) st
{    
    return [st isEqual:mCurrentState];
}

@end