/* 
 * Copyright © Elijah Insua & Ben Sgro, 2009, 2010.
 */

/**
 @title  An abstract state class. 
 @detail When creating a state used by the state machine, you must implement all the methods of this class. 
         All states will extend this class.
 
 Derived classes will typically implement these methods with a specific type instead of an (id). 
 Example: @code - (void) enter:(GameSpecificType *) agent; @endcode
 */
#import "m42TelegramStruct.h"

@interface m42State : NSObject { }

- (id) init;

- (void) enter:(id) agent;
- (void) execute:(id) agent;
- (void) exit:(id) agent;
- (Boolean) onMessage:(id) agent telegram:(struct m42Telegram) tGram;

- (void) dealloc;
@end
