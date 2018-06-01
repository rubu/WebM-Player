#import "EbmlTree.h"

@interface EbmlTreeNode : NSObject

@property (strong) NSString* title;
@property (strong) NSNumber* size;
@property (strong) NSString* value;
@property (strong) NSArray<EbmlTreeNode*>* children;

-(instancetype)initWithEbmlElement:(const EbmlElement&)ebmlElement;

@end

@implementation EbmlTreeNode

-(instancetype)initWithEbmlElement:(const EbmlElement&)ebmlElement
{
    if (self = [super init])
    {
        _title = [NSString stringWithUTF8String:get_ebml_element_name(ebmlElement.id()).c_str()];
        _size = [NSNumber numberWithUnsignedLongLong:ebmlElement.size()];
        if (ebmlElement.type() != EbmlElementType::Master)
        {
            _value = [NSString stringWithUTF8String:ebmlElement.value().c_str()];
        }
        else
        {
            _value = @"";
        }
        NSMutableArray<EbmlTreeNode*>* children = [NSMutableArray array];
        for (const auto& child : ebmlElement.children())
        {
            [children addObject:[[EbmlTreeNode alloc] initWithEbmlElement:child]];
        }
        _children = children;
    }
    return self;
}

@end

@implementation EbmlTree
{
    NSMutableArray<EbmlTreeNode*>* _nodes;
}

-(instancetype)initWithEbmlDocument:(const EbmlDocument*)ebmlDocument
{
    if (self = [super init])
    {
        const auto elements = ebmlDocument->elements();
        if (elements.empty() == false)
        {
            _nodes = [NSMutableArray array];
            for (const auto& element : elements)
            {
                [_nodes addObject:[[EbmlTreeNode alloc] initWithEbmlElement:element]];
            }
        }
    }
    return self;
}

- (NSInteger)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(nullable id)item
{
    if (item == nil)
    {
        return _nodes.count;
    }
    else
    {
        return ((EbmlTreeNode*)item).children.count;
    }
}

- (id)outlineView:(NSOutlineView *)outlineView child:(NSInteger)index ofItem:(nullable id)item
{
    if (item == nil)
    {
        return _nodes[index];
    }
    else
    {
        return ((EbmlTreeNode*)item).children[index];
    }
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
    return ((EbmlTreeNode*)item).children.count > 0;
}

- (nullable id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(nullable NSTableColumn *)tableColumn byItem:(nullable id)item
{
    if (item != nil)
    {
        EbmlTreeNode* node = item;
        if ([tableColumn.identifier isEqualToString:@"0"])
        {
            return node.title;
        }
        else if ([tableColumn.identifier isEqualToString:@"1"])
        {
            return node.size;
        }
        else if ([tableColumn.identifier isEqualToString:@"2"])
        {
            return node.value;
        }
    }
    return nil;
}

@end
