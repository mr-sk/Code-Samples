/* 
 * Copyright © Elijah Insua & Ben Sgro, 2009, 2010.
 */


#import "m42State.h"
#import "m42TelegramStruct.h"
#import "m42Debug.h"

/**
 @title  Our state machine implementation.
 @detail The state machine has two main components, the currentState and the globalState. The global
         state provides us the ability to uniformly provide implementations across all of an agents states.
         An agent can transition between states to perform the desired functionality, but the globalState
         can perform actions that are needed across all states, such as logging between state changes, playing
         a sound, update member properties and so on.
 
         GlobalStates can also handle messages. So a state that doesn't handle a message can have the globalState
         be a 'catch-all' for messages (if it wants too - it doesn't have to handle them at all). Unhandled messages
         do not get dispatched.
 
 @todo   Right now we only execute-hook. Add ability to pre/post-hook state transitions, via globalState enter
         and exit methods.
 */
@interface m42StateMachine : NSObject {

    /// The current state we are in.
    m42State *mCurrentState;

    /// The previous state.
    m42State *mPreviousState;
    
    /// Called when the state machine is updated.
    m42State *mGlobalState;
    
    /// The agent that owns this instance
    id mOwner;
}

// Properties.
@property (nonatomic, retain) m42State *mCurrentState, *mPreviousState, *mGlobalState;
@property (nonatomic, retain) id mOwner;

// Methods.
- (m42StateMachine *) init;
- (m42StateMachine *) initWithOwner:(id)agent;

- (void) update;
- (Boolean) handleMessage:(struct m42Telegram)telegram;
- (void) changeState:(m42State *) newState;
- (void) revertToPreviousState;
- (Boolean) isInState:(m42State *) st;

@end
