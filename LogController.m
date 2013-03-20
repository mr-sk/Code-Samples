//
//  LogController.m
//  dxLog
//
//  Created by sk on 1/20/10.
//  Copyright 2010 Ben Sgro aka Mr-sk. All rights reserved.
//

#import "LogController.h"
#import "Base64Category.h"
#import "ListParser.h"
#import "NameValueParser.h"

@implementation LogController
@synthesize scrollView;
@synthesize textView;
@synthesize logURL, logUser;
@synthesize logPass;
@synthesize myButton;
@synthesize myButtonCell;
@synthesize comboBox;
@synthesize comboBoxCell;
@synthesize pIndicator;
@synthesize sinceField, bodysizeField;

- (id) init
{
    if (self == [super init])
    {
        /**
         * This is used to clear out my test data from NSUserDefaults.
         * We should never build for 'release' with this in. 
         */
        /*
        NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
        [defaults removePersistentDomainForName:@"com.yourcompany.dxlog"];
        [defaults synchronize];
        */
        
        mLogTypes = [[NSArray alloc] initWithObjects:@"DEBUG", @"INFO", @"WARNING", @"ERROR", @"SEVERE", nil];
        mLogDict  = [[NSMutableDictionary alloc] initWithCapacity:100];
        
        mUser = nil;
        mPass = nil;
        
        NSUserDefaults *userPreferences = [NSUserDefaults standardUserDefaults];
        NSString *initUser = [userPreferences stringForKey:NSU_USERNAME];
        if (initUser == nil)
        {
            [userPreferences setObject:@"" forKey:NSU_USERNAME];
        }
        NSString *initPass = [userPreferences stringForKey:NSU_PASSWORD];
        if (initPass == nil)
        {
            [userPreferences setObject:@"" forKey:NSU_PASSWORD];
        }
        
        if ([userPreferences stringForKey:NSU_LOGURL] == nil)
        {
            [userPreferences setObject:@"" forKey:NSU_LOGURL];
        }

        if ([userPreferences stringForKey:NSU_SINCE] == nil)
        {
            [userPreferences setObject:@"" forKey:NSU_SINCE];
        }
        
        mLogLevelId = [userPreferences integerForKey:NSU_LOGLEVELID];
        [userPreferences setInteger:mLogLevelId forKey:NSU_LOGLEVELID];

        mBodySize = [userPreferences integerForKey:NSU_BODYSIZE];
        if (mBodySize == 0) mBodySize = 200; // The default
        [userPreferences setInteger:mBodySize forKey:NSU_BODYSIZE];
        
        [userPreferences synchronize];
    }

    return self;
}

-(void)awakeFromNib
{
    NSUserDefaults *userPreferences = [NSUserDefaults standardUserDefaults];
    
    [logURL setStringValue:[userPreferences stringForKey:NSU_LOGURL]];
    [logUser setStringValue:[userPreferences stringForKey:NSU_USERNAME]];
    [logPass setStringValue:[userPreferences stringForKey:NSU_PASSWORD]];
    [sinceField setStringValue:[userPreferences stringForKey:NSU_SINCE]];
    [bodysizeField setStringValue:[userPreferences stringForKey:NSU_BODYSIZE]];
    [comboBox selectItemAtIndex:[userPreferences integerForKey:NSU_LOGLEVELID]];
}

