/* 
 * Copyright © Elijah Insua & Ben Sgro, 2009, 2010.
 */

struct m42Telegram
{
    /// The id the entity/agent that send this telegram.
    uint_fast8_t mSenderId;
    
    /// The id of the entity/agent that is to receieve this telegram.
    uint_fast8_t mReceiverId;
    
    /// The message. They are enumerated in GameAgentMessageTypes.h
    uint_fast8_t mMessageId;
    
    /// Messages can either by dispatched immediately or delayed for a specified
    /// amount of time. If a delay is set its set here.
    uint64_t mDispatchTime;
    
    /// Any additional information.
    void *mExtraDS;
};