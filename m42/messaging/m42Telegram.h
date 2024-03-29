/* 
 * Copyright © Elijah Insua & Ben Sgro, 2009, 2010.
 */


#import <iostream>
#import <set>

#import "m42TelegramStruct.h"

/// Telegrams are stored in the prioriey queue. We will need to sort
/// telegrams by time priority. Times must be smaller than TELEGRAM_SMALLEST_DELAY
/// to be considered unique.
const double TELEGRAM_SMALLEST_DELAY = 0.25;

/// In order to utilize the CPP Priority queue, we have to provide
/// our own implementation to the following operators: == and <.
/// So this is them!

/**
 Provide our own implementation of the double equals ("==") operator for the PQ.
 
 @param m42Telegram Pointer to the first telegram for comparision.
 @param m42Telegram Pointer to the second telegram for comparision.
 
 Dereference each telegram using dot (".") notation and:
 -# Find the delta between the two telegram dispatch times and check if its less than
    the smallest delay (0.25).
 -# Is the sender Id's the same?
 -# Is the receiver Id's the same?
 -# Are both messagesId's the same?
 
 @return bool TRUE if all the conditions match, FALSE otherwise.
 */
inline bool operator==(const m42Telegram& t1, const m42Telegram& t2)
{
    return ( fabs(t1.mDispatchTime-t2.mDispatchTime) < TELEGRAM_SMALLEST_DELAY)
    && (t1.mSenderId == t2.mSenderId)        
    && (t1.mReceiverId == t2.mReceiverId)    
    && (t1.mMessageId == t2.mMessageId);
}


/**
 Provide our own implemenation of the less than ("<") operator.
 
 @param m42Telegram Pointer to the first telegram for comparision.
 @param m42Telegram Pointer to the second telegram for comparision.
 
 @reurn bool FALSE If both telegrams are the same.
 */
inline bool operator<(const m42Telegram& t1, const m42Telegram& t2)
{
    if (t1 == t2)
    {
        return false;
    }
    
    else
    {
        return  (t1.mDispatchTime < t2.mDispatchTime);
    }
}


/**
 Handy helper function for dereferencing the mExtraDS field of the m42Telegram to the required type.
 */
template <class T>
inline T DereferenceToType(void* p)
{
    return *(T*)(p);
}