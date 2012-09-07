/*
 * Copyright (c) 2005 Apple Computer, Inc.  All Rights Reserved.
 */

/*
 
 Structures for a Flattened Device Tree 
 */

#include "DT_Add.h"


#define RoundToLong(x)	(((x) + 3) & ~3)
#define kAllocSize 4096
#define DPRINTF(...)

static Node *rootNode;

static Node *freeNodes, *allocedNodes;
static Property *freeProperties, *allocedProperties;

static struct _DTSizeInfo {
    UINT32 numNodes;
    UINT32 numProperties;
    UINT32 totalPropertySize;
} DTInfo;

Property *
DT__AddProperty(Node *node, const char *name, UINT32 length, void *value)
{
    Property *prop;

    DPRINTF("DT__AddProperty([Node '%a'], '%a', %d, 0x%x)\n", DT__GetName(node), name, length, value);
    if (freeProperties == NULL) {
        void *buf = AllocateZeroPool(kAllocSize);
       INTN i;
        
        DPRINTF("Allocating more free properties\n");
        if (buf == 0) return 0;
        //bzero(buf, kAllocSize);
        // Use the first property to record the allocated buffer
        // for later freeing.
        prop = (Property *)buf;
        prop->next = allocedProperties;
        allocedProperties = prop;
        prop->value = buf;
        prop++;
        for (i=1; i<(kAllocSize / sizeof(Property)); i++) {
            prop->next = freeProperties;
            freeProperties = prop;
            prop++;
        }
    }
    prop = freeProperties;
    freeProperties = prop->next;

    prop->name = name;
    prop->length = length;
    prop->value = value;

    // Always add to end of list
    if (node->properties == 0) {
        node->properties = prop;
    } else {
        node->last_prop->next = prop;
    }
    node->last_prop = prop;
    prop->next = 0;

    DPRINTF("Done [0x%x]\n", prop);
    
    DTInfo.numProperties++;
    DTInfo.totalPropertySize += RoundToLong(length);

    return prop;
}

Node *
DT__AddChild(Node *parent, const char *name)
{
    Node *node;

    if (freeNodes == NULL) {
        void *buf = AllocateZeroPool(kAllocSize);
       INTN i;
        
        DPRINTF("Allocating more free nodes\n");
        if (buf == 0) return 0;
        //bzero(buf, kAllocSize);
        node = (Node *)buf;
        // Use the first node to record the allocated buffer
        // for later freeing.
        node->next = allocedNodes;
        allocedNodes = node;
        node->children = (Node *)buf;
        node++;
        for (i=1; i<(kAllocSize / sizeof(Node)); i++) {
            node->next = freeNodes;
            freeNodes = node;
            node++;
        }
    }
    DPRINTF("DT__AddChild(0x%x, '%a')\n", parent, name);
    node = freeNodes;
    freeNodes = node->next;
    DPRINTF("Got free node 0x%x\n", node);
    DPRINTF("prop = 0x%x, children = 0x%x, next = 0x%x\n", node->properties, node->children, node->next);

    if (parent == NULL) {
        rootNode = node;
        node->next = 0;
    } else {
        node->next = parent->children;
        parent->children = node;
    }
    DTInfo.numNodes++;
    DT__AddProperty(node, "name", AsciiStrLen(name) + 1, (void *) name);
    return node;
}

void
DT__FreeProperty(Property *prop)
{
    prop->next = freeProperties;
    freeProperties = prop;
}
void
DT__FreeNode(Node *node)
{
    node->next = freeNodes;
    freeNodes = node;
}

void
DT__Initialize(void)
{
    DPRINTF("DT__Initialize\n");
    
    freeNodes = 0;
    allocedNodes = 0;
    freeProperties = 0;
    allocedProperties = 0;
    
    DTInfo.numNodes = 0;
    DTInfo.numProperties = 0;
    DTInfo.totalPropertySize = 0;
    
    rootNode = DT__AddChild(NULL, "/");
    DPRINTF("DT__Initialize done\n");
}

/*
 * Free up memory used by in-memory representation
 * of device tree.
 */
void
DT__Finalize(void)
{
    Node *node;
    Property *prop;

    DPRINTF("DT__Finalize\n");
    for (prop = allocedProperties; prop != NULL; prop = prop->next) {
        FreePool(prop->value);
    }
    allocedProperties = NULL;
    freeProperties = NULL;

    for (node = allocedNodes; node != NULL; node = node->next) {
        FreePool((void *)node->children);
    }
    allocedNodes = NULL;
    freeNodes = NULL;
    rootNode = NULL;
    
    // XXX leaks any created strings

    DTInfo.numNodes = 0;
    DTInfo.numProperties = 0;
    DTInfo.totalPropertySize = 0;
}

