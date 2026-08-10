#include "GopFramebufferProtocol.h"

static int gopfb_u64_multiply(
    GopFbUint64 left,
    GopFbUint64 right,
    GopFbUint64 *result) {
    if (result == NULL) {
        return 0;
    }

    if ((left != GOPFB_U64_C(0)) && (right > (GOPFB_U64_MAX / left))) {
        return 0;
    }

    *result = left * right;
    return 1;
}

int gopfb_size_add(GopFbSize left, GopFbSize right, GopFbSize *result) {
    if (result == NULL) {
        return 0;
    }

    if (right > (GOPFB_SIZE_MAX - left)) {
        return 0;
    }

    *result = left + right;
    return 1;
}

int gopfb_size_multiply(GopFbSize left, GopFbSize right, GopFbSize *result) {
    if (result == NULL) {
        return 0;
    }

    if ((left != 0U) && (right > (GOPFB_SIZE_MAX / left))) {
        return 0;
    }

    *result = left * right;
    return 1;
}

static GopFbUint32 gopfb_crc32_update(
    GopFbUint32 crc,
    const GopFbUint8 *bytes,
    GopFbSize length) {
    GopFbSize index;

    if ((bytes == NULL) && (length != 0U)) {
        return GOPFB_U32_C(0);
    }

    for (index = 0U; index < length; ++index) {
        GopFbUint32 bit;
        crc ^= (GopFbUint32)bytes[index];

        for (bit = GOPFB_U32_C(0); bit < GOPFB_U32_C(8); ++bit) {
            const GopFbUint32 polynomial =
                (crc & GOPFB_U32_C(1)) != GOPFB_U32_C(0)
                    ? GOPFB_U32_C(0xEDB88320)
                    : GOPFB_U32_C(0);
            crc = (crc >> GOPFB_U32_C(1)) ^ polynomial;
        }
    }

    return crc;
}

static GopFbUint32 gopfb_crc32_with_zeroed_field(
    const void *data,
    GopFbSize length,
    GopFbSize fieldOffset,
    GopFbSize fieldSize) {
    static const GopFbUint8 zeros[8] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};
    const GopFbUint8 *bytes = (const GopFbUint8 *)data;
    GopFbSize suffixOffset;
    GopFbUint32 crc;

    if ((data == NULL) || (fieldSize > sizeof(zeros)) ||
        !gopfb_size_add(fieldOffset, fieldSize, &suffixOffset) ||
        (suffixOffset > length)) {
        return GOPFB_U32_C(0);
    }

    crc = GOPFB_U32_MAX;
    crc = gopfb_crc32_update(crc, bytes, fieldOffset);
    crc = gopfb_crc32_update(crc, zeros, fieldSize);
    crc = gopfb_crc32_update(crc, bytes + suffixOffset, length - suffixOffset);
    return ~crc;
}

static int gopfb_mask_is_contiguous_8bit(GopFbUint32 mask) {
    if (mask == GOPFB_U32_C(0)) {
        return 0;
    }

    while ((mask & GOPFB_U32_C(1)) == GOPFB_U32_C(0)) {
        mask >>= GOPFB_U32_C(1);
    }

    return mask == GOPFB_U32_C(0xFF);
}

static int gopfb_masks_do_not_overlap(
    GopFbUint32 redMask,
    GopFbUint32 greenMask,
    GopFbUint32 blueMask,
    GopFbUint32 reservedMask) {
    const GopFbUint32 colorUnion = redMask | greenMask | blueMask;

    if (((redMask & greenMask) != GOPFB_U32_C(0)) ||
        ((redMask & blueMask) != GOPFB_U32_C(0)) ||
        ((greenMask & blueMask) != GOPFB_U32_C(0)) ||
        ((colorUnion & reservedMask) != GOPFB_U32_C(0))) {
        return 0;
    }

    return 1;
}

