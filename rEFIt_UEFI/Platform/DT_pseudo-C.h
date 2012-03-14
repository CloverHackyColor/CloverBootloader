#define kPropNameLength	32
#define kDTMaxPropertyNameLength 31

typedef struct DeviceTreeNodeProperty {
  CHAR8	name[kDTMaxPropertyNameLength];	// NUL terminated property name max [kPropNameLength]
  .zero
	UINT32		length;		// Length (bytes) of following prop value
	UINT8		value[length];	// Variable length value of property
	.align 8				// Padded to a multiple of a longword?
} DeviceTreeNodeProperty;

struct DeviceTreeNode;

typedef struct OpaqueDTEntry {
  UINT32		nProperties;	// Number of props[] elements (0 => end)
  UINT32		nChildren;	// Number of children[] elements
	DeviceTreeNodeProperty	props[nProperties];// array size == nProperties
	DeviceTreeNode			children[nChildren];	// array size == nChildren
} DeviceTreeNode;

typedef DeviceTreeNode *RealDTEntry;

typedef struct DTSavedScope {
	struct DTSavedScope * nextScope;
	RealDTEntry scope;
	RealDTEntry entry;
	UINT32 index;		
} *DTSavedScopePtr;

/* Entry Iterator*/
typedef struct OpaqueDTEntryIterator {
	RealDTEntry outerScope;
	RealDTEntry currentScope;
	RealDTEntry currentEntry;
	DTSavedScopePtr savedScope;
	unsigned long currentIndex;		
} *RealDTEntryIterator;



