#include <windows.h>
#include <bcrypt.h>
#include <stdio.h>
#include <string.h>
#include "classicadapter.h"
#include "eventlog.h"

#define CLASSIC_FOG_PATCH_RVA 0xC791
#define CLASSIC_D2SERVER_LANGUAGE_PATCH_RVA 0x4739
#define CLASSIC_100_D2WIN_MPQ_ORDINAL_RVA 0x3BC0
#define CLASSIC_100_FOG_LOG_ORDINAL_RVA 0x3C08
#define CLASSIC_100_FOG_INIT_ORDINAL_RVA 0x3C14
#define CLASSIC_100_FOG_FREE_ORDINAL_RVA 0x3C20
#define CLASSIC_100_FOG_POOL_ORDINAL_RVA 0x3C2C
#define CLASSIC_100_FOG_STATS_ORDINAL_RVA 0x3C38
#define CLASSIC_100_FOG_MEMORY_ORDINAL_RVA 0x3CBC
#define CLASSIC_100_FOG_STATS_CALL_RVA 0x46F0
#define CLASSIC_100_FOG_MEMORY_CALL_RVA 0x480A
#define CLASSIC_100_D2COMMON_LOAD_ORDINAL_RVA 0x3CD4
#define CLASSIC_100_D2COMMON_LOAD_CLEANUP_RVA 0x47B1
#define CLASSIC_100_INIT_EVENT_POINTER_RVA 0x436A
#define CLASSIC_100_CLIENT_DESCRIPTOR_RVA 0x2AB4
#define CLASSIC_100_CLIENT_EXPECTED_RVA 0x3194
#define CLASSIC_100_D2GAME_RECV_GAME_IAT_RVA 0xA4AC8
#define CLASSIC_100_D2GAME_RECV_CONTROL_IAT_RVA 0xA4AD4
#define CLASSIC_100_D2NET_PARSER_THUNK_RVA 0x100A
#define CLASSIC_100_D2NET_ACCEPT_THUNK_RVA 0x1050
#define CLASSIC_100_D2NET_DELIVER_THUNK_RVA 0x1096
#define CLASSIC_100_D2NET_PARSER_RVA 0x2100
#define CLASSIC_100_D2NET_ACCEPT_RVA 0x21C0
#define CLASSIC_100_D2NET_DELIVER_RVA 0x20E0
#define SHA256_LENGTH 32

typedef int (__stdcall * D2NETRECVFUNC)(void *buffer, int length);
typedef int (__fastcall * D2NETPARSERFUNC)(BYTE *data, int length,
	DWORD arg1, DWORD arg2, DWORD arg3, DWORD arg4, DWORD arg5, DWORD arg6);
typedef int (__fastcall * D2NETACCEPTFUNC)(DWORD arg1, DWORD arg2);
typedef int (__fastcall * D2NETDELIVERFUNC)(DWORD arg1, DWORD arg2,
	DWORD arg3);

static BOOL gClassic100Active;
static D2NETRECVFUNC gClassic100RecvGame;
static D2NETRECVFUNC gClassic100RecvControl;
static D2NETPARSERFUNC gClassic100Parser;
static D2NETACCEPTFUNC gClassic100Accept;
static D2NETDELIVERFUNC gClassic100Deliver;
static LONG gClassic100RecvGameSeen;
static LONG gClassic100RecvControlSeen;

static BYTE const D2ServerSha256[SHA256_LENGTH] = {
	0xC6, 0xE2, 0x08, 0xD4, 0x63, 0x0F, 0x9E, 0x7F,
	0x77, 0x2B, 0x64, 0x7D, 0x4D, 0x77, 0xE9, 0xE7,
	0xF5, 0x30, 0xB8, 0xCE, 0x4C, 0xC8, 0x2F, 0x0C,
	0xD8, 0xA9, 0xCD, 0x7C, 0xEA, 0x01, 0xD4, 0x79
};

static BYTE const D2ServerLanguageExpected[] = {0x59, 0x57, 0x33, 0xC9};
static BYTE const D2ServerLanguageReplacement[] = {0x59, 0x56, 0x33, 0xC9};

static BYTE const Classic109FogSha256[SHA256_LENGTH] = {
	0x6C, 0xD7, 0x10, 0x8A, 0x41, 0x5A, 0x02, 0xBF,
	0x1F, 0x77, 0x48, 0x66, 0x69, 0xDC, 0x3A, 0x80,
	0x02, 0x90, 0x0E, 0x5E, 0x80, 0x55, 0x97, 0x93,
	0xC1, 0x87, 0xBD, 0x1C, 0x5C, 0x20, 0x07, 0x99
};

