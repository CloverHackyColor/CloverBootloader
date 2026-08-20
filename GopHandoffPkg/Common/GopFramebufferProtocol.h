#ifndef GOP_FRAMEBUFFER_PROTOCOL_H
#define GOP_FRAMEBUFFER_PROTOCOL_H

#if defined(GOPFB_EDK2)
#include <Base.h>

typedef UINT8 GopFbUint8;
typedef UINT16 GopFbUint16;
typedef UINT32 GopFbUint32;
typedef UINT64 GopFbUint64;
typedef UINTN GopFbSize;

#define GOPFB_U8_C(value) ((GopFbUint8)(value##U))
#define GOPFB_U16_C(value) ((GopFbUint16)(value##U))
#define GOPFB_U32_C(value) ((GopFbUint32)(value##U))
#define GOPFB_U64_C(value) ((GopFbUint64)(value##ULL))
#define GOPFB_U32_MAX MAX_UINT32
#define GOPFB_U64_MAX MAX_UINT64
#define GOPFB_SIZE_MAX MAX_UINTN
#define GOPFB_OFFSET_OF(type, field) OFFSET_OF(type, field)
#else
#include <stddef.h>
#include <stdint.h>

typedef uint8_t GopFbUint8;
typedef uint16_t GopFbUint16;
typedef uint32_t GopFbUint32;
typedef uint64_t GopFbUint64;
typedef size_t GopFbSize;

#define GOPFB_U8_C(value) UINT8_C(value)
#define GOPFB_U16_C(value) UINT16_C(value)
#define GOPFB_U32_C(value) UINT32_C(value)
#define GOPFB_U64_C(value) UINT64_C(value)
#define GOPFB_U32_MAX UINT32_MAX
#define GOPFB_U64_MAX UINT64_MAX
#define GOPFB_SIZE_MAX SIZE_MAX
#define GOPFB_OFFSET_OF(type, field) offsetof(type, field)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define GOPFB_HANDOFF_MAGIC GOPFB_U32_C(0x474F5046) /* GOPF */
#define GOPFB_HANDOFF_VERSION GOPFB_U16_C(2)

#define GOPFB_FLAG_CONSOLE_OUT GOPFB_U32_C(0x00000001)
#define GOPFB_FLAG_READY_TO_BOOT GOPFB_U32_C(0x00000002)
#define GOPFB_FLAG_MODE_REAPPLIED GOPFB_U32_C(0x00000004)
#define GOPFB_FLAG_MODE_REQUEST_APPLIED GOPFB_U32_C(0x00000008)
#define GOPFB_FLAG_EARLY_CAPTURE GOPFB_U32_C(0x00000010)
#define GOPFB_CAPTURE_PHASE_FLAGS                                           \
    (GOPFB_FLAG_READY_TO_BOOT | GOPFB_FLAG_EARLY_CAPTURE)
#define GOPFB_KNOWN_FLAGS                                                   \
    (GOPFB_FLAG_CONSOLE_OUT | GOPFB_FLAG_READY_TO_BOOT |                    \
     GOPFB_FLAG_MODE_REAPPLIED | GOPFB_FLAG_MODE_REQUEST_APPLIED |          \
     GOPFB_FLAG_EARLY_CAPTURE)

#define GOPFB_PIXEL_RED_GREEN_BLUE_RESERVED_8 GOPFB_U32_C(0)
#define GOPFB_PIXEL_BLUE_GREEN_RED_RESERVED_8 GOPFB_U32_C(1)
#define GOPFB_PIXEL_BIT_MASK GOPFB_U32_C(2)
#define GOPFB_PIXEL_BLT_ONLY GOPFB_U32_C(3)

#define GOPFB_PIXEL_ENCODING_BITS GOPFB_U32_C(32)
#define GOPFB_PIXEL_ENCODING_SIZE GOPFB_U32_C(33)

typedef struct GopFramebufferHandoff {
    GopFbUint32 magic;
    GopFbUint16 version;
    GopFbUint16 headerSize;
    GopFbUint32 totalSize;
    GopFbUint32 flags;
    GopFbUint64 framebufferBase;
    GopFbUint64 framebufferSize;
    GopFbUint32 width;
    GopFbUint32 height;
    GopFbUint32 pixelsPerScanLine;
    GopFbUint32 pixelFormat;
    GopFbUint32 bytesPerPixel;
    GopFbUint32 redMask;
    GopFbUint32 greenMask;
    GopFbUint32 blueMask;
    GopFbUint32 reservedMask;
    GopFbUint32 reserved[6];
    GopFbUint32 crc32;
} GopFramebufferHandoff;

