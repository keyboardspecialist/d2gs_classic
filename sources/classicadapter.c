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
#define CLASSIC_101_FOG_PREINIT_DIAGNOSTIC_RVA 0xD2D2
#define SHA256_LENGTH 32

typedef struct {
	LPCSTR name;
	LPCSTR enableVariable;
	BYTE const *d2GameSha256;
	BYTE const *d2ClientSha256;
	BYTE const *d2CommonSha256;
	BYTE const *d2NetSha256;
	BYTE const *d2WinSha256;
	BYTE const *fogSha256;
	BYTE const *clientDescriptor;
	BYTE const *clientPointer;
	DWORD const *databaseReturnRvas;
	DWORD databaseReturnCount;
	D2GSCALLBACKABI callbackAbi;
} CLASSICEARLYPROFILE;

static CLASSICEARLYPROFILE const *gClassicEarlyProfile;

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

static BYTE const Classic100D2NetSha256[SHA256_LENGTH] = {
	0x78, 0x76, 0xCC, 0x8D, 0xFE, 0x77, 0x91, 0x02,
	0xF1, 0x21, 0x8A, 0x5C, 0x73, 0x52, 0xA6, 0x96,
	0x2C, 0x95, 0xE3, 0x66, 0xDD, 0xD5, 0x24, 0xE4,
	0x54, 0x33, 0x2F, 0xAF, 0x95, 0x54, 0xD0, 0xFC
};

static BYTE const Classic101FogSha256[SHA256_LENGTH] = {
	0xE3, 0xDE, 0x58, 0x3C, 0xDF, 0xE7, 0xA6, 0x29,
	0x83, 0xDF, 0x8B, 0x7D, 0xB1, 0x4F, 0xB9, 0x5B,
	0xE6, 0x8F, 0xE5, 0xB3, 0x6C, 0x7E, 0x99, 0x5B,
	0x48, 0x53, 0x68, 0xFE, 0xAB, 0x62, 0x65, 0x66
};

static BYTE const Classic101D2GameSha256[SHA256_LENGTH] = {
	0xBA, 0x2A, 0x57, 0x3C, 0x7E, 0x80, 0x2F, 0x1B,
	0x5B, 0xA8, 0x20, 0x9B, 0x70, 0x1F, 0x7D, 0xEE,
	0x0D, 0x95, 0x6A, 0x16, 0xFE, 0x0C, 0x9E, 0x6F,
	0x72, 0x82, 0xDF, 0x18, 0xBB, 0xAC, 0x87, 0x56
};

static BYTE const Classic101D2ClientSha256[SHA256_LENGTH] = {
	0x1F, 0x77, 0x99, 0xA0, 0x4E, 0x15, 0xC2, 0xC7,
	0xB2, 0x02, 0x8E, 0xF7, 0x9B, 0x0E, 0xCF, 0x81,
	0x64, 0x0D, 0x07, 0x5C, 0xB7, 0x45, 0xBB, 0x5A,
	0x03, 0xF9, 0x26, 0x1E, 0xBB, 0xF6, 0x7A, 0x68
};

static BYTE const Classic101D2CommonSha256[SHA256_LENGTH] = {
	0xEE, 0x2C, 0x05, 0xC8, 0xB0, 0x67, 0x18, 0x81,
	0xF3, 0x69, 0x03, 0xBE, 0x3D, 0x55, 0x3D, 0x94,
	0xE0, 0x79, 0x9B, 0x61, 0x32, 0x20, 0xAF, 0x2C,
	0x5F, 0x1B, 0x16, 0x1F, 0x82, 0x4C, 0x83, 0xFD
};

static BYTE const Classic101D2NetSha256[SHA256_LENGTH] = {
	0x33, 0xCD, 0x75, 0x70, 0x45, 0x0D, 0xD5, 0x82,
	0xEA, 0x5A, 0x70, 0x77, 0xB6, 0x35, 0x72, 0xD8,
	0x2D, 0xF4, 0x4F, 0x36, 0x88, 0x28, 0xA5, 0xE1,
	0x96, 0x50, 0xB9, 0xE6, 0x6D, 0x81, 0xB8, 0xC5
};

static BYTE const Classic101D2WinSha256[SHA256_LENGTH] = {
	0xB6, 0xA9, 0x28, 0xE4, 0xA5, 0x29, 0xD5, 0x5D,
	0x37, 0x78, 0xA0, 0xCD, 0xCD, 0x80, 0x96, 0xF1,
	0xAB, 0x24, 0xC1, 0xF3, 0xC6, 0x03, 0xF1, 0xCB,
	0xBF, 0x24, 0x04, 0x15, 0xA9, 0x97, 0x79, 0x0D
};

