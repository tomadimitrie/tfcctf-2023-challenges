#include <sal.h>
#include <ntifs.h>
#include <ntddk.h>
#include <wdm.h>

VOID
WriteFlag(
	_In_ PVOID Buffer,
	_In_ ULONG Size,
	_In_ UINT8 NewLine
)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	OBJECT_ATTRIBUTES flagFile = { 0 };
	UNICODE_STRING filePath = RTL_CONSTANT_STRING(L"\\??\\c:\\flag.txt");
	IO_STATUS_BLOCK ioStatus;
	HANDLE hFile = NULL;
	LARGE_INTEGER position = { 0 };
	position.HighPart = -1;
	position.LowPart = FILE_WRITE_TO_END_OF_FILE;

	InitializeObjectAttributes(&flagFile, &filePath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

	status = ZwCreateFile(&hFile, FILE_APPEND_DATA, &flagFile, &ioStatus, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	if (!NT_SUCCESS(status))
	{
		goto exit;
	}

	status = ZwWriteFile(hFile, NULL, NULL, NULL, &ioStatus, Buffer, Size, &position, NULL);
	if (!NT_SUCCESS(status))
	{
		goto exit;
	}

	if (NewLine)
	{
		status = ZwWriteFile(hFile, NULL, NULL, NULL, &ioStatus, "\n", 1, &position, NULL);
		if (!NT_SUCCESS(status))
		{
			goto exit;
		}
	}

	status = STATUS_SUCCESS;
exit:
	if (hFile)
	{
		ZwClose(hFile);
	}
}

NTSTATUS
GetXorBytes(
	_Out_ CHAR Bytes[2]
)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	OBJECT_ATTRIBUTES ntdllFile = { 0 };
	UNICODE_STRING ntdllPath = RTL_CONSTANT_STRING(L"\\??\\c:\\windows\\system32\\ntdll.dll");
	IO_STATUS_BLOCK ioStatus;
	HANDLE hFile = NULL;

	InitializeObjectAttributes(&ntdllFile, &ntdllPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

	status = ZwCreateFile(&hFile, FILE_GENERIC_READ, &ntdllFile, &ioStatus, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	if (!NT_SUCCESS(status))
	{
		goto exit;
	}

	status = ZwReadFile(hFile, NULL, NULL, NULL, &ioStatus, Bytes, sizeof(Bytes), NULL, NULL);
	if (!NT_SUCCESS(status))
	{
		goto exit;
	}

	status = STATUS_SUCCESS;
exit:
	if (hFile)
	{
		ZwClose(hFile);
	}
	return status;
}

VOID
GetFinalComponent(
	_In_ PUNICODE_STRING Path,
	_Out_ PUNICODE_STRING FinalComponent
)
{
	UNICODE_STRING current = { 0 };
	UNICODE_STRING remaining = { 0 };

	current = *Path;

	for (;;)
	{
		FsRtlDissectName(current, FinalComponent, &remaining);
		if (remaining.Length == 0)
		{
			break;
		}
		current = remaining;
	}
}

VOID
Xor2(
	_In_ PCHAR Data,
	_In_ ULONG Length,
	_In_ CHAR Key[2]
)
{
	INT current = 0;

	for (ULONG i = 0; i < Length; i += 1)
	{
		Data[i] ^= Key[current];
		current = (1 - current);
	}
}

VOID
ProcessCreate(
	_In_ HANDLE ParentId,
	_In_ HANDLE ProcessId,
	_In_ BOOLEAN Create
)
{
	UNREFERENCED_PARAMETER(ParentId);
	UNREFERENCED_PARAMETER(ProcessId);
	UNREFERENCED_PARAMETER(Create);

	if (!Create)
	{
		return;
	}

	NTSTATUS status = STATUS_UNSUCCESSFUL;

	PEPROCESS child = NULL;
	PUNICODE_STRING childPath = NULL;
	UNICODE_STRING childName = { 0 };

	PEPROCESS parent = NULL;
	PUNICODE_STRING parentPath = NULL;
	UNICODE_STRING parentName = { 0 };

	CHAR key[2] = { 0 };

	CHAR parentExpected[] = { 0x7d, 0x63, 0x7b, 0x6d, 0x2e, 0x3f, 0x7a, 0x3c, 0x7a, 0x39, 0x74, 0x3f, 0x7a, 0x3f, 0x7f, 0x3f, 0x7f, 0x62, 0x2f, 0x39, 0x2c, 0x38, 0x7a, 0x63, 0x2e, 0x63, 0x7f, 0x6b, 0x7e, 0x63, 0x75, 0x38 };
	ANSI_STRING parentExpectedAnsi = { sizeof(parentExpected), sizeof(parentExpected), parentExpected };
	WCHAR parentExpectedBuffer[sizeof(parentExpected) + 1] = { 0 };
	UNICODE_STRING parentExpectedUnicode = { sizeof(parentExpectedBuffer), sizeof(parentExpectedBuffer), parentExpectedBuffer };

	CHAR childExpected[] = { 0x3b, 0x74, 0x68, 0x29, 0x3e, 0x7e, 0x3e, 0x74, 0x3c, 0x2e, 0x6c, 0x7d, 0x6e, 0x78, 0x3b, 0x78, 0x6e, 0x7b, 0x3c, 0x79, 0x39, 0x79, 0x6b, 0x7e, 0x6a, 0x7f, 0x6f, 0x7f, 0x38, 0x2b, 0x63, 0x2b };
	ANSI_STRING childExpectedAnsi = { sizeof(childExpected), sizeof(childExpected), childExpected };
	WCHAR childExpectedBuffer[sizeof(childExpected) + 1] = { 0 };
	UNICODE_STRING childExpectedUnicode = { sizeof(childExpectedBuffer), sizeof(childExpectedBuffer), childExpectedBuffer };

	status = GetXorBytes(key);
	if (!NT_SUCCESS(status))
	{
		goto exit;
	}

	Xor2(parentExpected, sizeof(parentExpected), key);

	key[0] ^= key[1];
	key[1] ^= key[0];
	key[0] ^= key[1];

	Xor2(childExpected, sizeof(childExpected), key);

	status = RtlAnsiStringToUnicodeString(&parentExpectedUnicode, &parentExpectedAnsi, FALSE);
	if (!NT_SUCCESS(status))
	{
		goto exit;
	}

	status = RtlAnsiStringToUnicodeString(&childExpectedUnicode, &childExpectedAnsi, FALSE);
	if (!NT_SUCCESS(status))
	{
		goto exit;
	}

	status = PsLookupProcessByProcessId(ProcessId, &child);
	if (!NT_SUCCESS(status))
	{
		goto exit;
	}
	status = SeLocateProcessImageName(child, &childPath);
	if (!NT_SUCCESS(status))
	{
		goto exit;
	}
	GetFinalComponent(childPath, &childName);
	if (childName.Length <= 4 * sizeof(WCHAR) || childName.MaximumLength <= 4 * sizeof(WCHAR))
	{
		status = STATUS_SUCCESS;
		goto exit;
	}
	childName.Length -= 4 * sizeof(WCHAR);
	childName.MaximumLength -= 4 * sizeof(WCHAR);

	status = PsLookupProcessByProcessId(ParentId, &parent);
	if (!NT_SUCCESS(status))
	{
		goto exit;
	}
	status = SeLocateProcessImageName(parent, &parentPath);
	if (!NT_SUCCESS(status))
	{
		goto exit;
	}
	GetFinalComponent(parentPath, &parentName);
	if (parentName.Length <= 4 * sizeof(WCHAR) || parentName.MaximumLength <= 4 * sizeof(WCHAR))
	{
		status = STATUS_SUCCESS;
		goto exit;
	}
	parentName.Length -= 4 * sizeof(WCHAR);
	parentName.MaximumLength -= 4 * sizeof(WCHAR);

	if (
		RtlCompareUnicodeString(&childExpectedUnicode, &childName, FALSE) == 0 &&
		RtlCompareUnicodeString(&parentExpectedUnicode, &parentName, FALSE) == 0
	)
	{
		WriteFlag(parentExpectedAnsi.Buffer, parentExpectedAnsi.Length, FALSE);
		WriteFlag(childExpectedAnsi.Buffer, childExpectedAnsi.Length, TRUE);
	}

exit:
	if (child)
	{
		ObDereferenceObject(child);
	}
	if (parent)
	{
		ObDereferenceObject(parent);
	}
	return;
}


VOID
DriverUnload(
	_In_ PDRIVER_OBJECT DriverObject
)
{
	UNREFERENCED_PARAMETER(DriverObject);
	PsSetCreateProcessNotifyRoutine(ProcessCreate, TRUE);
}

NTSTATUS
DriverEntry(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath
)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	DriverObject->DriverUnload = DriverUnload;

	NTSTATUS status = PsSetCreateProcessNotifyRoutine(ProcessCreate, FALSE);
	if (!NT_SUCCESS(status))
	{
		return status;
	}

	return STATUS_SUCCESS;
}