static BYTE const Classic109D2GameSha256[SHA256_LENGTH] = {
	0x1B, 0xC4, 0xEA, 0x52, 0x9B, 0x02, 0xFB, 0x18,
	0xA6, 0xB1, 0x65, 0xF8, 0xD6, 0x68, 0x15, 0x58,
	0x65, 0xB4, 0xF4, 0x95, 0x77, 0xA5, 0x3D, 0xAE,
	0x3D, 0xAC, 0xEA, 0xF3, 0x4F, 0x71, 0x1E, 0xEE
};

static BYTE const Classic100FogSha256[SHA256_LENGTH] = {
	0x1C, 0x6C, 0x1B, 0xD1, 0xB2, 0xFD, 0x0E, 0xDE,
	0x64, 0xDD, 0x80, 0x35, 0xA3, 0x6B, 0xC4, 0xF6,
	0x9A, 0x38, 0x72, 0x19, 0x82, 0x68, 0xF9, 0x98,
	0xCA, 0x2B, 0xBD, 0xD7, 0xAE, 0x7D, 0x8B, 0xE6
};

static BYTE const Classic100D2GameSha256[SHA256_LENGTH] = {
	0x69, 0x50, 0x24, 0xB9, 0xF2, 0xC9, 0xF0, 0xE9,
	0x44, 0x6A, 0x1B, 0x22, 0x4D, 0x82, 0x3D, 0x4A,
	0x8D, 0x5F, 0x94, 0x24, 0xB6, 0xCC, 0x31, 0x49,
	0x53, 0xF0, 0x1A, 0x5A, 0x34, 0xEA, 0xEB, 0xC4
};

static BYTE const Classic100D2ClientSha256[SHA256_LENGTH] = {
	0x07, 0x72, 0x54, 0x69, 0xC3, 0x67, 0x31, 0x04,
	0xD5, 0x94, 0x9A, 0xE2, 0x2C, 0x77, 0xDA, 0x53,
	0xE4, 0x6F, 0xF5, 0xD4, 0xEC, 0x4B, 0xC2, 0x4C,
	0xBA, 0x82, 0xA4, 0x74, 0x37, 0x65, 0x5A, 0x1A
};

static BYTE const Classic100D2WinSha256[SHA256_LENGTH] = {
	0x0F, 0x64, 0xDC, 0x8B, 0xB6, 0x3D, 0x99, 0x62,
	0xFA, 0xCE, 0x34, 0x26, 0xD7, 0x8D, 0x64, 0x5B,
	0x6B, 0x58, 0x1F, 0x33, 0xD0, 0x90, 0xD5, 0xF9,
	0x30, 0x4A, 0xE8, 0x1B, 0x23, 0xFA, 0x0B, 0x8C
};

static BYTE const Classic100D2CommonSha256[SHA256_LENGTH] = {
	0x6E, 0x8B, 0x73, 0x32, 0x5E, 0xC6, 0xB8, 0x85,
	0x21, 0x3E, 0xCC, 0x9A, 0x98, 0x01, 0x7E, 0xC5,
	0xCF, 0x6D, 0x69, 0x84, 0xFF, 0x4A, 0x27, 0x35,
	0xC7, 0x3A, 0x3F, 0x35, 0x95, 0xE0, 0x54, 0x4C
};

static BYTE const Classic109FogExpected[] = {
	0xF6, 0xC1, 0x03,
	0xB8, 0x02, 0x00, 0x00, 0x00,
	0x75, 0x7E,
	0x81, 0xF9, 0xFF, 0xFF, 0xFF, 0x7F,
	0xB8, 0x03, 0x00, 0x00, 0x00,
	0x77, 0x71
};

static BYTE const Classic109FogReplacement[] = {
	0x83, 0xF9, 0xFF,
	0x0F, 0x84, 0xF0, 0x00, 0x00, 0x00,
	0xB8, 0x02, 0x00, 0x00, 0x00,
	0xF6, 0xC1, 0x03,
	0x75, 0x75,
	0x85, 0xC9,
	0x78, 0x71
};

