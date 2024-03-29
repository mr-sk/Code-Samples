/* 
 * Copyright © Elijah Insua & Ben Sgro, 2009, 2010.
 */


#import "m42EntityManager.h"
#import "m42Debug.h"

static m42EntityManager *mSharedInstance = nil;

@implementation m42EntityManager

/**
 The singleton instance. This is the obj-c singleton pattern verbatim.
 
 @return mSharedInstance If class is nil (not alloc/inited), alloc/init it, set its instance
                         to the static mSharedInstance variable and return it.
 */
+ (m42EntityManager *)mSharedInstance
{
    @synchronized(self)
    {
        if (mSharedInstance == nil)
        {
            mSharedInstance = [[m42EntityManager alloc] init];
        }
    }
    return mSharedInstance;
}


/**
 Init over-ride. Setup our EntityDictionary, IdArray and set the nextId to zero.
 
 @returns self An instance of the class.
 */
- (id) init
{
    if (self == [super init])
    {
        mEntityDictionary = [[NSMutableDictionary alloc] initWithCapacity:4];
        mIdSet            = [[NSMutableArray alloc] initWithCapacity:4];
        mNextId           = 0;
    }
    
    return self;
}


/*
 When called we use post increment to return the current Id
 and increment it for the next call. 
 
 @Todo Are we even using this? If not, remove it and its member properties.
*/
- (uint_fast8_t) returnUniqueId
{
    return mNextId++;
}


/*
 Store the entitiy in the Manager.
*/
- (void) registerEntity:(id)entity byId:(uint_fast8_t)entityId
{
    _m42assert(entity);
    
    // Convert uint_fast8_t to NSInteger
    NSNumber *convertedEntityId = [[NSNumber alloc] initWithInt:entityId];
    _m42assert( (entity != [mEntityDictionary objectForKey:convertedEntityId]) );
    
    [mEntityDictionary setObject:entity forKey:convertedEntityId];

    [convertedEntityId release];
}


/* 
 Retrieve an entity by Id.
*/
- (id) getEntityFromId: (uint_fast8_t const) entityId
{    
    NSNumber *convertedEntityId = [[NSNumber alloc] initWithInt:entityId];
    _m42assert( (nil != [mEntityDictionary objectForKey:convertedEntityId]) );
        
    id tmpEntity = [mEntityDictionary objectForKey:convertedEntityId];
    [convertedEntityId release];

    return tmpEntity;
}


/*
 Remove an entity provided by the entityId
*/
- (void) removeEntity:(uint_fast8_t) entityId
{
    NSNumber *convertedEntityId = [[NSNumber alloc] initWithInt:entityId];
    _m42assert( (nil != [mEntityDictionary objectForKey:convertedEntityId]) );
    
    [mEntityDictionary removeObjectForKey:convertedEntityId];

    [convertedEntityId release];
}

/*
- (id) getNextEntity
{
    for (id key in mEntityDictionary) 
    {
        NSLog(@"key: %@, value: %@", key, [mEntityDictionary objectForKey:key]);
    }
}
 */

@end