static void *
FlattenNodes(Node *node, void *buffer)
{
    Property *prop;
    DeviceTreeNode *flatNode;
    DeviceTreeNodeProperty *flatProp;
   INTN count;

    if (node == 0) return buffer;

    flatNode = (DeviceTreeNode *)buffer;
    buffer += sizeof(DeviceTreeNode);

    for (count = 0, prop = node->properties; prop != 0; count++, prop = prop->next) {
        flatProp = (DeviceTreeNodeProperty *)buffer;
        AsciiStrCpy(flatProp->name, prop->name);
        flatProp->length = prop->length;
        buffer += sizeof(DeviceTreeNodeProperty);
        CopyMem(buffer, prop->value, prop->length);
        buffer += RoundToLong(prop->length);
    }
    flatNode->nProperties = count;

    for (count = 0, node = node->children; node != 0; count++, node = node->next) {
        buffer = FlattenNodes(node, buffer);
    }
    flatNode->nChildren = count;

    return buffer;
}

/*
 * Flatten the in-memory representation of the device tree
 * into a binary DT block.
 * To get the buffer size needed, call with result = 0.
 * To have a buffer allocated for you, call with *result = 0.
 * To use your own buffer, call with *result = &buffer.
 */

void
DT__FlattenDeviceTree(void **buffer_p, UINT32 *length)
{
    UINT32 totalSize;
    void *buf;

    DPRINTF("DT__FlattenDeviceTree(0x%x, 0x%x)\n", buffer_p, length);
#if DEBUG
    if (buffer_p) DT__PrintTree(rootNode);
#endif
    
    totalSize = DTInfo.numNodes * sizeof(DeviceTreeNode) + 
        DTInfo.numProperties * sizeof(DeviceTreeNodeProperty) +
        DTInfo.totalPropertySize;

    DPRINTF("Total size 0x%x\n", totalSize);
    if (buffer_p != 0) {
        if (totalSize == 0) {
            buf = 0;
        } else {
            if (*buffer_p == 0) {
                buf = AllocateZeroPool(totalSize);
            } else {
                buf = *buffer_p;
            }
            //bzero(buf, totalSize);
            
            FlattenNodes(rootNode, buf);
        }
        *buffer_p = buf;
    }
    if (length)
        *length = totalSize;
}

char *
DT__GetName(Node *node)
{
    Property *prop;

    DPRINTF("DT__GetName(0x%x)\n", node);
    DPRINTF("Node properties = 0x%x\n", node->properties);
    for (prop = node->properties; prop; prop = prop->next) {
        DPRINTF("Prop '%a'\n", prop->name);
        if (AsciiStrCmp(prop->name, "name") == 0) {
            return prop->value;
        }
    }
    DPRINTF("DT__GetName returns 0\n");
    return "(null)";
}

Node *
DT__FindNode(const char *path, BOOLEAN createIfMissing)
{
    Node *node, *child;
    DTPropertyNameBuf nameBuf;
    char *bp;
   INTN i;

    DPRINTF("DT__FindNode('%a', %d)\n", path, createIfMissing);
    
    // Start at root
    node = rootNode;
    DPRINTF("root = 0x%x\n", rootNode);

    while (node) {
        // Skip leading slash
        while (*path == '/') path++;

        for (i=0, bp = nameBuf; ++i < kDTMaxEntryNameLength && *path && *path != '/'; bp++, path++) *bp = *path;
        *bp = '\0';

        if (nameBuf[0] == '\0') {
            // last path entry
            break;
        }
        DPRINTF("Node '%a'\n", nameBuf);

        for (child = node->children; child != 0; child = child->next) {
            DPRINTF("Child 0x%x\n", child);
            if (AsciiStrCmp(DT__GetName(child), nameBuf) == 0) {
                break;
            }
        }
        if (child == 0 && createIfMissing) {
            DPRINTF("Creating node\n");
            char *str = AllocatePool(AsciiStrLen(nameBuf) + 1);
            // XXX this will leak
            AsciiStrCpy(str, nameBuf);

            child = DT__AddChild(node, str);
        }
        node = child;
    }
    return node;
}

#if DEBUG

