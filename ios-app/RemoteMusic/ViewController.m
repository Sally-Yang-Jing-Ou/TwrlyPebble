//
//  ViewController.m
//  RemoteMusic
//
//  Created by Sally Yang Jing Ou on 2015-07-18.
//  Copyright (c) 2015 Sally Yang Jing Ou. All rights reserved.
//

#import "ViewController.h"
#import "GVMusicPlayerController.h"

@interface ViewController () <BTDropInViewControllerDelegate>
{
    BOOL check;
}
@property (weak, nonatomic) IBOutlet UIButton *PlayMusic;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    check = false;
    [self getBraintreeToken];
    // Do any additional setup after loading the view, typically from a nib.
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)playMusicAction {
    if (!check){
        MPMediaQuery *query = [MPMediaQuery songsQuery];
        [[GVMusicPlayerController sharedInstance] setQueueWithQuery:query];
        [[GVMusicPlayerController sharedInstance] play];
        check = true;
    } else {
        if ([GVMusicPlayerController sharedInstance].playbackState == MPMusicPlaybackStatePlaying) {
            [[GVMusicPlayerController sharedInstance] pause];
        } else {
            [[GVMusicPlayerController sharedInstance] play];
        }
    }
}

- (IBAction)pauseButtonPressed {
    if ([GVMusicPlayerController sharedInstance].playbackState == MPMusicPlaybackStatePlaying) {
        [[GVMusicPlayerController sharedInstance] pause];
    } else {
        [[GVMusicPlayerController sharedInstance] play];
    }
}

- (IBAction)prevButtonPressed {
    [[GVMusicPlayerController sharedInstance] skipToPreviousItem];
}

- (IBAction)nextButtonPressed {
    [[GVMusicPlayerController sharedInstance] skipToNextItem];
}

- (IBAction)makeCalls {
    NSString *phNo = @"+12266004998";
    NSURL *phoneUrl = [NSURL URLWithString:[NSString  stringWithFormat:@"tel://%@",phNo]];
    
    if ([[UIApplication sharedApplication] canOpenURL:phoneUrl]) {
        [[UIApplication sharedApplication] openURL:phoneUrl];
    } else
    {
        UIAlertView *calert = [[UIAlertView alloc]initWithTitle:@"Alert" message:@"Call facility is not available!!!" delegate:nil cancelButtonTitle:@"ok" otherButtonTitles:nil, nil];
        [calert show];
    }
}

- (void) getBraintreeToken  {
    NSURL *clientTokenServerURL = [NSURL URLWithString:@"http://agile-hamlet-5554.herokuapp.com/client_token"];
    NSMutableURLRequest *clientTokenRequest = [NSMutableURLRequest requestWithURL:clientTokenServerURL];
    [clientTokenRequest setValue:@"text/plain" forHTTPHeaderField:@"Accept"];
    
    [NSURLConnection sendAsynchronousRequest:clientTokenRequest
                                       queue:[NSOperationQueue mainQueue]
                           completionHandler:^(NSURLResponse *response, NSData *data, NSError *connectionError) {
                               self.clientToken = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
                               NSLog(@"%@", self.clientToken);
                               self.btnPay.enabled = TRUE;
                           }];
    
}

- (IBAction) postToRSS {
    NSURL *rssURL = [NSURL URLWithString:@"https://quiet-sands-6289.herokuapp.com/addtoxml"];
    
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:rssURL];
    
    [request setHTTPMethod:@"POST"];
    [request setValue:@"text/plain" forHTTPHeaderField:@"Accept"];
    [request setHTTPBody: [[NSString stringWithFormat:@"action=music-stop&userid=127876"] dataUsingEncoding:NSUTF8StringEncoding]];
    
    
    [NSURLConnection sendAsynchronousRequest:request
                                       queue:[NSOperationQueue mainQueue]
                           completionHandler:^(NSURLResponse *response, NSData *data, NSError *connectionError) {
                               
                               if (connectionError == nil && data != nil)  {
                                   UIAlertView *calert = [[UIAlertView alloc]initWithTitle:@"Success" message:@"If-This-Then-That: EMAIL Sent!" delegate:nil cancelButtonTitle:@"Ok" otherButtonTitles:nil, nil];
                                   [calert show];
                                   
                                   NSLog(@"success");
                                   
                               }    else    {
                                   NSLog(@"what");
                                }
                               
                           }];
}

#pragma mark call dropin
- (IBAction)callDropIn:(id)sender   {
    
    self.braintree = [Braintree braintreeWithClientToken:self.clientToken];
    
    BTDropInViewController *dropInViewController = [self.braintree dropInViewControllerWithDelegate:self];
    
    dropInViewController.navigationItem.leftBarButtonItem = [[UIBarButtonItem alloc]
                                                             initWithBarButtonSystemItem:UIBarButtonSystemItemCancel
                                                             target:self
                                                             action:@selector(userDidCancelPayment)];
    
    dropInViewController.summaryTitle = @"United Way Charity";
    dropInViewController.summaryDescription = @"Donate to this charity to gain a better soul";
    dropInViewController.displayAmount = @"$ 20.00";
    
    UINavigationController *navController = [[UINavigationController alloc] initWithRootViewController:dropInViewController];
    
    [self presentViewController:navController animated:YES completion:nil];
    
}

- (void)userDidCancelPayment    {
    [self dismissViewControllerAnimated:YES completion:nil];
    
}


#pragma mark receiving return from dropin
- (void) dropInViewController:(__unused BTDropInViewController *)viewController didSucceedWithPaymentMethod:(BTPaymentMethod *)paymentMethod    {
    [self postNonceToTheServer:paymentMethod.nonce];
    [self dismissViewControllerAnimated:YES completion:nil];
}

- (void) dropInViewControllerDidCancel:(BTDropInViewController *)viewController {
    [self dismissViewControllerAnimated:YES completion:nil];
}


- (void) postNonceToTheServer:(NSString *) nonce     {
    
    NSURL *paymentURL = [NSURL URLWithString:@"http://agile-hamlet-5554.herokuapp.com/payment-methods"];
    
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:paymentURL];
    
    [request setHTTPMethod:@"POST"];
    [request setValue:@"text/plain" forHTTPHeaderField:@"Accept"];
    [request setHTTPBody: [[NSString stringWithFormat:@"payment_method_nonce=%@&amount=20", nonce] dataUsingEncoding:NSUTF8StringEncoding]];
    
    
    [NSURLConnection sendAsynchronousRequest:request
                                       queue:[NSOperationQueue mainQueue]
                           completionHandler:^(NSURLResponse *response, NSData *data, NSError *connectionError) {
                               
                               if (connectionError == nil && data != nil)  {
                                   
                                   self.transactionID = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
                                   
                                   UIAlertView *alert = [[UIAlertView alloc] initWithTitle: @"Braintree returns"
                                                                                   message: self.transactionID
                                                                                  delegate:nil
                                                                         cancelButtonTitle:@" OK "
                                                                         otherButtonTitles:nil];
                                   [alert show];
                                   [self.btnPay setEnabled:FALSE];
                                   [self.btnPay setTitle:@" Thanks! " forState:UIControlStateNormal];
                                   
                                   
                               }else{
                                   
                                   UIAlertView *alert = [[UIAlertView alloc] initWithTitle: @"Braintree returns"
                                                                                   message: connectionError.localizedDescription
                                                                                  delegate:nil
                                                                         cancelButtonTitle:@" OK "
                                                                         otherButtonTitles:nil];
                                   [alert show];
                               }
                           }];
}

@end
