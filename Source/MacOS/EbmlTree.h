#pragma once

#import <Cocoa/Cocoa.h>

#include "../EbmlParser.h"

@interface EbmlTree : NSObject<NSOutlineViewDataSource, NSOutlineViewDelegate>

-(instancetype)initWithEbmlDocument:(const EbmlDocument*)ebmlDocument;

@end