void
DT__PrintNode(Node *node, INTN level)
{
    char spaces[10], *cp = spaces;
    Property *prop;

    if (level > 9) level = 9;
    while (level--) *cp++ = ' ';
    *cp = '\0';

    verbose("%a===Node===\n", spaces);
    for (prop = node->properties; prop; prop = prop->next) {
        char c = *((char *)prop->value);
        if (prop->length < 64 && (
            AsciiStrCmp(prop->name, "name") == 0 || 
            (c >= '0' && c <= '9') ||
            (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') || c == '_')) {
            verbose("%a Property '%a' [%d] = '%a'\n", spaces, prop->name, prop->length, prop->value);
        } else {
            verbose("%a Property '%a' [%d] = (data)\n", spaces, prop->name, prop->length);
        }
    }
    verbose("%a==========\n", spaces);
}

static void
_PrintTree(Node *node, INTN level)
{
    DT__PrintNode(node, level);
    level++;
    for (node = node->children; node; node = node->next)
        _PrintTree(node, level);
}

void
DT__PrintTree(Node *node)
{
    if (node == 0) node = rootNode;
    _PrintTree(node, 0);
}

void
DT__PrintFlattenedNode(DTEntry entry, INTN level)
{
    char spaces[10], *cp = spaces;
    DTPropertyIterator	                propIter;
    char *name;
    void *prop;
   INTN propSize;

    if (level > 9) level = 9;
    while (level--) *cp++ = ' ';
    *cp = '\0';

    verbose("%a===Entry %p===\n", spaces, entry);
    if (kSuccess != DTCreatePropertyIterator(entry, &propIter)) {
        verbose("Couldn't create property iterator\n");
        return;
    }
    while( kSuccess == DTIterateProperties( propIter, &name)) {
        if(  kSuccess != DTGetProperty( entry, name, &prop, &propSize ))
            continue;
        verbose("%a Property %a = %a\n", spaces, name, prop);
    }
    DTDisposePropertyIterator(propIter);

    verbose("%a==========\n", spaces);
}

static void
_PrintFlattenedTree(DTEntry entry, INTN level)
{
    DTEntryIterator entryIter;

    PrintFlattenedNode(entry, level);

    if (kSuccess != DTCreateEntryIterator(entry, &entryIter)) {
            printf("Couldn't create entry iterator\n");
            return;
    }
    level++;
    while (kSuccess == DTIterateEntries( entryIter, &entry )) {
        _PrintFlattenedTree(entry, level);
    }
    DTDisposeEntryIterator(entryIter);
}

void
DT__PrintFlattenedTree(DTEntry entry)
{
    _PrintFlattenedTree(entry, 0);
}


int
main(int argc, char **argv)
{
    DTEntry                             dtEntry;
    DTPropertyIterator	                propIter;
    DTEntryIterator                     entryIter;
    void				*prop;
    int					propSize;
    char				*name;
    void *flatTree;
    UINT32 flatSize;

    Node *node;

    node = AddChild(NULL, "device-tree");
    AddProperty(node, "potato", 4, "foo");
    AddProperty(node, "chemistry", 4, "bar");
    AddProperty(node, "physics", 4, "baz");

    node = AddChild(node, "dev");
    AddProperty(node, "one", 4, "one");
    AddProperty(node, "two", 4, "two");
    AddProperty(node, "three", 6, "three");

    node = AddChild(rootNode, "foo");
    AddProperty(node, "aaa", 4, "aab");
    AddProperty(node, "bbb", 4, "bbc");
    AddProperty(node, "cccc", 6, "ccccd");

    node = FindNode("/this/is/a/test", 1);
    AddProperty(node, "dddd", 12, "abcdefghijk");

    verbose("In-memory tree:\n\n");

    PrintTree(rootNode);

    FlattenDeviceTree(&flatTree, &flatSize);

    verbose("Flat tree = %p, size %d\n", flatTree, flatSize);

    dtEntry = (DTEntry)flatTree;

    verbose("\n\nPrinting flat tree\n\n");

    DTInit(dtEntry);

    PrintFlattenedTree((DTEntry)flatTree);
#if 0
        printf("=== Entry %p ===\n", dtEntry);
        if (kSuccess != DTCreatePropertyIterator(dtEntry, &propIter)) {
            printf("Couldn't create property iterator\n");
            return 1;
        }
        while( kSuccess == DTIterateProperties( propIter, &name)) {
            if(  kSuccess != DTGetProperty( dtEntry, name, &prop, &propSize ))
                continue;
            printf(" Property %a = %a\n", name, prop);
        }
        DTDisposePropertyIterator(propIter);
        printf("========\n");

    if (kSuccess != DTCreateEntryIterator(dtEntry, &entryIter)) {
            printf("Couldn't create entry iterator\n");
            return 1;
    }
    while (kSuccess == DTIterateEntries( entryIter, &dtEntry )) {
        printf("=== Entry %p ===\n", dtEntry);

        if (kSuccess != DTCreatePropertyIterator(dtEntry, &propIter)) {
            printf("Couldn't create property iterator\n");
            return 1;
        }
        while( kSuccess == DTIterateProperties( propIter, &name)) {
            if(  kSuccess != DTGetProperty( dtEntry, name, &prop, &propSize ))
                continue;
            printf(" Property %a = %a\n", name, prop);
        }
        DTDisposePropertyIterator(propIter);
        printf("========\n");
    }
    DTDisposeEntryIterator(entryIter);
#endif

    return 0;
}

#endif