static BYTE const FogOrdinal10019[] = {0x23, 0x27, 0x00, 0x00};
static BYTE const D2WinOrdinal10037[] = {0x35, 0x27, 0x00, 0x00};
static BYTE const D2WinOrdinal10033[] = {0x31, 0x27, 0x00, 0x00};
static BYTE const FogOrdinal10021[] = {0x25, 0x27, 0x00, 0x00};
static BYTE const FogOrdinal10101[] = {0x75, 0x27, 0x00, 0x00};
static BYTE const FogOrdinal10074[] = {0x5A, 0x27, 0x00, 0x00};
static BYTE const FogOrdinal10089[] = {0x69, 0x27, 0x00, 0x00};
static BYTE const FogOrdinal10065[] = {0x51, 0x27, 0x00, 0x00};
static BYTE const FogOrdinal10218[] = {0xEA, 0x27, 0x00, 0x00};
static BYTE const FogOrdinal10185[] = {0xC9, 0x27, 0x00, 0x00};
static BYTE const FogOrdinal10017[] = {0x21, 0x27, 0x00, 0x00};
static BYTE const FogStatsCallExpected[] = {0xFF, 0x15, 0x88, 0x6D, 0x00, 0x68};
static BYTE const FogMemoryCallExpected[] = {0xFF, 0x15, 0xA0, 0x6D, 0x00, 0x68};
static BYTE const OptionalCallReplacement[] = {0x33, 0xC0, 0x90, 0x90, 0x90, 0x90};
static BYTE const D2CommonOrdinal10576[] = {0x50, 0x29, 0x00, 0x00};
static BYTE const D2CommonOrdinal10554[] = {0x3A, 0x29, 0x00, 0x00};
static BYTE const D2CommonLoadResultStore[] = {0x89, 0x45, 0xF8};
static BYTE const D2CommonLoadCallerCleanup[] = {0x83, 0xC4, 0x0C};
static BYTE const InitEventSavedPointer[] = {0xFF, 0x76, 0x14};
static BYTE const InitEventCanonicalPointer[] = {0xFF, 0x70, 0x14};
static BYTE const ClientDescriptorExpected[] = {0xE1, 0x81, 0x00, 0x00};
static BYTE const Classic100ClientDescriptor[] = {0x9C, 0x5F, 0x01, 0x00};
static BYTE const ClientPointerExpected[] = {0x88, 0x0A, 0xBB, 0x6F};
static BYTE const Classic100ClientPointer[] = {0xA8, 0xEC, 0x12, 0x10};
static DWORD const Classic100DatabaseReturnRvas[] = {
	0x5710, 0x57A2, 0x57E2, 0x5819, 0x584F, 0x58A7
};
static BYTE const Classic100DatabaseReturnExpected[] = {0xC2, 0x14, 0x00};
static BYTE const Classic100DatabaseReturnReplacement[] = {0xC2, 0x1C, 0x00};

static BOOL HashFileSha256(LPCSTR fileName, BYTE hash[SHA256_LENGTH])
{
	BCRYPT_ALG_HANDLE algorithm;
	BCRYPT_HASH_HANDLE hashHandle;
	BYTE *hashObject;
	BYTE buffer[64 * 1024];
	DWORD objectLength;
	DWORD propertyLength;
	DWORD bytesRead;
	HANDLE file;
	NTSTATUS status;
	BOOL result;

	algorithm = NULL;
	hashHandle = NULL;
	hashObject = NULL;
	file = INVALID_HANDLE_VALUE;
	result = FALSE;
	status = BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM,
		NULL, 0);
	if (!BCRYPT_SUCCESS(status)) goto cleanup;
	status = BCryptGetProperty(algorithm, BCRYPT_OBJECT_LENGTH,
		(PUCHAR)&objectLength, sizeof(objectLength), &propertyLength, 0);
	if (!BCRYPT_SUCCESS(status)) goto cleanup;
	hashObject = HeapAlloc(GetProcessHeap(), 0, objectLength);
	if (!hashObject) goto cleanup;
	status = BCryptCreateHash(algorithm, &hashHandle, hashObject,
		objectLength, NULL, 0, 0);
	if (!BCRYPT_SUCCESS(status)) goto cleanup;
	file = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE) goto cleanup;
	for (;;) {
		if (!ReadFile(file, buffer, sizeof(buffer), &bytesRead, NULL)) goto cleanup;
		if (!bytesRead) break;
		status = BCryptHashData(hashHandle, buffer, bytesRead, 0);
		if (!BCRYPT_SUCCESS(status)) goto cleanup;
	}
	status = BCryptFinishHash(hashHandle, hash, SHA256_LENGTH, 0);
	result = BCRYPT_SUCCESS(status);

cleanup:
	if (file != INVALID_HANDLE_VALUE) CloseHandle(file);
	if (hashHandle) BCryptDestroyHash(hashHandle);
	if (hashObject) HeapFree(GetProcessHeap(), 0, hashObject);
	if (algorithm) BCryptCloseAlgorithmProvider(algorithm, 0);
	return result;
}

static BOOL IsEnabled(LPCSTR variableName)
{
	CHAR value[2];

	return GetEnvironmentVariableA(variableName, value, sizeof(value)) == 1 &&
		value[0] == '1';
}

static BOOL VerifyBytes(HMODULE module, DWORD rva, BYTE const *expected,
		DWORD length, LPCSTR description)
{
	if (!memcmp((BYTE *)module+rva, expected, length)) return TRUE;
	D2GSEventLog("ClassicAdapter", "%s bytes do not match", description);
	return FALSE;
}