#define GOPFB_PCI_INFO_MAGIC GOPFB_U32_C(0x47504349) /* GPCI */
#define GOPFB_PCI_INFO_VERSION GOPFB_U16_C(1)
#define GOPFB_PCI_FLAG_LOCATION_VALID GOPFB_U32_C(0x00000001)
#define GOPFB_PCI_FLAG_IDENTITY_VALID GOPFB_U32_C(0x00000002)
#define GOPFB_PCI_FLAG_CLASS_VALID GOPFB_U32_C(0x00000004)
#define GOPFB_PCI_FLAG_BAR_VALID GOPFB_U32_C(0x00000008)
#define GOPFB_PCI_KNOWN_FLAGS \
    (GOPFB_PCI_FLAG_LOCATION_VALID | GOPFB_PCI_FLAG_IDENTITY_VALID | \
     GOPFB_PCI_FLAG_CLASS_VALID | GOPFB_PCI_FLAG_BAR_VALID)
#define GOPFB_PCI_INVALID_BAR_INDEX GOPFB_U32_C(0xFFFFFFFF)

typedef struct GopFramebufferPciInfo {
    GopFbUint32 magic;
    GopFbUint16 version;
    GopFbUint16 headerSize;
    GopFbUint32 totalSize;
    GopFbUint32 flags;
    GopFbUint32 segment;
    GopFbUint32 bus;
    GopFbUint32 device;
    GopFbUint32 function;
    GopFbUint32 vendorId;
    GopFbUint32 deviceId;
    GopFbUint32 classCode;
    GopFbUint32 barIndex;
    GopFbUint64 barBase;
    GopFbUint64 barSize;
    GopFbUint64 framebufferOffset;
    GopFbUint32 reserved[3];
    GopFbUint32 crc32;
} GopFramebufferPciInfo;

#define GOPFB_DISPLAY_INFO_MAGIC GOPFB_U32_C(0x47444946) /* GDIF */
#define GOPFB_DISPLAY_INFO_VERSION GOPFB_U16_C(1)
#define GOPFB_DISPLAY_FLAG_EDID_ACTIVE GOPFB_U32_C(0x00000001)
#define GOPFB_DISPLAY_FLAG_EDID_DISCOVERED GOPFB_U32_C(0x00000002)
#define GOPFB_DISPLAY_KNOWN_FLAGS                                      \
    (GOPFB_DISPLAY_FLAG_EDID_ACTIVE | GOPFB_DISPLAY_FLAG_EDID_DISCOVERED)
#define GOPFB_INVALID_MODE_INDEX GOPFB_U32_MAX
#define GOPFB_EDID_BLOCK_SIZE GOPFB_U32_C(128)
#define GOPFB_EDID_MAX_BLOCK_COUNT GOPFB_U32_C(256)
#define GOPFB_EDID_MAX_SIZE \
    (GOPFB_EDID_BLOCK_SIZE * GOPFB_EDID_MAX_BLOCK_COUNT)
#define GOPFB_MODE_CATALOG_MAX_COUNT GOPFB_U32_C(4096)

typedef struct GopFramebufferModeDescriptor {
    GopFbUint32 modeNumber;
    GopFbUint32 width;
    GopFbUint32 height;
    GopFbUint32 pixelsPerScanLine;
    GopFbUint32 pixelFormat;
    GopFbUint32 bytesPerPixel;
    GopFbUint32 redMask;
    GopFbUint32 greenMask;
    GopFbUint32 blueMask;
    GopFbUint32 reservedMask;
} GopFramebufferModeDescriptor;

typedef struct GopFramebufferDisplayInfoHeader {
    GopFbUint32 magic;
    GopFbUint16 version;
    GopFbUint16 headerSize;
    GopFbUint32 totalSize;
    GopFbUint32 flags;
    GopFbUint32 modeCount;
    GopFbUint32 currentModeArrayIndex;
    GopFbUint32 modesOffset;
    GopFbUint32 edidOffset;
    GopFbUint32 edidSize;
    GopFbUint32 reserved[6];
    GopFbUint32 crc32;
} GopFramebufferDisplayInfoHeader;

