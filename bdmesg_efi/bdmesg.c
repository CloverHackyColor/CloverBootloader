#include <Library/UefiLib.h>
#include <Library/ShellLib.h>
#include <Library/MemLogLib.h>

EFI_STATUS
EFIAPI
BdmesgMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
	static CHAR16 const NoMemLog[] = L"%EUnsuccessful getting memory log%N\n";
	static CHAR16 const Usage[] = L"%HUsage: bdmesg [-b]\n  -b: paginate%N\n";
	CHAR8 const* log;
	LIST_ENTRY* Package;
	UINTN logLength, numPrinted;
	EFI_STATUS Status;
	BOOLEAN SkipLn;

	logLength = GetMemLogLen();
	log = GetMemLogBuffer();
	if (!log) {
		ShellPrintEx(-1, -1, &NoMemLog[0]);
		return EFI_NOT_FOUND;
	}
	Status = ShellCommandLineParseEx(&EmptyParamList[0], &Package, NULL, TRUE, FALSE);
	if (EFI_ERROR(Status)) {
		ShellPrintEx(-1, -1, &Usage[0]);
		return EFI_INVALID_PARAMETER;
	}
	ShellCommandLineFreeVarList(Package);
	Status = EFI_SUCCESS;
	SkipLn = TRUE;
	while (logLength) {
		numPrinted = Print(L"%.*a", logLength, log);
		if (!numPrinted) {
			SkipLn = FALSE;
			Status = EFI_ABORTED;
			break;
		}
		if (ShellGetExecutionBreakFlag()) {
			Status = EFI_ABORTED;
			break;
		}
		if (numPrinted >= logLength)
			break;
		logLength -= numPrinted;
		log += numPrinted;
	}
	ShellSetPageBreakMode(FALSE);
	if (SkipLn)
		Print(L"\n");
	return Status;
}