static BOOL PatchBytes(HMODULE module, DWORD rva, BYTE const *replacement,
		DWORD length, LPCSTR description)
{
	BYTE *patchAddress;
	DWORD oldProtection;
	DWORD ignoredProtection;

	patchAddress = (BYTE *)module+rva;
	if (!VirtualProtect(patchAddress, length, PAGE_EXECUTE_READWRITE,
			&oldProtection)) {
		D2GSEventLog("ClassicAdapter", "Failed making %s writable. Code: %lu",
			description, GetLastError());
		return FALSE;
	}
	memcpy(patchAddress, replacement, length);
	FlushInstructionCache(GetCurrentProcess(), patchAddress, length);
	if (!VirtualProtect(patchAddress, length, oldProtection,
			&ignoredProtection)) {
		D2GSEventLog("ClassicAdapter",
			"Failed restoring %s page protection. Code: %lu",
			description, GetLastError());
		return FALSE;
	}
	return TRUE;
}

static void TraceD2NetPacket(LPCSTR queue, void const *buffer, int length)
{
	BYTE const *bytes;
	CHAR text[3 * 40 + 1];
	DWORD available;
	DWORD i;
	DWORD offset;

	if (!buffer || length < 0) return;
	bytes = buffer;
	available = (DWORD)length + sizeof(DWORD);
	if (available > 40) available = 40;
	offset = 0;
	for (i = 0; i < available; i++) {
		offset += sprintf(text+offset, i ? " %02X" : "%02X", bytes[i]);
	}
	D2GSEventLog("ClassicAdapterTrace", "%s queue returned %d bytes: %s",
		queue, length, text);
}

static int __stdcall Classic100RecvGame(void *buffer, int length)
{
	int result;

	result = gClassic100RecvGame(buffer, length);
	if (InterlockedCompareExchange(&gClassic100RecvGameSeen, 1, 0) == 0)
		D2GSEventLog("ClassicAdapterTrace",
			"D2Game began polling the D2Net game queue");
	TraceD2NetPacket("game", buffer, result);
	return result;
}

static int __stdcall Classic100RecvControl(void *buffer, int length)
{
	int result;

	result = gClassic100RecvControl(buffer, length);
	if (InterlockedCompareExchange(&gClassic100RecvControlSeen, 1, 0) == 0)
		D2GSEventLog("ClassicAdapterTrace",
			"D2Game began polling the D2Net control queue");
	TraceD2NetPacket("control", buffer, result);
	return result;
}

static BOOL PatchImportPointer(HMODULE module, DWORD rva, FARPROC expected,
		FARPROC replacement, LPCSTR description)
{
	FARPROC *slot;
	DWORD oldProtection;
	DWORD ignoredProtection;

	slot = (FARPROC *)((BYTE *)module+rva);
	if (*slot != expected) {
		D2GSEventLog("ClassicAdapter", "%s target does not match", description);
		return FALSE;
	}
	if (!VirtualProtect(slot, sizeof(*slot), PAGE_READWRITE, &oldProtection)) {
		D2GSEventLog("ClassicAdapter", "Failed making %s writable. Code: %lu",
			description, GetLastError());
		return FALSE;
	}
	*slot = replacement;
	if (!VirtualProtect(slot, sizeof(*slot), oldProtection,
			&ignoredProtection)) {
		D2GSEventLog("ClassicAdapter",
			"Failed restoring %s page protection. Code: %lu",
			description, GetLastError());
		return FALSE;
	}
	return TRUE;
}

static BOOL PatchRelativeJump(HMODULE module, DWORD rva, FARPROC expected,
		FARPROC replacement, LPCSTR description)
{
	BYTE *instruction;
	BYTE replacementBytes[5];
	DWORD target;

	instruction = (BYTE *)module+rva;
	if (instruction[0] != 0xE9) {
		D2GSEventLog("ClassicAdapter", "%s is not a relative jump",
			description);
		return FALSE;
	}
	target = (DWORD)(instruction+5) + *(DWORD *)(instruction+1);
	if ((FARPROC)target != expected) {
		D2GSEventLog("ClassicAdapter", "%s target does not match",
			description);
		return FALSE;
	}
	replacementBytes[0] = 0xE9;
	*(DWORD *)(replacementBytes+1) =
		(DWORD)(BYTE *)replacement - (DWORD)(instruction+5);
	return PatchBytes(module, rva, replacementBytes,
		sizeof(replacementBytes), description);
}

static BOOL PatchClassic100DatabaseReturns(HMODULE d2Game)
{
	DWORD i;

	for (i = 0; i < ARRAYSIZE(Classic100DatabaseReturnRvas); i++) {
		if (!VerifyBytes(d2Game, Classic100DatabaseReturnRvas[i],
				Classic100DatabaseReturnExpected,
				sizeof(Classic100DatabaseReturnExpected),
				"D2Game.dll database-character return")) return FALSE;
	}
	for (i = 0; i < ARRAYSIZE(Classic100DatabaseReturnRvas); i++) {
		if (!PatchBytes(d2Game, Classic100DatabaseReturnRvas[i],
				Classic100DatabaseReturnReplacement,
				sizeof(Classic100DatabaseReturnReplacement),
				"D2Game.dll database-character return")) return FALSE;
	}
	D2GSEventLog("ClassicAdapter",
		"Adapted classic 1.00 database-character export cleanup");
	return TRUE;
}