#define GOPFB_MODE_REQUEST_MAGIC GOPFB_U32_C(0x474F504D) /* GOPM */
#define GOPFB_MODE_REQUEST_VERSION GOPFB_U16_C(1)
#define GOPFB_MODE_REQUEST_BY_NUMBER GOPFB_U32_C(0x00000001)
#define GOPFB_MODE_REQUEST_BY_DIMENSIONS GOPFB_U32_C(0x00000002)
#define GOPFB_MODE_REQUEST_KNOWN_FLAGS                                  \
    (GOPFB_MODE_REQUEST_BY_NUMBER | GOPFB_MODE_REQUEST_BY_DIMENSIONS)

typedef struct GopFramebufferModeRequest {
    GopFbUint32 magic;
    GopFbUint16 version;
    GopFbUint16 totalSize;
    GopFbUint32 flags;
    GopFbUint32 modeNumber;
    GopFbUint32 width;
    GopFbUint32 height;
    GopFbUint32 reserved[2];
    GopFbUint32 crc32;
} GopFramebufferModeRequest;

#if defined(GOPFB_EDK2)
STATIC_ASSERT(sizeof(GopFramebufferHandoff) == 96U,
              "GopFramebufferHandoff ABI size changed");
STATIC_ASSERT(GOPFB_OFFSET_OF(GopFramebufferHandoff, crc32) == 92U,
              "GopFramebufferHandoff checksum offset changed");
STATIC_ASSERT(sizeof(GopFramebufferPciInfo) == 88U,
              "GopFramebufferPciInfo ABI size changed");
STATIC_ASSERT(GOPFB_OFFSET_OF(GopFramebufferPciInfo, crc32) == 84U,
              "GopFramebufferPciInfo checksum offset changed");
STATIC_ASSERT(sizeof(GopFramebufferModeDescriptor) == 40U,
              "GopFramebufferModeDescriptor ABI size changed");
STATIC_ASSERT(sizeof(GopFramebufferDisplayInfoHeader) == 64U,
              "GopFramebufferDisplayInfoHeader ABI size changed");
STATIC_ASSERT(GOPFB_OFFSET_OF(GopFramebufferDisplayInfoHeader, crc32) == 60U,
              "GopFramebufferDisplayInfoHeader checksum offset changed");
STATIC_ASSERT(sizeof(GopFramebufferModeRequest) == 36U,
              "GopFramebufferModeRequest ABI size changed");
STATIC_ASSERT(GOPFB_OFFSET_OF(GopFramebufferModeRequest, crc32) == 32U,
              "GopFramebufferModeRequest checksum offset changed");
#elif defined(__cplusplus)
static_assert(sizeof(GopFramebufferHandoff) == 96U,
              "GopFramebufferHandoff ABI size changed");
static_assert(GOPFB_OFFSET_OF(GopFramebufferHandoff, crc32) == 92U,
              "GopFramebufferHandoff checksum offset changed");
static_assert(sizeof(GopFramebufferPciInfo) == 88U,
              "GopFramebufferPciInfo ABI size changed");
static_assert(GOPFB_OFFSET_OF(GopFramebufferPciInfo, crc32) == 84U,
              "GopFramebufferPciInfo checksum offset changed");
static_assert(sizeof(GopFramebufferModeDescriptor) == 40U,
              "GopFramebufferModeDescriptor ABI size changed");
static_assert(sizeof(GopFramebufferDisplayInfoHeader) == 64U,
              "GopFramebufferDisplayInfoHeader ABI size changed");
static_assert(GOPFB_OFFSET_OF(GopFramebufferDisplayInfoHeader, crc32) == 60U,
              "GopFramebufferDisplayInfoHeader checksum offset changed");
static_assert(sizeof(GopFramebufferModeRequest) == 36U,
              "GopFramebufferModeRequest ABI size changed");
static_assert(GOPFB_OFFSET_OF(GopFramebufferModeRequest, crc32) == 32U,
              "GopFramebufferModeRequest checksum offset changed");
