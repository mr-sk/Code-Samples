//
//  LogController.h
//  dxLog
//
//  Created by sk on 1/20/10.
//  Copyright 2010 Ben Sgro aka Mr-sk. All rights reserved.
//

#import <Foundation/Foundation.h>

// Used for NSUserDefaults
#define NSU_USERNAME   @"username"
#define NSU_PASSWORD   @"password"
#define NSU_LOGLEVELID @"loglevelid"
#define NSU_LOGURL     @"logurl"
#define NSU_SINCE      @"since"
#define NSU_BODYSIZE   @"bodysize"

@interface LogController : NSObject {
    NSMutableData *responseData;
    NSArray *mLogTypes;
    NSTimer *mTimer;
    NSString *mUser;
    NSString *mPass;
    NSString *mSince;
    NSMutableString *mLogURL;
    NSString *mFinishedURL;
    NSMutableDictionary *mLogDict;
    int mLogLevelId;
    int mBodySize;
    
    IBOutlet NSScrollView *scrollView;
    IBOutlet NSTextView *textView;
    IBOutlet NSButton* myButton;
    IBOutlet NSButtonCell *myButtonCell;
    IBOutlet NSTextField *logURL, *logUser;
    IBOutlet NSSecureTextField *logPass;
    IBOutlet NSComboBox *comboBox;
    IBOutlet NSComboBoxCell *comboBoxCell;
    IBOutlet NSProgressIndicator *pIndicator;
    IBOutlet NSTextField *sinceField, *bodysizeField;
}

@property(nonatomic, retain) IBOutlet NSScrollView *scrollView;
@property(nonatomic, retain) IBOutlet NSTextView *textView;
@property(nonatomic, retain) IBOutlet NSTextField *logURL, *logUser;
@property(nonatomic, retain) IBOutlet NSSecureTextField *logPass;
@property(nonatomic, retain) IBOutlet NSButton *myButton;
@property(nonatomic, retain) IBOutlet NSButtonCell *myButtonCell;
@property(nonatomic, retain) IBOutlet NSComboBox *comboBox;
@property(nonatomic, retain) IBOutlet NSComboBoxCell *comboBoxCell;
@property(nonatomic, retain) IBOutlet NSProgressIndicator *pIndicator;
@property(nonatomic, retain) IBOutlet NSTextField *sinceField, *bodysizeField;

- (IBAction) update:(id) sender;
- (void)load;
- (void)firstRun;
- (Boolean) showRecordedLog:(NSString *) logLevelString;
- (int) mapLogLevelToInt:(NSString *) logLevelString;


@end