static int __fastcall Classic100Parser(BYTE *data, int length,
	DWORD arg1, DWORD arg2, DWORD arg3, DWORD arg4, DWORD arg5, DWORD arg6)
{
	CHAR text[3 * 16 + 1];
	DWORD available;
	DWORD i;
	DWORD offset;
	int result;

	available = length > 0 ? (DWORD)length : 0;
	if (available > 16) available = 16;
	offset = 0;
	for (i = 0; i < available; i++)
		offset += sprintf(text+offset, i ? " %02X" : "%02X", data[i]);
	result = gClassic100Parser(data, length, arg1, arg2, arg3, arg4, arg5,
		arg6);
	D2GSEventLog("ClassicAdapterTrace",
		"D2Net parser returned %d for %d bytes: %s", result, length,
		available ? text : "<empty>");
	return result;
}

static BOOL PatchClassic100Parser(HMODULE d2Net)
{
	static BYTE const expected[] = {0x83, 0xFA, 0x04, 0x53, 0x73, 0x09};
	BYTE replacement[sizeof(expected)];
	BYTE *target;
	BYTE *trampoline;

	target = (BYTE *)d2Net+CLASSIC_100_D2NET_PARSER_RVA;
	if (memcmp(target, expected, sizeof(expected))) {
		D2GSEventLog("ClassicAdapter",
			"D2Net.dll parser prologue bytes do not match");
		return FALSE;
	}
	trampoline = VirtualAlloc(NULL, 15, MEM_COMMIT | MEM_RESERVE,
		PAGE_EXECUTE_READWRITE);
	if (!trampoline) {
		D2GSEventLog("ClassicAdapter",
			"Failed allocating D2Net parser trampoline. Code: %lu",
			GetLastError());
		return FALSE;
	}
	memcpy(trampoline, expected, 4);
	trampoline[4] = 0x0F;
	trampoline[5] = 0x83;
	*(DWORD *)(trampoline+6) = (DWORD)(target+15) -
		(DWORD)(trampoline+10);
	trampoline[10] = 0xE9;
	*(DWORD *)(trampoline+11) = (DWORD)(target+6) -
		(DWORD)(trampoline+15);
	FlushInstructionCache(GetCurrentProcess(), trampoline, 15);
	gClassic100Parser = (D2NETPARSERFUNC)trampoline;
	replacement[0] = 0xE9;
	*(DWORD *)(replacement+1) = (DWORD)(BYTE *)Classic100Parser -
		(DWORD)(target+5);
	replacement[5] = 0x90;
	return PatchBytes(d2Net, CLASSIC_100_D2NET_PARSER_RVA, replacement,
		sizeof(replacement), "D2Net.dll parser entry");
}

static int __fastcall Classic100Accept(DWORD arg1, DWORD arg2)
{
	int result;

	result = gClassic100Accept(arg1, arg2);
	D2GSEventLog("ClassicAdapterTrace",
		"D2Net accept callback returned %d for client %lu", result, arg2);
	return result;
}

static int __fastcall Classic100Deliver(DWORD arg1, DWORD arg2, DWORD arg3)
{
	int result;

	result = gClassic100Deliver(arg1, arg2, arg3);
	D2GSEventLog("ClassicAdapterTrace",
		"D2Net delivery callback returned %d for client %lu", result, arg2);
	return result;
}

static void FormatHash(BYTE const hash[SHA256_LENGTH],
		CHAR hashText[(SHA256_LENGTH * 2) + 1])
{
	DWORD i;

	for (i = 0; i < SHA256_LENGTH; i++)
		sprintf(hashText+(i*2), "%02X", hash[i]);
	hashText[SHA256_LENGTH*2] = '\0';
}