#else
_Static_assert(sizeof(GopFramebufferHandoff) == 96U,
               "GopFramebufferHandoff ABI size changed");
_Static_assert(GOPFB_OFFSET_OF(GopFramebufferHandoff, crc32) == 92U,
               "GopFramebufferHandoff checksum offset changed");
_Static_assert(sizeof(GopFramebufferPciInfo) == 88U,
               "GopFramebufferPciInfo ABI size changed");
_Static_assert(GOPFB_OFFSET_OF(GopFramebufferPciInfo, crc32) == 84U,
               "GopFramebufferPciInfo checksum offset changed");
_Static_assert(sizeof(GopFramebufferModeDescriptor) == 40U,
               "GopFramebufferModeDescriptor ABI size changed");
_Static_assert(sizeof(GopFramebufferDisplayInfoHeader) == 64U,
               "GopFramebufferDisplayInfoHeader ABI size changed");
_Static_assert(GOPFB_OFFSET_OF(GopFramebufferDisplayInfoHeader, crc32) == 60U,
               "GopFramebufferDisplayInfoHeader checksum offset changed");
_Static_assert(sizeof(GopFramebufferModeRequest) == 36U,
               "GopFramebufferModeRequest ABI size changed");
_Static_assert(GOPFB_OFFSET_OF(GopFramebufferModeRequest, crc32) == 32U,
               "GopFramebufferModeRequest checksum offset changed");
#endif

typedef enum GopFramebufferStatus {
    GOPFB_STATUS_OK = 0,
    GOPFB_STATUS_NULL,
    GOPFB_STATUS_BAD_MAGIC,
    GOPFB_STATUS_BAD_VERSION,
    GOPFB_STATUS_BAD_STRUCTURE_SIZE,
    GOPFB_STATUS_RESERVED_NOT_ZERO,
    GOPFB_STATUS_BAD_CHECKSUM,
    GOPFB_STATUS_BAD_FLAGS,
    GOPFB_STATUS_BAD_FRAMEBUFFER_RANGE,
    GOPFB_STATUS_BAD_DIMENSIONS,
    GOPFB_STATUS_BAD_STRIDE,
    GOPFB_STATUS_UNSUPPORTED_PIXEL_FORMAT,
    GOPFB_STATUS_UNSUPPORTED_PIXEL_SIZE,
    GOPFB_STATUS_INVALID_PIXEL_MASKS,
    GOPFB_STATUS_ARITHMETIC_OVERFLOW,
    GOPFB_STATUS_FRAMEBUFFER_TOO_SMALL
} GopFramebufferStatus;

typedef enum GopFramebufferPciInfoStatus {
    GOPFB_PCI_STATUS_OK = 0,
    GOPFB_PCI_STATUS_NULL,
    GOPFB_PCI_STATUS_BAD_MAGIC,
    GOPFB_PCI_STATUS_BAD_VERSION,
    GOPFB_PCI_STATUS_BAD_STRUCTURE_SIZE,
    GOPFB_PCI_STATUS_RESERVED_NOT_ZERO,
    GOPFB_PCI_STATUS_BAD_CHECKSUM,
    GOPFB_PCI_STATUS_BAD_FLAGS,
    GOPFB_PCI_STATUS_BAD_LOCATION,
    GOPFB_PCI_STATUS_BAD_IDENTITY,
    GOPFB_PCI_STATUS_BAD_CLASS,
    GOPFB_PCI_STATUS_BAD_BAR
} GopFramebufferPciInfoStatus;

typedef enum GopFramebufferEdidStatus {
    GOPFB_EDID_STATUS_OK = 0,
    GOPFB_EDID_STATUS_ABSENT,
    GOPFB_EDID_STATUS_BAD_SIZE,
    GOPFB_EDID_STATUS_BAD_HEADER,
    GOPFB_EDID_STATUS_BAD_EXTENSION_COUNT,
    GOPFB_EDID_STATUS_BAD_CHECKSUM
} GopFramebufferEdidStatus;

