/* 
 * Copyright © Elijah Insua & Ben Sgro, 2009, 2010.
 */

/**
 @title  The m42EntityManager class. 
 @detail This class is a singleton. It manages entities of any type storing them as id pointers. 
 
 */

@interface m42EntityManager : NSObject {
    /// Store all the entities here.
    NSMutableDictionary *mEntityDictionary;

    /// A unique Id tracked by the EntityManger. This can 
    /// be called upon to generate unique Id's for entities
    /// that you want stored in the Manager.
    uint_fast8_t mNextId;
    
    /// Keep a list of all the ID's we have here. This currently is NOT used.
    /// @Todo Possibly remove this.
    NSMutableArray *mIdSet;
}


+ (m42EntityManager *)mSharedInstance;

- (uint_fast8_t) returnUniqueId;
- (void) registerEntity:(id)entity byId:(uint_fast8_t)entityId;
- (id) getEntityFromId: (uint_fast8_t const) entityId;
- (void) removeEntity:(uint_fast8_t) entityId;
//- (id) getNextEntity;

@end
