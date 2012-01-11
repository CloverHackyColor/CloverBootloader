/*
 * Copyright (c) 2005 Apple Computer, Inc.  All Rights Reserved.
 */

#ifndef __DEVICE_TREE_H
#define __DEVICE_TREE_H
#include <Uefi.h>
#include "device_tree.h"

typedef struct _Property {
    const char *             name;
    UINT32             length;
    void *             value;
    struct _Property * next;
} Property;

typedef struct _Node {
    struct _Property * properties;
    struct _Property * last_prop;    
    struct _Node *     children;
    struct _Node *     next;
} Node;

extern Property *
DT__AddProperty(Node *node, const char *name, UINT32 length, void *value);

extern Node *
DT__AddChild(Node *parent, const char *name);

Node *
DT__FindNode(const char *path, BOOLEAN createIfMissing);

extern void
DT__FreeProperty(Property *prop);

extern void
DT__FreeNode(Node *node);

extern char *
DT__GetName(Node *node);

void
DT__Initialize(void);

/*
 * Free up memory used by in-memory representation
 * of device tree.
 */
extern void
DT__Finalize(void);

void
DT__FlattenDeviceTree(void **result, UINT32 *length);


#endif /* __DEVICE_TREE_H */
