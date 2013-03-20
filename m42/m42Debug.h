/* 
 * Copyright © Elijah Insua & Ben Sgro, 2009, 2010.
 */

/**
 
 
 @section Description
// NOTE:
// Both these functions are removed in release builds.
// DO NOT, I REPEAT, DO NOT ALLOC An OBJECT
// THAT YOU INTEND YOU USE ELSEWHERE IN EITHER OF THESE
// METHODS. kthxbye.

/*
 This toggles assertions. When defined, assertions are
 active in the code. When undefine, assertions are still
 defined in the code, but at compile time they are removed
 because they execute to nothing:
 
 usage:
      _m42assert(@"This is true!" && 1 != 1);
      Will cause the debugger to break.
 */


/**************************************
 Toggle this for assertions and logging
 *************************************/
#define M42_DEBUG

#ifdef M42_DEBUG
#define _m42assert(statement) \
    if (statement) { } \
    else \
    { \
        assert(statement); \
    } 
#else
    #define _m42assert(statement)
#endif


/*
 This toggles our logging facilitiy. I hate this code.
 
 usage:
     _m42log(CHAN_*, @"Here is a format @%", @"string");
*/

// Our available CHANs
#define CHAN_AUDIO     0x0001 
#define CHAN_RENDER    0x0002 
#define CHAN_GAME      0x0004
#define CHAN_MESSAGING 0x0008
#define CHAN_AI        0x0010
#define CHAN_CLOCK     0x0020
#define CHAN_MENU      0x0040
#define CHAN_STATE     0x0080

// Create a MASK (Include all the logging you WANT to happen)

// TURNS ON ALL LOGGING CHANNELS
#define CHAN_LEVEL     ( CHAN_AUDIO | CHAN_RENDER | CHAN_GAME | CHAN_MESSAGING | CHAN_AI | CHAN_CLOCK | CHAN_MENU | CHAN_STATE )

// TURNS ON MESSAGING ONLY
//#define CHAN_LEVEL CHAN_MESSAGING

// Our custom log wrapper
#ifdef M42_DEBUG
#define _m42log( chan, s, ... ) \
    if ( ( (chan & 0xffff) & CHAN_LEVEL ) != 0) \
    { \
        NSString *chanStr = [[[NSString alloc] init] autorelease]; \
        switch( chan ) \
        { \
            case CHAN_AUDIO: \
                chanStr = @"Audio"; \
            break; \
            case CHAN_RENDER: \
                chanStr = @"Render"; \
            break; \
            case CHAN_GAME: \
                chanStr = @"Game"; \
            break; \
            case CHAN_MESSAGING: \
                chanStr = @"Messaging"; \
            break; \
            case CHAN_AI: \
                chanStr = @"AI"; \
            break; \
            case CHAN_CLOCK: \
                chanStr = @"Clock"; \
            break; \
            case CHAN_MENU: \
                chanStr = @"Menu"; \
            break; \
            case CHAN_STATE: \
                chanStr = @"State"; \
            break; \
        } \
        NSLog( @"<%p %@:(%d)>[%@] %@", self, [[NSString stringWithUTF8String:__FILE__] lastPathComponent], __LINE__, chanStr, [NSString stringWithFormat:(s), ##__VA_ARGS__] ); \
        [chanStr release]; \
    }
#else
    #define _m42log( chan, s, ... )
#endif