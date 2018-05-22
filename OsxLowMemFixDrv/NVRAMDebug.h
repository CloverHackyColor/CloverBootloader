/**

  NVRAM debug stuff.
  
  by dmazar

**/

//
// For debugging with NVRAM.
// DEBUG_TO_NVRAM = 1 -> enable NVRAM debug
// DEBUG_TO_NVRAM = 0 -> disable NVRAM debug
//
#define DEBUG_TO_NVRAM 0

// Max size of the log.
#define DEBUG_LOG_SIZE				16384 // 16 KB

// Name of the log NVRAM variable.
#define DEBUG_LOG_NVRAM_VAR_NAME	L"DebugLog"

// Apple's boot guid.
#define EFI_APPLE_BOOT_GUID			{0x7C436110, 0xAB2A, 0x4BBB, {0xA8, 0x80, 0xFE, 0x41, 0x99, 0x5C, 0x9F, 0x82}}

// GUID of the log NVRAM variable.
// We'll reuse Apple's boot guid to get access to this var from OS X.
#define DEBUG_LOG_NVRAM_VAR_GUID	EFI_APPLE_BOOT_GUID


//
// DBGnvr macro definition.
// Can be used instead of direct NVRAMDebugLog() call,
// and then NVRAM debug can be enabled/disabled with DEBUG_TO_NVRAM
//
#if DEBUG_TO_NVRAM == 1
#define DBGnvr(...) NVRAMDebugLog(__VA_ARGS__);
#else
#define DBGnvr(...)
#endif


extern EFI_GUID gEfiNVRAMDebugVarGuid;


/** Appends given data to the log and writes it to NVRAM variable. */
EFI_STATUS
EFIAPI
NVRAMDebugLog(CHAR8 *Format, ...);