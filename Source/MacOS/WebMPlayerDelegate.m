#import "WebMPlayerDelegate.h"
#import "WebMPlayerOpenGLView.h"

@interface WebMPlayerDelegate ()

@property (weak) IBOutlet NSWindow *playerWindow;
@property (weak) IBOutlet NSWindow *ebmlTreeWindow;
@end

@implementation WebMPlayerDelegate
{
    NSURL* _openFileBaseURL;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
}


- (void)applicationWillTerminate:(NSNotification *)aNotification
{
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return YES;
}

-(BOOL)application:(NSApplication *)sender openFile:(NSString*)filename;
{
    if ([filename.pathExtension isEqualToString:@"webm"] == YES)
    {
        [self openFile:[NSURL fileURLWithPath:filename]];
        return YES;
    }
    return NO;
}

-(void)openFile:(NSURL*)fileURL
{
    NSArray* subviews = ((NSView*)_playerWindow.contentView).subviews;
    [(WebMPlayerOpenGLView*)subviews[0] playFile:fileURL];
    [[NSDocumentController sharedDocumentController] noteNewRecentDocumentURL:[NSURL fileURLWithPath:fileURL.path]];
}

- (IBAction)openDocument:(id)sender
{
    NSOpenPanel* openFilePanel = [[NSOpenPanel alloc] init];
    openFilePanel.allowedFileTypes = [NSArray arrayWithObject:@"webm"];
    openFilePanel.canChooseDirectories = NO;
    openFilePanel.canChooseFiles = YES;
    openFilePanel.allowsMultipleSelection = NO;
    if (_openFileBaseURL)
    {
        openFilePanel.directoryURL = _openFileBaseURL;
    }
    NSModalResponse response = [openFilePanel runModal];
    if (response == NSModalResponseOK)
    {
        NSURL* fileURL = openFilePanel.URLs[0];
        _openFileBaseURL = [fileURL URLByDeletingLastPathComponent];
        [self openFile:fileURL];
    }
}

@end