static int gopfb_masks_are_valid(
    GopFbUint32 pixelFormat,
    GopFbUint32 redMask,
    GopFbUint32 greenMask,
    GopFbUint32 blueMask,
    GopFbUint32 reservedMask) {
    if (!gopfb_mask_is_contiguous_8bit(redMask) ||
        !gopfb_mask_is_contiguous_8bit(greenMask) ||
        !gopfb_mask_is_contiguous_8bit(blueMask) ||
        !gopfb_masks_do_not_overlap(
            redMask,
            greenMask,
            blueMask,
            reservedMask)) {
        return 0;
    }

    switch (pixelFormat) {
        case GOPFB_PIXEL_RED_GREEN_BLUE_RESERVED_8:
            return redMask == GOPFB_U32_C(0x000000FF) &&
                   greenMask == GOPFB_U32_C(0x0000FF00) &&
                   blueMask == GOPFB_U32_C(0x00FF0000) &&
                   reservedMask == GOPFB_U32_C(0xFF000000);

        case GOPFB_PIXEL_BLUE_GREEN_RED_RESERVED_8:
            return redMask == GOPFB_U32_C(0x00FF0000) &&
                   greenMask == GOPFB_U32_C(0x0000FF00) &&
                   blueMask == GOPFB_U32_C(0x000000FF) &&
                   reservedMask == GOPFB_U32_C(0xFF000000);

        case GOPFB_PIXEL_BIT_MASK:
            return 1;

        default:
            return 0;
    }
}

int gopfb_build_pixel_encoding(
    const GopFramebufferModeDescriptor *mode,
    char *encoding,
    GopFbSize encodingSize) {
    GopFbUint32 bitIndex;

    if ((encoding == NULL) || (encodingSize == 0U)) {
        return 0;
    }

    encoding[0] = '\0';
    if ((mode == NULL) ||
        (encodingSize < (GopFbSize)GOPFB_PIXEL_ENCODING_SIZE) ||
        (gopfb_validate_mode_descriptor(mode) != GOPFB_STATUS_OK)) {
        return 0;
    }

    for (bitIndex = GOPFB_U32_C(0);
         bitIndex < GOPFB_PIXEL_ENCODING_BITS;
         ++bitIndex) {
        const GopFbUint32 hardwareBit =
            (GOPFB_PIXEL_ENCODING_BITS - GOPFB_U32_C(1)) - bitIndex;
        const GopFbUint32 bitMask = GOPFB_U32_C(1) << hardwareBit;
        char component = '-';

        if ((mode->redMask & bitMask) != GOPFB_U32_C(0)) {
            component = 'R';
        } else if ((mode->greenMask & bitMask) != GOPFB_U32_C(0)) {
            component = 'G';
        } else if ((mode->blueMask & bitMask) != GOPFB_U32_C(0)) {
            component = 'B';
        }

        encoding[bitIndex] = component;
    }

    encoding[GOPFB_PIXEL_ENCODING_BITS] = '\0';
    return 1;
}

GopFbUint32 gopfb_crc32(const void *data, GopFbSize length) {
    if ((data == NULL) && (length != 0U)) {
        return GOPFB_U32_C(0);
    }

    return ~gopfb_crc32_update(
        GOPFB_U32_MAX,
        (const GopFbUint8 *)data,
        length);
}

void gopfb_initialize(GopFramebufferHandoff *handoff) {
    GopFbSize index;
    GopFbUint8 *bytes;

    if (handoff == NULL) {
        return;
    }

    bytes = (GopFbUint8 *)handoff;
    for (index = 0U; index < sizeof(*handoff); ++index) {
        bytes[index] = GOPFB_U8_C(0);
    }

    handoff->magic = GOPFB_HANDOFF_MAGIC;
    handoff->version = GOPFB_HANDOFF_VERSION;
    handoff->headerSize =
        (GopFbUint16)GOPFB_OFFSET_OF(GopFramebufferHandoff, reserved);
    handoff->totalSize = (GopFbUint32)sizeof(*handoff);
}