- (IBAction) update:(id) sender
{
    mUser =  [logUser stringValue];
    mPass =  [logPass stringValue];
    mLogURL = [[NSMutableString alloc] initWithString:[logURL stringValue]];
    mSince  = [sinceField stringValue];
    mBodySize = [bodysizeField intValue];
    
    if ([mLogURL isEqual:@""])
    {
        [textView setString:@"Log URL is required"];
        return;
    }
    
    if ([mUser isEqual:@""] || [mPass isEqual:@""])
    {
        [textView setString:@"Username and Password required"];
        return;
    }
    
    if ([mSince isEqual:@""])
    {
        [textView setString:@"Fetch From Required"];
        return;
    }
    
    if (mBodySize <= 0)
    {
        [textView setString:@"Body Size Required"];
        return;
    }
        
    NSUserDefaults *userPreferences = [NSUserDefaults standardUserDefaults];
    
    [userPreferences setObject:mUser forKey:NSU_USERNAME];
    [userPreferences setObject:mPass forKey:NSU_PASSWORD];
    [userPreferences setObject:mLogURL forKey:NSU_LOGURL];
    [userPreferences setObject:mSince forKey:NSU_SINCE];
    [userPreferences setInteger:mLogLevelId forKey:NSU_LOGLEVELID];
    [userPreferences setInteger:mBodySize forKey:NSU_BODYSIZE];
    
    if ([[myButtonCell title] isEqualToString:@"Start"])
    {
        [myButtonCell setTitle:@"Stop"];  

        // Remove all the objects from the mLogDict
        // Clear the view
        [mLogDict removeAllObjects];
        [textView setString:@""]; // Clear the view
        
        mTimer = [NSTimer scheduledTimerWithTimeInterval:10
                                                  target:self 
                                                selector:@selector(load) 
                                                userInfo:nil 
                                                 repeats:YES];  
        [self firstRun];
    }
    else // They pressed on 'Stop' 
    {
        [myButtonCell setTitle:@"Start"];

        [mTimer invalidate];
        mTimer = nil;
    }
}

- (void)firstRun
{
    // Prep the URL
    // Kill the trailing slash (if any) we'll add it back later.
    NSString *urlString;
    if([mLogURL characterAtIndex:[mLogURL length]-1] == '/')
    {
        urlString = [mLogURL substringToIndex:[mLogURL length] - 1];        
    }
    else 
    {
        urlString = [[[NSString alloc] initWithString:mLogURL] autorelease];
    }
    
    NSString *sinceString = [mSince stringByReplacingOccurrencesOfString:@" " withString:@"%20"];
    mFinishedURL = [[[NSString alloc] initWithFormat:@"%@/records?since=%@", urlString, sinceString] autorelease];
    
    [self load];
}

- (void)load {    
    [pIndicator startAnimation:self];
    [pIndicator setHidden:NO];

    NSURL *myURL = [NSURL URLWithString:mFinishedURL];
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:myURL
                                             cachePolicy:NSURLRequestReloadIgnoringLocalCacheData
                                         timeoutInterval:60];

    // Compute the base64 string and remove newline from the NSTask
    NSString *string = [[[NSString alloc] initWithFormat:@"%@:%@", mUser, mPass] autorelease];
    NSString *tmpB64String = [[[NSString alloc] initWithFormat:@"Basic %@", [string _base64Encoding:string]] autorelease];
    NSString *b64String = [tmpB64String stringByReplacingOccurrencesOfString:@"\n" withString:@""];
    
    // Add the Authorization header and kick-off the request
    [request addValue:b64String forHTTPHeaderField:@"Authorization"];
    [[NSURLConnection alloc] initWithRequest:request delegate:self];
}

- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response {
    responseData = [[NSMutableData alloc] init];
}

- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data {
    [responseData appendData:data];
}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error {
    [connection release];
    [textView setString:@"Unable to fetch data"];
    
    [mTimer invalidate];
    mTimer = nil;
}

