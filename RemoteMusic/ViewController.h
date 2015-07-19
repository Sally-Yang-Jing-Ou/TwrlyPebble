//
//  ViewController.h
//  RemoteMusic
//
//  Created by Sally Yang Jing Ou on 2015-07-18.
//  Copyright (c) 2015 Sally Yang Jing Ou. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <Braintree/Braintree.h>

@import MediaPlayer;

@interface ViewController : UIViewController


@property(nonatomic,strong) Braintree *braintree;
@property(nonatomic,strong) NSString *clientToken;
@property(nonatomic,strong) NSString *transactionID;
@property (weak, nonatomic) IBOutlet UIButton *btnPay;

- (IBAction)callDropIn:(id)sender;
- (void) playMusicAction;

@end