void gopfb_finalize(GopFramebufferHandoff *handoff) {
    if (handoff == NULL) {
        return;
    }

    handoff->crc32 = gopfb_crc32(
        handoff,
        GOPFB_OFFSET_OF(GopFramebufferHandoff, crc32));
}

GopFramebufferStatus gopfb_validate_mode_descriptor(
    const GopFramebufferModeDescriptor *mode) {
    if (mode == NULL) {
        return GOPFB_STATUS_NULL;
    }

    if ((mode->width == GOPFB_U32_C(0)) ||
        (mode->height == GOPFB_U32_C(0))) {
        return GOPFB_STATUS_BAD_DIMENSIONS;
    }

    if (mode->pixelsPerScanLine < mode->width) {
        return GOPFB_STATUS_BAD_STRIDE;
    }

    if ((mode->pixelFormat != GOPFB_PIXEL_RED_GREEN_BLUE_RESERVED_8) &&
        (mode->pixelFormat != GOPFB_PIXEL_BLUE_GREEN_RED_RESERVED_8) &&
        (mode->pixelFormat != GOPFB_PIXEL_BIT_MASK)) {
        return GOPFB_STATUS_UNSUPPORTED_PIXEL_FORMAT;
    }

    if (mode->bytesPerPixel != GOPFB_U32_C(4)) {
        return GOPFB_STATUS_UNSUPPORTED_PIXEL_SIZE;
    }

    if (!gopfb_masks_are_valid(
            mode->pixelFormat,
            mode->redMask,
            mode->greenMask,
            mode->blueMask,
            mode->reservedMask)) {
        return GOPFB_STATUS_INVALID_PIXEL_MASKS;
    }

    return GOPFB_STATUS_OK;
}

GopFramebufferStatus gopfb_validate(
    const GopFramebufferHandoff *handoff,
    GopFbUint64 *requiredBytes) {
    GopFramebufferModeDescriptor mode;
    GopFbUint64 bytesPerRow;
    GopFbUint64 visibleBytes;
    GopFbSize index;
    GopFramebufferStatus modeStatus;

    if (requiredBytes != NULL) {
        *requiredBytes = GOPFB_U64_C(0);
    }

    if (handoff == NULL) {
        return GOPFB_STATUS_NULL;
    }

    if (handoff->magic != GOPFB_HANDOFF_MAGIC) {
        return GOPFB_STATUS_BAD_MAGIC;
    }

    if (handoff->version != GOPFB_HANDOFF_VERSION) {
        return GOPFB_STATUS_BAD_VERSION;
    }

    if ((handoff->headerSize !=
         (GopFbUint16)GOPFB_OFFSET_OF(GopFramebufferHandoff, reserved)) ||
        (handoff->totalSize != (GopFbUint32)sizeof(*handoff))) {
        return GOPFB_STATUS_BAD_STRUCTURE_SIZE;
    }

    for (index = 0U;
         index < (sizeof(handoff->reserved) / sizeof(handoff->reserved[0]));
         ++index) {
        if (handoff->reserved[index] != GOPFB_U32_C(0)) {
            return GOPFB_STATUS_RESERVED_NOT_ZERO;
        }
    }

    if (handoff->crc32 !=
        gopfb_crc32(
            handoff,
            GOPFB_OFFSET_OF(GopFramebufferHandoff, crc32))) {
        return GOPFB_STATUS_BAD_CHECKSUM;
    }

    if (((handoff->flags & ~GOPFB_KNOWN_FLAGS) != GOPFB_U32_C(0)) ||
        ((handoff->flags & GOPFB_FLAG_READY_TO_BOOT) == GOPFB_U32_C(0)) ||
        ((handoff->flags & GOPFB_FLAG_MODE_REAPPLIED) == GOPFB_U32_C(0))) {
        return GOPFB_STATUS_BAD_FLAGS;
    }

    if ((handoff->framebufferBase == GOPFB_U64_C(0)) ||
        (handoff->framebufferSize == GOPFB_U64_C(0)) ||
        (handoff->framebufferBase >
         (GOPFB_U64_MAX - handoff->framebufferSize))) {
        return GOPFB_STATUS_BAD_FRAMEBUFFER_RANGE;
    }

    mode.modeNumber = GOPFB_U32_C(0);
    mode.width = handoff->width;
    mode.height = handoff->height;
    mode.pixelsPerScanLine = handoff->pixelsPerScanLine;
    mode.pixelFormat = handoff->pixelFormat;
    mode.bytesPerPixel = handoff->bytesPerPixel;
    mode.redMask = handoff->redMask;
    mode.greenMask = handoff->greenMask;
    mode.blueMask = handoff->blueMask;
    mode.reservedMask = handoff->reservedMask;
    modeStatus = gopfb_validate_mode_descriptor(&mode);
    if (modeStatus != GOPFB_STATUS_OK) {
        return modeStatus;
    }

    if (!gopfb_u64_multiply(
            (GopFbUint64)handoff->pixelsPerScanLine,
            (GopFbUint64)handoff->bytesPerPixel,
            &bytesPerRow) ||
        !gopfb_u64_multiply(
            bytesPerRow,
            (GopFbUint64)handoff->height,
            &visibleBytes)) {
        return GOPFB_STATUS_ARITHMETIC_OVERFLOW;
    }

    if (requiredBytes != NULL) {
        *requiredBytes = visibleBytes;
    }

    if (handoff->framebufferSize < visibleBytes) {
        return GOPFB_STATUS_FRAMEBUFFER_TOO_SMALL;
    }

    return GOPFB_STATUS_OK;
}

