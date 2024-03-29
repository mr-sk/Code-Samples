/* 
 * Copyright © Elijah Insua & Ben Sgro, 2009, 2010.
 */

//#import <Cocoa/Cocoa.h>

#import "m42Graph.h"
//#import "m42Connection.h"
#import "AStarNode.h"

@interface AStarSearch : NSObject {

    m42Graph *nodeGraph;
    AStarNode *sourceNode;
    AStarNode *targetNode;
    AStarNode *pathByNode;
    int pathWeight;
}

@property (nonatomic, retain) m42Graph* nodeGraph;
@property (nonatomic, retain) AStarNode *sourceNode, *targetNode, *pathByNode;
@property int pathWeight;

-(id) init;
-(id) initWithGraph:(m42Graph *)graph andStartNode:(AStarNode *)startNode andEndNode:(AStarNode *)endNode;

-(void) search;
-(NSArray *) getPathToTarget;
-(int) getCostToTarget;
-(Boolean) doesNode:(AStarNode *)node existInTargetNodeSet:(NSMutableArray *)targetNodeSet;
-(AStarNode *) lowestCostNodeInArray:(NSMutableArray *) nodeSet;

@end