extern BOOL ClassicAdapterApply(D2GSCALLBACKABI *callbackAbi)
{
	BYTE d2GameHash[SHA256_LENGTH];
	BYTE d2ClientHash[SHA256_LENGTH];
	BYTE d2CommonHash[SHA256_LENGTH];
	BYTE d2WinHash[SHA256_LENGTH];
	BYTE fogHash[SHA256_LENGTH];
	CHAR hashText[(SHA256_LENGTH * 2) + 1];
	HMODULE d2server;
	HMODULE fog;
	BOOL is100;

	if (!callbackAbi) return FALSE;
	gClassic100Active = FALSE;
	fog = NULL;
	if (!HashFileSha256("d2server.dll", fogHash) ||
			memcmp(fogHash, D2ServerSha256, sizeof(fogHash))) {
		D2GSEventLog("ClassicAdapter",
			"d2server.dll does not match the supported build output");
		return FALSE;
	}
	if (!HashFileSha256("D2Game.dll", d2GameHash)) {
		D2GSEventLog("ClassicAdapter", "Failed hashing D2Game.dll");
		return FALSE;
	}
	is100 = !memcmp(d2GameHash, Classic100D2GameSha256, sizeof(d2GameHash));
	if (!is100 && memcmp(d2GameHash, Classic109D2GameSha256, sizeof(d2GameHash))) {
		FormatHash(d2GameHash, hashText);
		D2GSEventLog("ClassicAdapter",
			"D2Game.dll SHA-256 is unsupported: %s", hashText);
		return FALSE;
	}
	if (is100) {
		if (!IsEnabled("D2GS_EXPERIMENTAL_CLASSIC_100")) {
			D2GSEventLog("ClassicAdapter",
				"Classic 1.00 runtime detected; set D2GS_EXPERIMENTAL_CLASSIC_100=1 to enable its experimental profile");
			return FALSE;
		}
		if (!HashFileSha256("D2Client.dll", d2ClientHash)) {
			D2GSEventLog("ClassicAdapter", "Failed hashing D2Client.dll");
			return FALSE;
		}
		if (memcmp(d2ClientHash, Classic100D2ClientSha256,
				sizeof(d2ClientHash))) {
			FormatHash(d2ClientHash, hashText);
			D2GSEventLog("ClassicAdapter",
				"D2Client.dll SHA-256 is unsupported: %s", hashText);
			return FALSE;
		}
		if (!HashFileSha256("D2Common.dll", d2CommonHash)) {
			D2GSEventLog("ClassicAdapter", "Failed hashing D2Common.dll");
			return FALSE;
		}
		if (memcmp(d2CommonHash, Classic100D2CommonSha256,
				sizeof(d2CommonHash))) {
			FormatHash(d2CommonHash, hashText);
			D2GSEventLog("ClassicAdapter",
				"D2Common.dll SHA-256 is unsupported: %s", hashText);
			return FALSE;
		}
		if (!HashFileSha256("D2Win.dll", d2WinHash)) {
			D2GSEventLog("ClassicAdapter", "Failed hashing D2Win.dll");
			return FALSE;
		}
		if (memcmp(d2WinHash, Classic100D2WinSha256, sizeof(d2WinHash))) {
			FormatHash(d2WinHash, hashText);
			D2GSEventLog("ClassicAdapter",
				"D2Win.dll SHA-256 is unsupported: %s", hashText);
			return FALSE;
		}
	} else if (!IsEnabled("D2GS_EXPERIMENTAL_CLASSIC_109")) {
		D2GSEventLog("ClassicAdapter",
			"Classic 1.09 runtime detected; set D2GS_EXPERIMENTAL_CLASSIC_109=1 to enable its experimental profile");
		return FALSE;
	}

	if (!HashFileSha256("Fog.dll", fogHash)) {
		D2GSEventLog("ClassicAdapter", "Failed hashing Fog.dll");
		return FALSE;
	}
	if (memcmp(fogHash, is100 ? Classic100FogSha256 : Classic109FogSha256,
			sizeof(fogHash))) {
		FormatHash(fogHash, hashText);
		D2GSEventLog("ClassicAdapter", "Fog.dll SHA-256 is unsupported: %s",
			hashText);
		return FALSE;
	}

	d2server = GetModuleHandleA("d2server.dll");
	if (!d2server) {
		D2GSEventLog("ClassicAdapter", "d2server.dll is not loaded");
		return FALSE;
	}
	if (!VerifyBytes(d2server, CLASSIC_D2SERVER_LANGUAGE_PATCH_RVA,
			D2ServerLanguageExpected, sizeof(D2ServerLanguageExpected),
			"d2server.dll language-mode call")) return FALSE;
	if (is100) {
		if (!VerifyBytes(d2server, CLASSIC_100_D2WIN_MPQ_ORDINAL_RVA,
				D2WinOrdinal10037, sizeof(D2WinOrdinal10037),
				"d2server.dll D2Win MPQ ordinal") ||
			!VerifyBytes(d2server, CLASSIC_100_FOG_LOG_ORDINAL_RVA,
				FogOrdinal10021, sizeof(FogOrdinal10021),
				"d2server.dll Fog log ordinal") ||
			!VerifyBytes(d2server, CLASSIC_100_FOG_INIT_ORDINAL_RVA,
				FogOrdinal10101, sizeof(FogOrdinal10101),
				"d2server.dll Fog initialization ordinal") ||
			!VerifyBytes(d2server, CLASSIC_100_FOG_FREE_ORDINAL_RVA,
				FogOrdinal10019, sizeof(FogOrdinal10019),
				"d2server.dll Fog free ordinal") ||
			!VerifyBytes(d2server, CLASSIC_100_FOG_POOL_ORDINAL_RVA,
				FogOrdinal10089, sizeof(FogOrdinal10089),
				"d2server.dll Fog pool ordinal") ||
			!VerifyBytes(d2server, CLASSIC_100_FOG_STATS_ORDINAL_RVA,
				FogOrdinal10218, sizeof(FogOrdinal10218),
				"d2server.dll Fog statistics ordinal") ||
			!VerifyBytes(d2server, CLASSIC_100_FOG_MEMORY_ORDINAL_RVA,
				FogOrdinal10185, sizeof(FogOrdinal10185),
				"d2server.dll Fog memory ordinal") ||
			!VerifyBytes(d2server, CLASSIC_100_FOG_STATS_CALL_RVA,
				FogStatsCallExpected, sizeof(FogStatsCallExpected),
				"d2server.dll Fog statistics call") ||
			!VerifyBytes(d2server, CLASSIC_100_FOG_MEMORY_CALL_RVA,
				FogMemoryCallExpected, sizeof(FogMemoryCallExpected),
				"d2server.dll Fog memory call") ||
			!VerifyBytes(d2server, CLASSIC_100_D2COMMON_LOAD_ORDINAL_RVA,
				D2CommonOrdinal10576, sizeof(D2CommonOrdinal10576),
				"d2server.dll D2Common data-table loader ordinal") ||
			!VerifyBytes(d2server, CLASSIC_100_D2COMMON_LOAD_CLEANUP_RVA,
				D2CommonLoadResultStore, sizeof(D2CommonLoadResultStore),
				"d2server.dll D2Common data-table loader cleanup") ||
			!VerifyBytes(d2server, CLASSIC_100_INIT_EVENT_POINTER_RVA,
				InitEventSavedPointer, sizeof(InitEventSavedPointer),
				"d2server.dll initialization event pointer") ||
			!VerifyBytes(d2server, CLASSIC_100_CLIENT_DESCRIPTOR_RVA,
				ClientDescriptorExpected, sizeof(ClientDescriptorExpected),
				"d2server.dll client-singleton descriptor RVA") ||
			!VerifyBytes(d2server, CLASSIC_100_CLIENT_EXPECTED_RVA,
				ClientPointerExpected, sizeof(ClientPointerExpected),
				"d2server.dll client-singleton descriptor bytes")) return FALSE;
	} else {
		fog = LoadLibraryA("Fog.dll");
		if (!fog) {
			D2GSEventLog("ClassicAdapter", "Failed loading Fog.dll. Code: %lu",
				GetLastError());
			return FALSE;
		}
		if (!VerifyBytes(fog, CLASSIC_FOG_PATCH_RVA, Classic109FogExpected,
				sizeof(Classic109FogExpected),
				"Fog.dll critical-section validator")) return FALSE;
	}

	if (!PatchBytes(d2server, CLASSIC_D2SERVER_LANGUAGE_PATCH_RVA,
			D2ServerLanguageReplacement, sizeof(D2ServerLanguageReplacement),
			"d2server.dll language-mode call")) return FALSE;
	if (is100) {
		if (!PatchBytes(d2server, CLASSIC_100_D2WIN_MPQ_ORDINAL_RVA,
				D2WinOrdinal10033, sizeof(D2WinOrdinal10033),
				"d2server.dll D2Win MPQ ordinal") ||
			!PatchBytes(d2server, CLASSIC_100_FOG_LOG_ORDINAL_RVA,
				FogOrdinal10019, sizeof(FogOrdinal10019),
				"d2server.dll Fog log ordinal") ||
			!PatchBytes(d2server, CLASSIC_100_FOG_INIT_ORDINAL_RVA,
				FogOrdinal10074, sizeof(FogOrdinal10074),
				"d2server.dll Fog initialization ordinal") ||
			!PatchBytes(d2server, CLASSIC_100_FOG_FREE_ORDINAL_RVA,
				FogOrdinal10017, sizeof(FogOrdinal10017),
				"d2server.dll Fog free ordinal") ||
			!PatchBytes(d2server, CLASSIC_100_FOG_POOL_ORDINAL_RVA,
				FogOrdinal10065, sizeof(FogOrdinal10065),
				"d2server.dll Fog pool ordinal") ||
			!PatchBytes(d2server, CLASSIC_100_FOG_STATS_ORDINAL_RVA,
				FogOrdinal10017, sizeof(FogOrdinal10017),
				"d2server.dll Fog statistics ordinal") ||
			!PatchBytes(d2server, CLASSIC_100_FOG_MEMORY_ORDINAL_RVA,
				FogOrdinal10017, sizeof(FogOrdinal10017),
				"d2server.dll Fog memory ordinal") ||
			!PatchBytes(d2server, CLASSIC_100_FOG_STATS_CALL_RVA,
				OptionalCallReplacement, sizeof(OptionalCallReplacement),
				"d2server.dll Fog statistics call") ||
			!PatchBytes(d2server, CLASSIC_100_FOG_MEMORY_CALL_RVA,
				OptionalCallReplacement, sizeof(OptionalCallReplacement),
				"d2server.dll Fog memory call") ||
			!PatchBytes(d2server, CLASSIC_100_D2COMMON_LOAD_ORDINAL_RVA,
				D2CommonOrdinal10554, sizeof(D2CommonOrdinal10554),
				"d2server.dll D2Common data-table loader ordinal") ||
			!PatchBytes(d2server, CLASSIC_100_D2COMMON_LOAD_CLEANUP_RVA,
				D2CommonLoadCallerCleanup, sizeof(D2CommonLoadCallerCleanup),
				"d2server.dll D2Common data-table loader cleanup") ||
			!PatchBytes(d2server, CLASSIC_100_INIT_EVENT_POINTER_RVA,
				InitEventCanonicalPointer, sizeof(InitEventCanonicalPointer),
				"d2server.dll initialization event pointer") ||
			!PatchBytes(d2server, CLASSIC_100_CLIENT_DESCRIPTOR_RVA,
				Classic100ClientDescriptor, sizeof(Classic100ClientDescriptor),
				"d2server.dll client-singleton descriptor RVA") ||
			!PatchBytes(d2server, CLASSIC_100_CLIENT_EXPECTED_RVA,
				Classic100ClientPointer, sizeof(Classic100ClientPointer),
				"d2server.dll client-singleton descriptor bytes")) return FALSE;
		*callbackAbi = D2GS_CALLBACK_ABI_100;
		gClassic100Active = TRUE;
		D2GSEventLog("ClassicAdapter",
			"Applied the hash-gated classic 1.00 host and callback profile");
	} else {
		if (!PatchBytes(fog, CLASSIC_FOG_PATCH_RVA, Classic109FogReplacement,
				sizeof(Classic109FogReplacement),
				"Fog.dll critical-section validator")) return FALSE;
		*callbackAbi = D2GS_CALLBACK_ABI_109B;
		D2GSEventLog("ClassicAdapter",
			"Applied the hash-gated classic 1.09b host and callback profile");
	}
	return TRUE;
}