const char *gopfb_status_string(GopFramebufferStatus status) {
    switch (status) {
        case GOPFB_STATUS_OK:
            return "ok";
        case GOPFB_STATUS_NULL:
            return "null input";
        case GOPFB_STATUS_BAD_MAGIC:
            return "bad magic";
        case GOPFB_STATUS_BAD_VERSION:
            return "unsupported version";
        case GOPFB_STATUS_BAD_STRUCTURE_SIZE:
            return "bad structure size";
        case GOPFB_STATUS_RESERVED_NOT_ZERO:
            return "reserved field is not zero";
        case GOPFB_STATUS_BAD_CHECKSUM:
            return "bad checksum";
        case GOPFB_STATUS_BAD_FLAGS:
            return "bad flags";
        case GOPFB_STATUS_BAD_FRAMEBUFFER_RANGE:
            return "bad framebuffer range";
        case GOPFB_STATUS_BAD_DIMENSIONS:
            return "bad dimensions";
        case GOPFB_STATUS_BAD_STRIDE:
            return "bad stride";
        case GOPFB_STATUS_UNSUPPORTED_PIXEL_FORMAT:
            return "unsupported pixel format";
        case GOPFB_STATUS_UNSUPPORTED_PIXEL_SIZE:
            return "unsupported pixel size";
        case GOPFB_STATUS_INVALID_PIXEL_MASKS:
            return "invalid pixel masks";
        case GOPFB_STATUS_ARITHMETIC_OVERFLOW:
            return "arithmetic overflow";
        case GOPFB_STATUS_FRAMEBUFFER_TOO_SMALL:
            return "framebuffer is smaller than the active mode";
        default:
            return "unknown status";
    }
}