static BYTE const Classic101FogPreinitDiagnosticExpected[] = {
	0xE8, 0x63, 0x3F, 0xFF, 0xFF
};
static BYTE const Classic101FogPreinitDiagnosticReplacement[] = {
	0x90, 0x90, 0x90, 0x90, 0x90
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
static BYTE const Classic101ClientDescriptor[] = {0xDC, 0x5B, 0x01, 0x00};
static BYTE const ClientPointerExpected[] = {0x88, 0x0A, 0xBB, 0x6F};
static BYTE const Classic100ClientPointer[] = {0xA8, 0xEC, 0x12, 0x10};
static BYTE const Classic101ClientPointer[] = {0xD0, 0xEA, 0x12, 0x10};
static DWORD const Classic100DatabaseReturnRvas[] = {
	0x5710, 0x57A2, 0x57E2, 0x5819, 0x584F, 0x58A7
};
static DWORD const Classic101DatabaseReturnRvas[] = {
	0x5790, 0x5822, 0x5862, 0x5899, 0x5924
};
static BYTE const ClassicDatabaseReturnExpected[] = {0xC2, 0x14, 0x00};
static BYTE const ClassicDatabaseReturnReplacement[] = {0xC2, 0x1C, 0x00};
static CLASSICEARLYPROFILE const Classic100Profile = {
	"1.00", "D2GS_EXPERIMENTAL_CLASSIC_100",
	Classic100D2GameSha256, Classic100D2ClientSha256,
	Classic100D2CommonSha256, Classic100D2NetSha256,
	Classic100D2WinSha256, Classic100FogSha256,
	Classic100ClientDescriptor, Classic100ClientPointer,
	Classic100DatabaseReturnRvas, ARRAYSIZE(Classic100DatabaseReturnRvas),
	D2GS_CALLBACK_ABI_100
};

static CLASSICEARLYPROFILE const Classic101Profile = {
	"1.01", "D2GS_EXPERIMENTAL_CLASSIC_101",
	Classic101D2GameSha256, Classic101D2ClientSha256,
	Classic101D2CommonSha256, Classic101D2NetSha256,
	Classic101D2WinSha256, Classic101FogSha256,
	Classic101ClientDescriptor, Classic101ClientPointer,
	Classic101DatabaseReturnRvas, ARRAYSIZE(Classic101DatabaseReturnRvas),
	D2GS_CALLBACK_ABI_101
};

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

static BOOL PatchClassicDatabaseReturns(HMODULE d2Game,
		CLASSICEARLYPROFILE const *profile)
{
	DWORD i;

	for (i = 0; i < profile->databaseReturnCount; i++) {
		if (!VerifyBytes(d2Game, profile->databaseReturnRvas[i],
				ClassicDatabaseReturnExpected,
				sizeof(ClassicDatabaseReturnExpected),
				"D2Game.dll database-character return")) return FALSE;
	}
	for (i = 0; i < profile->databaseReturnCount; i++) {
		if (!PatchBytes(d2Game, profile->databaseReturnRvas[i],
				ClassicDatabaseReturnReplacement,
				sizeof(ClassicDatabaseReturnReplacement),
				"D2Game.dll database-character return")) return FALSE;
	}
	D2GSEventLog("ClassicAdapter",
		"Adapted classic %s database-character export cleanup",
		profile->name);
	return TRUE;
}

static void FormatHash(BYTE const hash[SHA256_LENGTH],
		CHAR hashText[(SHA256_LENGTH * 2) + 1])
{
	DWORD i;

	for (i = 0; i < SHA256_LENGTH; i++)
		sprintf(hashText+(i*2), "%02X", hash[i]);
	hashText[SHA256_LENGTH*2] = '\0';
}

static BOOL VerifyFileSha256(LPCSTR fileName,
		BYTE const expected[SHA256_LENGTH])
{
	BYTE actual[SHA256_LENGTH];
	CHAR hashText[(SHA256_LENGTH * 2) + 1];

	if (!HashFileSha256(fileName, actual)) {
		D2GSEventLog("ClassicAdapter", "Failed hashing %s", fileName);
		return FALSE;
	}
	if (!memcmp(actual, expected, sizeof(actual))) return TRUE;
	FormatHash(actual, hashText);
	D2GSEventLog("ClassicAdapter", "%s SHA-256 is unsupported: %s",
		fileName, hashText);
	return FALSE;
}

extern BOOL ClassicAdapterApply(D2GSCALLBACKABI *callbackAbi)
{
	BYTE d2GameHash[SHA256_LENGTH];
	CHAR hashText[(SHA256_LENGTH * 2) + 1];
	CLASSICEARLYPROFILE const *profile;
	HMODULE d2server;
	HMODULE fog;

	if (!callbackAbi) return FALSE;
	gClassicEarlyProfile = NULL;
	profile = NULL;
	fog = NULL;
	if (!VerifyFileSha256("d2server.dll", D2ServerSha256)) return FALSE;
	if (!HashFileSha256("D2Game.dll", d2GameHash)) {
		D2GSEventLog("ClassicAdapter", "Failed hashing D2Game.dll");
		return FALSE;
	}
	if (!memcmp(d2GameHash, Classic100Profile.d2GameSha256,
			sizeof(d2GameHash)))
		profile = &Classic100Profile;
	else if (!memcmp(d2GameHash, Classic101Profile.d2GameSha256,
			sizeof(d2GameHash)))
		profile = &Classic101Profile;
	else if (memcmp(d2GameHash, Classic109D2GameSha256,
			sizeof(d2GameHash))) {
		FormatHash(d2GameHash, hashText);
		D2GSEventLog("ClassicAdapter",
			"D2Game.dll SHA-256 is unsupported: %s", hashText);
		return FALSE;
	}
	if (profile) {
		if (!IsEnabled(profile->enableVariable)) {
			D2GSEventLog("ClassicAdapter",
				"Classic %s runtime detected; set %s=1 to enable its experimental profile",
				profile->name, profile->enableVariable);
			return FALSE;
		}
		if (!VerifyFileSha256("D2Client.dll", profile->d2ClientSha256) ||
				!VerifyFileSha256("D2Common.dll", profile->d2CommonSha256) ||
				!VerifyFileSha256("D2Net.dll", profile->d2NetSha256) ||
				!VerifyFileSha256("D2Win.dll", profile->d2WinSha256) ||
				!VerifyFileSha256("Fog.dll", profile->fogSha256)) return FALSE;
		if (profile == &Classic101Profile) {
			fog = LoadLibraryA("Fog.dll");
			if (!fog) {
				D2GSEventLog("ClassicAdapter",
					"Failed loading Fog.dll. Code: %lu", GetLastError());
				return FALSE;
			}
			if (!VerifyBytes(fog, CLASSIC_101_FOG_PREINIT_DIAGNOSTIC_RVA,
					Classic101FogPreinitDiagnosticExpected,
					sizeof(Classic101FogPreinitDiagnosticExpected),
					"Fog.dll pre-initialization diagnostic call")) return FALSE;
		}
	} else if (!IsEnabled("D2GS_EXPERIMENTAL_CLASSIC_109")) {
		D2GSEventLog("ClassicAdapter",
			"Classic 1.09 runtime detected; set D2GS_EXPERIMENTAL_CLASSIC_109=1 to enable its experimental profile");
		return FALSE;
	} else if (!VerifyFileSha256("Fog.dll", Classic109FogSha256)) return FALSE;

	d2server = GetModuleHandleA("d2server.dll");
	if (!d2server) {
		D2GSEventLog("ClassicAdapter", "d2server.dll is not loaded");
		return FALSE;
	}
	if (!VerifyBytes(d2server, CLASSIC_D2SERVER_LANGUAGE_PATCH_RVA,
			D2ServerLanguageExpected, sizeof(D2ServerLanguageExpected),
			"d2server.dll language-mode call")) return FALSE;
	if (profile) {
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
	if (profile) {
		if (profile == &Classic101Profile &&
				!PatchBytes(fog, CLASSIC_101_FOG_PREINIT_DIAGNOSTIC_RVA,
					Classic101FogPreinitDiagnosticReplacement,
					sizeof(Classic101FogPreinitDiagnosticReplacement),
					"Fog.dll pre-initialization diagnostic call")) return FALSE;
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
				profile->clientDescriptor, sizeof(ClientDescriptorExpected),
				"d2server.dll client-singleton descriptor RVA") ||
			!PatchBytes(d2server, CLASSIC_100_CLIENT_EXPECTED_RVA,
				profile->clientPointer, sizeof(ClientPointerExpected),
				"d2server.dll client-singleton descriptor bytes")) return FALSE;
		*callbackAbi = profile->callbackAbi;
		gClassicEarlyProfile = profile;
		D2GSEventLog("ClassicAdapter",
			"Applied the hash-gated classic %s host and callback profile",
			profile->name);
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

extern BOOL ClassicAdapterFinalize(void)
{
	HMODULE d2Game;

	if (!gClassicEarlyProfile) return TRUE;
	d2Game = GetModuleHandleA("D2Game.dll");
	if (!d2Game) {
		D2GSEventLog("ClassicAdapter",
			"D2Game.dll is not loaded after initialization");
		return FALSE;
	}
	return PatchClassicDatabaseReturns(d2Game, gClassicEarlyProfile);
}