typedef enum GopFramebufferDisplayInfoStatus {
    GOPFB_DISPLAY_STATUS_OK = 0,
    GOPFB_DISPLAY_STATUS_NULL,
    GOPFB_DISPLAY_STATUS_BAD_MAGIC,
    GOPFB_DISPLAY_STATUS_BAD_VERSION,
    GOPFB_DISPLAY_STATUS_BAD_STRUCTURE_SIZE,
    GOPFB_DISPLAY_STATUS_RESERVED_NOT_ZERO,
    GOPFB_DISPLAY_STATUS_BAD_CHECKSUM,
    GOPFB_DISPLAY_STATUS_BAD_FLAGS,
    GOPFB_DISPLAY_STATUS_BAD_LAYOUT,
    GOPFB_DISPLAY_STATUS_TOO_MANY_MODES,
    GOPFB_DISPLAY_STATUS_BAD_CURRENT_MODE,
    GOPFB_DISPLAY_STATUS_DUPLICATE_MODE_NUMBER,
    GOPFB_DISPLAY_STATUS_NON_CANONICAL_MODE_ORDER,
    GOPFB_DISPLAY_STATUS_INVALID_MODE,
    GOPFB_DISPLAY_STATUS_INVALID_EDID
} GopFramebufferDisplayInfoStatus;

typedef enum GopFramebufferModeRequestStatus {
    GOPFB_MODE_REQUEST_STATUS_OK = 0,
    GOPFB_MODE_REQUEST_STATUS_NULL,
    GOPFB_MODE_REQUEST_STATUS_BAD_MAGIC,
    GOPFB_MODE_REQUEST_STATUS_BAD_VERSION,
    GOPFB_MODE_REQUEST_STATUS_BAD_STRUCTURE_SIZE,
    GOPFB_MODE_REQUEST_STATUS_RESERVED_NOT_ZERO,
    GOPFB_MODE_REQUEST_STATUS_BAD_CHECKSUM,
    GOPFB_MODE_REQUEST_STATUS_BAD_FLAGS,
    GOPFB_MODE_REQUEST_STATUS_BAD_TARGET
} GopFramebufferModeRequestStatus;

GopFbUint32 gopfb_crc32(const void *data, GopFbSize length);
int gopfb_size_add(GopFbSize left, GopFbSize right, GopFbSize *result);
int gopfb_size_multiply(GopFbSize left, GopFbSize right, GopFbSize *result);

void gopfb_initialize(GopFramebufferHandoff *handoff);
void gopfb_finalize(GopFramebufferHandoff *handoff);
GopFramebufferStatus gopfb_validate(const GopFramebufferHandoff *handoff,
                                    GopFbUint64 *requiredBytes);
GopFramebufferStatus gopfb_validate_mode_descriptor(
    const GopFramebufferModeDescriptor *mode);
int gopfb_build_pixel_encoding(
    const GopFramebufferModeDescriptor *mode,
    char *encoding,
    GopFbSize encodingSize);
const char *gopfb_status_string(GopFramebufferStatus status);

void gopfb_initialize_pci_info(GopFramebufferPciInfo *pciInfo);
void gopfb_finalize_pci_info(GopFramebufferPciInfo *pciInfo);
GopFramebufferPciInfoStatus gopfb_validate_pci_info(
    const GopFramebufferPciInfo *pciInfo);
const char *gopfb_pci_status_string(GopFramebufferPciInfoStatus status);

GopFramebufferEdidStatus gopfb_validate_edid(const GopFbUint8 *edid,
                                             GopFbSize edidSize);
const char *gopfb_edid_status_string(GopFramebufferEdidStatus status);

void gopfb_initialize_display_info(GopFramebufferDisplayInfoHeader *header,
                                   GopFbUint32 totalSize);
void gopfb_finalize_display_info(void *displayInfo, GopFbSize displayInfoSize);
GopFramebufferDisplayInfoStatus gopfb_validate_display_info(
    const void *displayInfo,
    GopFbSize displayInfoSize,
    const GopFramebufferModeDescriptor **modes,
    const GopFbUint8 **edid);
const char *gopfb_display_status_string(
    GopFramebufferDisplayInfoStatus status);

void gopfb_initialize_mode_request(GopFramebufferModeRequest *request);
void gopfb_finalize_mode_request(GopFramebufferModeRequest *request);
GopFramebufferModeRequestStatus gopfb_validate_mode_request(
    const GopFramebufferModeRequest *request);
const char *gopfb_mode_request_status_string(
    GopFramebufferModeRequestStatus status);

#ifdef __cplusplus
}
#endif

#endif