GopFramebufferEdidStatus gopfb_validate_edid(
    const GopFbUint8 *edid,
    GopFbSize edidSize) {
    static const GopFbUint8 header[8] = {
        0x00U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0x00U
    };
    GopFbSize blockCount;
    GopFbSize expectedBlocks;
    GopFbSize blockIndex;
    GopFbSize index;

    if (edidSize == 0U) {
        return edid == NULL ? GOPFB_EDID_STATUS_ABSENT
                            : GOPFB_EDID_STATUS_BAD_SIZE;
    }

    if ((edid == NULL) ||
        (edidSize < (GopFbSize)GOPFB_EDID_BLOCK_SIZE) ||
        ((edidSize % (GopFbSize)GOPFB_EDID_BLOCK_SIZE) != 0U)) {
        return GOPFB_EDID_STATUS_BAD_SIZE;
    }

    for (index = 0U; index < sizeof(header); ++index) {
        if (edid[index] != header[index]) {
            return GOPFB_EDID_STATUS_BAD_HEADER;
        }
    }

    blockCount = edidSize / (GopFbSize)GOPFB_EDID_BLOCK_SIZE;
    expectedBlocks = (GopFbSize)edid[126] + 1U;
    if (expectedBlocks != blockCount) {
        return GOPFB_EDID_STATUS_BAD_EXTENSION_COUNT;
    }

    for (blockIndex = 0U; blockIndex < blockCount; ++blockIndex) {
        GopFbUint32 sum = GOPFB_U32_C(0);
        const GopFbSize blockOffset =
            blockIndex * (GopFbSize)GOPFB_EDID_BLOCK_SIZE;

        for (index = 0U;
             index < (GopFbSize)GOPFB_EDID_BLOCK_SIZE;
             ++index) {
            sum += (GopFbUint32)edid[blockOffset + index];
        }

        if ((sum & GOPFB_U32_C(0xFF)) != GOPFB_U32_C(0)) {
            return GOPFB_EDID_STATUS_BAD_CHECKSUM;
        }
    }

    return GOPFB_EDID_STATUS_OK;
}

const char *gopfb_edid_status_string(GopFramebufferEdidStatus status) {
    switch (status) {
        case GOPFB_EDID_STATUS_OK:
            return "ok";
        case GOPFB_EDID_STATUS_ABSENT:
            return "absent";
        case GOPFB_EDID_STATUS_BAD_SIZE:
            return "bad size";
        case GOPFB_EDID_STATUS_BAD_HEADER:
            return "bad header";
        case GOPFB_EDID_STATUS_BAD_EXTENSION_COUNT:
            return "bad extension count";
        case GOPFB_EDID_STATUS_BAD_CHECKSUM:
            return "bad checksum";
        default:
            return "unknown EDID status";
    }
}

void gopfb_initialize_display_info(
    GopFramebufferDisplayInfoHeader *header,
    GopFbUint32 totalSize) {
    GopFbSize index;
    GopFbUint8 *bytes;

    if (header == NULL) {
        return;
    }

    bytes = (GopFbUint8 *)header;
    for (index = 0U; index < sizeof(*header); ++index) {
        bytes[index] = GOPFB_U8_C(0);
    }

    header->magic = GOPFB_DISPLAY_INFO_MAGIC;
    header->version = GOPFB_DISPLAY_INFO_VERSION;
    header->headerSize = (GopFbUint16)sizeof(*header);
    header->totalSize = totalSize;
    header->currentModeArrayIndex = GOPFB_INVALID_MODE_INDEX;
    header->modesOffset = (GopFbUint32)sizeof(*header);
    header->edidOffset = (GopFbUint32)sizeof(*header);
}

void gopfb_finalize_display_info(
    void *displayInfo,
    GopFbSize displayInfoSize) {
    GopFramebufferDisplayInfoHeader *header;

    if ((displayInfo == NULL) ||
        (displayInfoSize < sizeof(GopFramebufferDisplayInfoHeader))) {
        return;
    }

    header = (GopFramebufferDisplayInfoHeader *)displayInfo;
    header->crc32 = gopfb_crc32_with_zeroed_field(
        displayInfo,
        displayInfoSize,
        GOPFB_OFFSET_OF(GopFramebufferDisplayInfoHeader, crc32),
        sizeof(header->crc32));
}

