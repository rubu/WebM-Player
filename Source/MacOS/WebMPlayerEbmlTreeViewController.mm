#import "WebMPlayerEbmlTreeViewController.h"
#import "EbmlTree.h"

@implementation WebMPLayerEbmlTreeViewController
{
    EbmlTree* _ebmlTree;
}

-(BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    return _ebmlTree != nil;
}

-(IBAction)show:(id)sender
{
    [self.view.window orderFront:nil];
}

-(void)setEbmlTree:(EbmlTree*)ebmlTree
{
    _ebmlTree  = ebmlTree;
    [(NSOutlineView*)self.view setDataSource:_ebmlTree];
    [(NSOutlineView*)self.view setDelegate:_ebmlTree];
}

@end
