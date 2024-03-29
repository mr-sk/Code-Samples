/* 
 * Copyright © Elijah Insua & Ben Sgro, 2009, 2010.
 */


#import "m42Telegram.h"
#import "m42Agent.h"
#import "m42EntityManager.h"
#import "m42Telegram.h"
#import "m42Debug.h"
#import "m42Time.h"

#include <iostream>
#include <set>

using namespace std;

// Send the message immediately. 
const double NO_WAIT = 0.0f;

// No additional info in this message.
const int    NO_INFO = 0;

/**
 @title  The MessageDispatcher class. 
 @detail This class is a singleton. It dispatches messages to m42Agents and derived agent types.
 The MessageDispatcher supports two actions on a Telgram:
 -# Immediate dispatch - messages are sent as soon as they are recieved by the MessageDispatcher.
 -# Delayed dispatch - messages are placed in a C++ PriorityQueue and sorted on the member property mDispatchTime.
 
 Messages are dispatched using the 
 @code 
 - (void) dispatchMessage:(double)delay 
                   sender:(uint_fast8_t)s
                 receiver:(uint_fast8_t)r
                  message:(uint_fast8_t)msg
                    extra:(void *)extraDS; 
 @endcode 
 
 method, where delay can be set to NO_WAIT or any double value and extra can be set to any optional data structure 
 or NO_INFO.
 
 @warning If you include this file in your class, it needs to be a .mm, since its C++. 
 */
@interface m42MessageDispatcher : NSObject {
    std::set<m42Telegram> mPriorityQueue;
}

+ (m42MessageDispatcher *)mSharedInstance;

- (void) sendToHandler:(m42Agent *)agent telegram:(struct m42Telegram)tGram;

- (void) dispatchMessage:(uint64_t)delay 
                  sender:(uint_fast8_t)s
                receiver:(uint_fast8_t)r
                 message:(uint_fast8_t)msg
                   extra:(void *)extraDS;

- (void) dispatchDelayedMessages;


@end