GopFramebufferDisplayInfoStatus gopfb_validate_display_info(
    const void *displayInfo,
    GopFbSize displayInfoSize,
    const GopFramebufferModeDescriptor **modes,
    const GopFbUint8 **edid) {
    const GopFramebufferDisplayInfoHeader *header;
    const GopFramebufferModeDescriptor *modeArray;
    const GopFbUint8 *edidBytes;
    GopFbSize modeBytes;
    GopFbSize expectedEdidOffset;
    GopFbSize expectedTotalSize;
    GopFbSize index;
    GopFbSize otherIndex;
    GopFramebufferEdidStatus edidStatus;

    if (modes != NULL) {
        *modes = NULL;
    }
    if (edid != NULL) {
        *edid = NULL;
    }

    if ((displayInfo == NULL) ||
        (displayInfoSize < sizeof(GopFramebufferDisplayInfoHeader))) {
        return GOPFB_DISPLAY_STATUS_NULL;
    }

    header = (const GopFramebufferDisplayInfoHeader *)displayInfo;
    if (header->magic != GOPFB_DISPLAY_INFO_MAGIC) {
        return GOPFB_DISPLAY_STATUS_BAD_MAGIC;
    }
    if (header->version != GOPFB_DISPLAY_INFO_VERSION) {
        return GOPFB_DISPLAY_STATUS_BAD_VERSION;
    }
    if ((header->headerSize !=
         (GopFbUint16)sizeof(GopFramebufferDisplayInfoHeader)) ||
        (header->totalSize != displayInfoSize)) {
        return GOPFB_DISPLAY_STATUS_BAD_STRUCTURE_SIZE;
    }

    for (index = 0U;
         index < (sizeof(header->reserved) / sizeof(header->reserved[0]));
         ++index) {
        if (header->reserved[index] != GOPFB_U32_C(0)) {
            return GOPFB_DISPLAY_STATUS_RESERVED_NOT_ZERO;
        }
    }

    if (header->crc32 != gopfb_crc32_with_zeroed_field(
                             displayInfo,
                             displayInfoSize,
                             GOPFB_OFFSET_OF(
                                 GopFramebufferDisplayInfoHeader,
                                 crc32),
                             sizeof(header->crc32))) {
        return GOPFB_DISPLAY_STATUS_BAD_CHECKSUM;
    }

    if ((header->flags & ~GOPFB_DISPLAY_KNOWN_FLAGS) != GOPFB_U32_C(0)) {
        return GOPFB_DISPLAY_STATUS_BAD_FLAGS;
    }

    if ((header->flags & GOPFB_DISPLAY_KNOWN_FLAGS) ==
        GOPFB_DISPLAY_KNOWN_FLAGS) {
        return GOPFB_DISPLAY_STATUS_BAD_FLAGS;
    }

    if ((header->modeCount == GOPFB_U32_C(0)) ||
        !gopfb_size_multiply(
            (GopFbSize)header->modeCount,
            sizeof(GopFramebufferModeDescriptor),
            &modeBytes) ||
        !gopfb_size_add(
            sizeof(GopFramebufferDisplayInfoHeader),
            modeBytes,
            &expectedEdidOffset) ||
        !gopfb_size_add(
            expectedEdidOffset,
            (GopFbSize)header->edidSize,
            &expectedTotalSize)) {
        return GOPFB_DISPLAY_STATUS_BAD_LAYOUT;
    }

    if ((header->modesOffset !=
         (GopFbUint32)sizeof(GopFramebufferDisplayInfoHeader)) ||
        (header->edidOffset != expectedEdidOffset) ||
        (expectedTotalSize != displayInfoSize)) {
        return GOPFB_DISPLAY_STATUS_BAD_LAYOUT;
    }

    if (header->currentModeArrayIndex >= header->modeCount) {
        return GOPFB_DISPLAY_STATUS_BAD_CURRENT_MODE;
    }

    modeArray = (const GopFramebufferModeDescriptor *)(const void *)(header + 1);
    for (index = 0U; index < (GopFbSize)header->modeCount; ++index) {
        if (gopfb_validate_mode_descriptor(&modeArray[index]) !=
            GOPFB_STATUS_OK) {
            return GOPFB_DISPLAY_STATUS_INVALID_MODE;
        }

        for (otherIndex = index + 1U;
             otherIndex < (GopFbSize)header->modeCount;
             ++otherIndex) {
            if (modeArray[index].modeNumber ==
                modeArray[otherIndex].modeNumber) {
                return GOPFB_DISPLAY_STATUS_DUPLICATE_MODE_NUMBER;
            }
        }
    }

    edidBytes = header->edidSize == GOPFB_U32_C(0)
                    ? NULL
                    : ((const GopFbUint8 *)displayInfo + header->edidOffset);
    edidStatus = gopfb_validate_edid(edidBytes, (GopFbSize)header->edidSize);
    if (header->edidSize == GOPFB_U32_C(0)) {
        if ((edidStatus != GOPFB_EDID_STATUS_ABSENT) ||
            ((header->flags & GOPFB_DISPLAY_KNOWN_FLAGS) != GOPFB_U32_C(0))) {
            return GOPFB_DISPLAY_STATUS_INVALID_EDID;
        }
    } else if ((edidStatus != GOPFB_EDID_STATUS_OK) ||
               ((header->flags & GOPFB_DISPLAY_KNOWN_FLAGS) ==
                GOPFB_U32_C(0))) {
        return GOPFB_DISPLAY_STATUS_INVALID_EDID;
    }

    if (modes != NULL) {
        *modes = modeArray;
    }
    if (edid != NULL) {
        *edid = edidBytes;
    }

    return GOPFB_DISPLAY_STATUS_OK;
}