extern void ClassicAdapterTraceNetwork(void)
{
	HMODULE d2Game;
	HMODULE d2Net;
	FARPROC recvGame;
	FARPROC recvControl;

	if (!gClassic100Active) return;
	d2Game = GetModuleHandleA("D2Game.dll");
	d2Net = GetModuleHandleA("D2Net.dll");
	if (!d2Game || !d2Net) {
		D2GSEventLog("ClassicAdapterTrace",
			"D2Game.dll or D2Net.dll is not loaded after initialization");
		return;
	}
	if (!PatchClassic100DatabaseReturns(d2Game)) return;
	recvGame = GetProcAddress(d2Net, MAKEINTRESOURCEA(10010));
	recvControl = GetProcAddress(d2Net, MAKEINTRESOURCEA(10011));
	if (!recvGame || !recvControl) {
		D2GSEventLog("ClassicAdapterTrace",
			"D2Net.dll receive exports are unavailable");
		return;
	}
	gClassic100RecvGame = (D2NETRECVFUNC)recvGame;
	gClassic100RecvControl = (D2NETRECVFUNC)recvControl;
	gClassic100Accept = (D2NETACCEPTFUNC)((BYTE *)d2Net+
		CLASSIC_100_D2NET_ACCEPT_RVA);
	gClassic100Deliver = (D2NETDELIVERFUNC)((BYTE *)d2Net+
		CLASSIC_100_D2NET_DELIVER_RVA);
	if (!PatchImportPointer(d2Game, CLASSIC_100_D2GAME_RECV_GAME_IAT_RVA,
			recvGame, (FARPROC)Classic100RecvGame,
			"D2Game.dll game receive import") ||
			!PatchImportPointer(d2Game,
				CLASSIC_100_D2GAME_RECV_CONTROL_IAT_RVA, recvControl,
				(FARPROC)Classic100RecvControl,
				"D2Game.dll control receive import") ||
			!PatchClassic100Parser(d2Net) ||
			!PatchRelativeJump(d2Net, CLASSIC_100_D2NET_ACCEPT_THUNK_RVA,
				(FARPROC)gClassic100Accept, (FARPROC)Classic100Accept,
				"D2Net.dll accept callback thunk") ||
			!PatchRelativeJump(d2Net, CLASSIC_100_D2NET_DELIVER_THUNK_RVA,
				(FARPROC)gClassic100Deliver, (FARPROC)Classic100Deliver,
				"D2Net.dll delivery callback thunk")) return;
	D2GSEventLog("ClassicAdapterTrace",
		"Installed classic 1.00 D2Net receive tracing");
}
