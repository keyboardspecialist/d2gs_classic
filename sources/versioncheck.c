#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "eventlog.h"

#define	DEFAULT_VERSIONCHECK_KEY	0x12345678
#define	DEFAULT_PIECE_NUMBER		100
#define	DEFAULT_CHECK_SIZE 			5000000

typedef struct {
	char const *name;
	BOOL required;
} CHECK_FILE;

static CHECK_FILE const CheckFileList[]={
	{"Patch_d2.mpq", FALSE}, {"D2Data.mpq", TRUE}, {"D2Sfx.mpq", TRUE},
	{"D2Speech.mpq", TRUE}, {"D2Exp.mpq", FALSE},
	{"d2server.dll", TRUE}, {"D2Win.dll", TRUE}, {"D2Game.dll", TRUE},
	{"D2Client.dll", TRUE}, {"D2Common.dll", TRUE}, {"D2Net.dll", TRUE},
	{"Fog.dll", TRUE}, {"Storm.dll", TRUE}, {"D2Lang.dll", TRUE},
	{"D2Cmp.dll", TRUE}, {NULL, FALSE}
};

static BOOL hasExpansionData;

static DWORD CheckFile(DWORD dwKey, LPDWORD pdwChecksum, LPCSTR lpFileName);

extern DWORD VersionCheck(void)
{
	DWORD	dwChecksum, dwKey;
	CHAR	temp[MAX_PATH];
	DWORD	i;

	dwChecksum=0;
	hasExpansionData=FALSE;
	if (!GetModuleFileName(NULL,temp,sizeof(temp))) {
		D2GSEventLog("VersionCheck", "Failed to locate the server executable");
		return FALSE;
	}
	dwKey=DEFAULT_VERSIONCHECK_KEY;
	if (!CheckFile(dwKey, &dwChecksum, temp)) {
		D2GSEventLog("VersionCheck", "Failed reading server executable '%s'", temp);
		return FALSE;
	}
	for (i=0; CheckFileList[i].name; i++) {
		if (!CheckFile(dwKey, &dwChecksum, CheckFileList[i].name)) {
			if (CheckFileList[i].required) {
				D2GSEventLog("VersionCheck", "Required runtime file is missing or unreadable: %s", CheckFileList[i].name);
				return FALSE;
			}
			if (!strcmp(CheckFileList[i].name, "D2Exp.mpq"))
				D2GSEventLog("VersionCheck", "Optional runtime file not found: %s; expansion games are disabled", CheckFileList[i].name);
			else
				D2GSEventLog("VersionCheck", "Optional runtime file not found: %s", CheckFileList[i].name);
			continue;
		}
		if (!strcmp(CheckFileList[i].name, "D2Exp.mpq")) hasExpansionData=TRUE;
	}
	if (!dwChecksum) dwChecksum--;
	return dwChecksum;
}

extern BOOL VersionCheckHasExpansionData(void)
{
	return hasExpansionData;
}

static DWORD CheckFile(DWORD dwKey, LPDWORD pdwChecksum, LPCSTR lpFileName)
{
	DWORD		dwSize, dwCount;
	DWORD		i,j,temp,data;
	FILE * 		fp;

	if (!pdwChecksum || !lpFileName) return FALSE;
	if (!(fp=fopen(lpFileName,"rb"))) {
		return FALSE;
	}
	fseek(fp,0,SEEK_END);
	dwSize=ftell(fp);
	if (dwSize>DEFAULT_CHECK_SIZE) temp=DEFAULT_CHECK_SIZE;
	else temp=dwSize;
	dwCount=(temp/DEFAULT_PIECE_NUMBER)/sizeof(data);
	for (j=0; j<DEFAULT_PIECE_NUMBER; j++) {
		fseek(fp,(dwSize/DEFAULT_PIECE_NUMBER)*j,SEEK_SET);
		for (i=0; i<dwCount; i++) {
			fread(&data,1,sizeof(data),fp);
			*pdwChecksum += data;
			*pdwChecksum ^= dwKey;
		}
	}
	fclose(fp);
	*pdwChecksum += dwSize;
	return TRUE;
}