const char *gopfb_display_status_string(
    GopFramebufferDisplayInfoStatus status) {
    switch (status) {
        case GOPFB_DISPLAY_STATUS_OK:
            return "ok";
        case GOPFB_DISPLAY_STATUS_NULL:
            return "null or short display info";
        case GOPFB_DISPLAY_STATUS_BAD_MAGIC:
            return "bad magic";
        case GOPFB_DISPLAY_STATUS_BAD_VERSION:
            return "unsupported version";
        case GOPFB_DISPLAY_STATUS_BAD_STRUCTURE_SIZE:
            return "bad structure size";
        case GOPFB_DISPLAY_STATUS_RESERVED_NOT_ZERO:
            return "reserved field is not zero";
        case GOPFB_DISPLAY_STATUS_BAD_CHECKSUM:
            return "bad checksum";
        case GOPFB_DISPLAY_STATUS_BAD_FLAGS:
            return "bad flags";
        case GOPFB_DISPLAY_STATUS_BAD_LAYOUT:
            return "bad variable layout";
        case GOPFB_DISPLAY_STATUS_BAD_CURRENT_MODE:
            return "bad current mode index";
        case GOPFB_DISPLAY_STATUS_DUPLICATE_MODE_NUMBER:
            return "duplicate mode number";
        case GOPFB_DISPLAY_STATUS_INVALID_MODE:
            return "invalid mode descriptor";
        case GOPFB_DISPLAY_STATUS_INVALID_EDID:
            return "invalid EDID";
        default:
            return "unknown display info status";
    }
}

void gopfb_initialize_mode_request(GopFramebufferModeRequest *request) {
    GopFbSize index;
    GopFbUint8 *bytes;

    if (request == NULL) {
        return;
    }

    bytes = (GopFbUint8 *)request;
    for (index = 0U; index < sizeof(*request); ++index) {
        bytes[index] = GOPFB_U8_C(0);
    }

    request->magic = GOPFB_MODE_REQUEST_MAGIC;
    request->version = GOPFB_MODE_REQUEST_VERSION;
    request->totalSize = (GopFbUint16)sizeof(*request);
}

void gopfb_finalize_mode_request(GopFramebufferModeRequest *request) {
    if (request == NULL) {
        return;
    }

    request->crc32 = gopfb_crc32_with_zeroed_field(
        request,
        sizeof(*request),
        GOPFB_OFFSET_OF(GopFramebufferModeRequest, crc32),
        sizeof(request->crc32));
}

