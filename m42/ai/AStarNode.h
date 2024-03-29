/* 
 * Copyright © Elijah Insua & Ben Sgro, 2009, 2010.
 */
/* Our node class, used by the AStarSearch class.
   Contains a class with the following memeber
   properties. 
*/
@interface AStarNode : NSObject {
    
    /* The calculated weight of the node. */
    int weight;
    
    /* The node id - "node 1". */
    NSString *nodeId;
    
    /* The id of the parent node. */
    NSString *cameFromNode;
    
    /* A pointer to an instance of a AStarNode.
       This enables us to traverse up the parent chain
       to the base (starting) node and derive a path.
    */
    AStarNode *parentNode;
}

/* All private member properties are provided as properties. */
@property int weight;
@property (nonatomic, retain) NSString *nodeId, *cameFromNode;
@property (nonatomic, retain) AStarNode *parentNode;

-(id) init;
-(id) initWithWeight:(int)nodeWeight andNodeId:(NSString *)nId andParentNodeId:(NSString *)parentId 
       andParentNode:(AStarNode *)parent;

@end
