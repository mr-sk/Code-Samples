/* 
 * Copyright © Elijah Insua & Ben Sgro, 2009, 2010.
 */

#import "AStarNode.h"

@implementation AStarNode

/* Allow AStartNode.{property} access to all member properties. */
@synthesize weight;
@synthesize nodeId, cameFromNode;
@synthesize parentNode;


-(id) init;
{
    if (self == [super init])
    {
        weight       = 0;
        nodeId       = @"";
        cameFromNode = @"";
        parentNode   = nil;
    }
    
    return self;
}


-(id) initWithWeight:(int)nodeWeight andNodeId:(NSString *)nId andParentNodeId:(NSString *)parentId 
       andParentNode:(AStarNode *)parent
{
    if (self == [super init])
    {
        weight       = nodeWeight;
        nodeId       = nId;
        cameFromNode = parentId;
        parentNode   = parent;
    }
    
    return self;
}

@end