GopFramebufferModeRequestStatus gopfb_validate_mode_request(
    const GopFramebufferModeRequest *request) {
    GopFbSize index;

    if (request == NULL) {
        return GOPFB_MODE_REQUEST_STATUS_NULL;
    }
    if (request->magic != GOPFB_MODE_REQUEST_MAGIC) {
        return GOPFB_MODE_REQUEST_STATUS_BAD_MAGIC;
    }
    if (request->version != GOPFB_MODE_REQUEST_VERSION) {
        return GOPFB_MODE_REQUEST_STATUS_BAD_VERSION;
    }
    if (request->totalSize != (GopFbUint16)sizeof(*request)) {
        return GOPFB_MODE_REQUEST_STATUS_BAD_STRUCTURE_SIZE;
    }

    for (index = 0U;
         index < (sizeof(request->reserved) / sizeof(request->reserved[0]));
         ++index) {
        if (request->reserved[index] != GOPFB_U32_C(0)) {
            return GOPFB_MODE_REQUEST_STATUS_RESERVED_NOT_ZERO;
        }
    }

    if (request->crc32 != gopfb_crc32_with_zeroed_field(
                              request,
                              sizeof(*request),
                              GOPFB_OFFSET_OF(
                                  GopFramebufferModeRequest,
                                  crc32),
                              sizeof(request->crc32))) {
        return GOPFB_MODE_REQUEST_STATUS_BAD_CHECKSUM;
    }

    if ((request->flags == GOPFB_U32_C(0)) ||
        ((request->flags & ~GOPFB_MODE_REQUEST_KNOWN_FLAGS) !=
         GOPFB_U32_C(0))) {
        return GOPFB_MODE_REQUEST_STATUS_BAD_FLAGS;
    }

    if (((request->flags & GOPFB_MODE_REQUEST_BY_NUMBER) == GOPFB_U32_C(0)) &&
        (request->modeNumber != GOPFB_U32_C(0))) {
        return GOPFB_MODE_REQUEST_STATUS_BAD_TARGET;
    }

    if ((request->flags & GOPFB_MODE_REQUEST_BY_DIMENSIONS) !=
        GOPFB_U32_C(0)) {
        if ((request->width == GOPFB_U32_C(0)) ||
            (request->height == GOPFB_U32_C(0))) {
            return GOPFB_MODE_REQUEST_STATUS_BAD_TARGET;
        }
    } else if ((request->width != GOPFB_U32_C(0)) ||
               (request->height != GOPFB_U32_C(0))) {
        return GOPFB_MODE_REQUEST_STATUS_BAD_TARGET;
    }

    return GOPFB_MODE_REQUEST_STATUS_OK;
}

const char *gopfb_mode_request_status_string(
    GopFramebufferModeRequestStatus status) {
    switch (status) {
        case GOPFB_MODE_REQUEST_STATUS_OK:
            return "ok";
        case GOPFB_MODE_REQUEST_STATUS_NULL:
            return "null request";
        case GOPFB_MODE_REQUEST_STATUS_BAD_MAGIC:
            return "bad magic";
        case GOPFB_MODE_REQUEST_STATUS_BAD_VERSION:
            return "unsupported version";
        case GOPFB_MODE_REQUEST_STATUS_BAD_STRUCTURE_SIZE:
            return "bad structure size";
        case GOPFB_MODE_REQUEST_STATUS_RESERVED_NOT_ZERO:
            return "reserved field is not zero";
        case GOPFB_MODE_REQUEST_STATUS_BAD_CHECKSUM:
            return "bad checksum";
        case GOPFB_MODE_REQUEST_STATUS_BAD_FLAGS:
            return "bad flags";
        case GOPFB_MODE_REQUEST_STATUS_BAD_TARGET:
            return "bad target";
        default:
            return "unknown mode request status";
    }
}
