#import "WebMPlayerDelegate.h"
#import "WebMPlayerOpenGLView.h"

@interface WebMPlayerDelegate ()

@property (weak) IBOutlet NSWindow *window;
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
        NSArray* subviews = ((NSView*)_window.contentView).subviews;
        [(WebMPlayerOpenGLView*)subviews[0] playFile:fileURL];
        [[NSDocumentController sharedDocumentController] noteNewRecentDocumentURL:[NSURL fileURLWithPath:fileURL.path]];
    }
}

@end