- (void)connectionDidFinishLoading:(NSURLConnection *)connection 
{
    [pIndicator stopAnimation:self];
    [pIndicator setHidden:YES];
    
    NSLog(@"Succeeded! Received %d bytes of data",[responseData
                                                   length]);
    NSString *txt = [[[NSString alloc] initWithData:responseData encoding: NSASCIIStringEncoding] autorelease];
    NSLog(@"%@", txt);
        
    ListParser *parser = [ListParser parser];
    [parser addAttributeName:@"id"]; //1
    [parser addFieldName:@"ReceivedTimeStamp"]; //4
	[parser addFieldName:@"LevelId"]; // 2
	[parser addFieldName:@"Message"]; // 3
	[parser parseData:[txt dataUsingEncoding:NSUTF8StringEncoding]];

    NSArray *list = [[NSArray alloc] initWithArray:[parser list]];
    
    if ([list count] == 0)
    {
        // Don't keep updating the screen with 'No data' spam.
        NSString *t = [textView string];
        if ([t isEqual:@"No data\n"])
        {
            return;
        }
        
        NSString *tmpStr = [[NSString alloc] initWithFormat:@"\n%@", [textView string]];              
        [textView setString:[@"No data" stringByAppendingString:tmpStr]];
        [tmpStr release];
        
        return;
    }
    
    NSEnumerator *e = [list objectEnumerator];
    NSString *object;
    
    NSString *levelString;
    NSString *timeString;
    NSString *messageString;
    
    Boolean ignoreLog = NO;
    int count = 1;
    int logId = 0;
    while (object = [e nextObject]) 
    {
        // This is for log level filtering
        if (ignoreLog)
        {
            if (count == 5)
            {
                count = 1;
                ignoreLog = NO;
            }
            if (count == 4) ++count;
            if (count == 3) ++count;
            if (count == 2) ++count;
        }
        else 
        {
            if (count == 1)
            {
                // This is the logId (the index to the dictionary - aka uniqueness)
                logId = [object intValue];
                ++count;
            }
            else if (count == 2)
            {
                // This is the level id
                levelString = [[[NSString alloc] initWithString:object] autorelease];
                if (![self showRecordedLog:levelString])
                {
                    ignoreLog = YES;
                }
                ++count;
            }
            else if (count == 3)
            {
                // This is the message
                int length = [object length];
                if (length > mBodySize)
                {
                    messageString = [[[NSString alloc] initWithString:[object substringToIndex:mBodySize]] autorelease];
                }
                else
                {
                    messageString = [[[NSString alloc] initWithString:[object substringToIndex:length]] autorelease];
                }
                ++count;
            }
            else
            {                
                // This is the timestamp
                timeString = [[NSString alloc] initWithString:object];
                NSString *formatString = [[NSString alloc] initWithFormat:@"[%@] [%@] %@", timeString, levelString,
                                          messageString];
             
                NSString *logIdAsString = [NSString stringWithFormat:@"%d", logId];
                if (![mLogDict objectForKey:logIdAsString])
                {
                    NSLog(@"never seen this:%@", logIdAsString);
                    
                    // Add to the dictionary & display
                    [mLogDict setObject:formatString forKey:logIdAsString];
                    
                    NSString *tmpStr = [[NSString alloc] initWithFormat:@"\n%@", [textView string]];              
                    [textView setString:[formatString stringByAppendingString:tmpStr]];                    
                    [tmpStr release];
                }
                
                [timeString release];
                [formatString release];

                count = 1;
            }
        }
    }  

    [connection release];
}

- (NSInteger)numberOfItemsInComboBoxCell:(NSComboBoxCell *)aComboBoxCell
{
    return 5;
}

- (id)comboBoxCell:(NSComboBoxCell *)aComboBoxCell objectValueForItemAtIndex:(NSInteger)index
{
    mLogLevelId = index;
    return [mLogTypes objectAtIndex:index];
}

- (Boolean) showRecordedLog:(NSString *) logLevelString
{
    if ([self mapLogLevelToInt:logLevelString] >= [self mapLogLevelToInt:[mLogTypes objectAtIndex:mLogLevelId]])
    {
        return YES;
    }
    return NO;
}

- (int) mapLogLevelToInt:(NSString *) logLevelString
{
    if ([logLevelString isEqual:@"DEBUG"])  return 1;
    if ([logLevelString isEqual:@"INFO"])   return 2;
    if ([logLevelString isEqual:@"WARN"])   return 3;
    if ([logLevelString isEqual:@"ERROR"])  return 4;
    if ([logLevelString isEqual:@"SEVERE"]) return 5;
    
    return -1;
}

@end
