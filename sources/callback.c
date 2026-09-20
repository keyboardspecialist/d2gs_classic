#include <windows.h>
#include <stdio.h>
#include "d2gelib/d2server.h"
#include "callback.h"
#include "debug.h"
#include "vars.h"
#include "eventlog.h"
#include "handle_s2s.h"


#define _D(v) #v,v


EVENTCALLBACKTABLE	gEventCallbackTable;


extern void __fastcall CloseGame(WORD wGameId)
{
	DebugEventCallback("CloseGame",1, _D(wGameId));
	D2GSCBCloseGame(wGameId);
	return;
}


extern void __fastcall LeaveGame(LPGAMEDATA lpGameData, WORD wGameId, WORD wCharClass, 
				DWORD dwCharLevel, DWORD dwExpLow, DWORD dwExpHigh,
				WORD wCharStatus, LPCSTR lpCharName, LPCSTR lpCharPortrait,
				BOOL bUnlock, DWORD dwZero1, DWORD dwZero2,
				LPCSTR lpAccountName, PLAYERDATA PlayerData,
				PLAYERMARK PlayerMark)
{
	DebugEventCallback("LeaveGame",15, _D(lpGameData), _D(wGameId), _D(wCharClass),
			_D(dwCharLevel), _D(dwExpLow), _D(dwExpHigh), _D(wCharStatus),
			_D(lpCharName), _D(lpCharPortrait), _D(bUnlock), _D(dwZero1),
			_D(dwZero2), _D(lpAccountName), _D(PlayerData),_D(PlayerMark));
	D2GSCBLeaveGame(lpGameData, wGameId, wCharClass, dwCharLevel, dwExpLow,
		dwExpHigh, wCharStatus, lpCharName, lpCharPortrait, bUnlock,
		dwZero1, dwZero2, lpAccountName, PlayerData, PlayerMark);
	return;
}


static void __fastcall LeaveGame109b(LPGAMEDATA lpGameData, WORD wGameId,
				WORD wCharClass, DWORD dwCharLevel, DWORD dwExpLow,
				DWORD dwExpHigh, WORD wCharStatus, LPCSTR lpCharName,
				LPCSTR lpCharPortrait, BOOL bUnlock, DWORD dwZero1,
				DWORD dwZero2, LPCSTR lpAccountName, PLAYERDATA PlayerData)
{
	DebugEventCallback("LeaveGame109b",14, _D(lpGameData), _D(wGameId),
			_D(wCharClass), _D(dwCharLevel), _D(dwExpLow), _D(dwExpHigh),
			_D(wCharStatus), _D(lpCharName), _D(lpCharPortrait), _D(bUnlock),
			_D(dwZero1), _D(dwZero2), _D(lpAccountName), _D(PlayerData));
	D2GSCBLeaveGame(lpGameData, wGameId, wCharClass, dwCharLevel, dwExpLow,
		dwExpHigh, wCharStatus, lpCharName, lpCharPortrait, bUnlock,
		dwZero1, dwZero2, lpAccountName, PlayerData, 0);
	return;
}


static void __fastcall LeaveGame100(WORD wGameId, WORD wCharClass,
				DWORD dwCharLevel, DWORD dwExpLow, DWORD dwExpHigh,
				WORD wCharStatus, LPCSTR lpCharName, LPCSTR lpCharPortrait)
{
	CHAR accountName[MAX_ACCTNAME_LEN];

	ZeroMemory(accountName, sizeof(accountName));
	D2GSGetAccountName(lpCharName, accountName, sizeof(accountName));
	DebugEventCallback("LeaveGame100", 8, _D(wGameId), _D(wCharClass),
			_D(dwCharLevel), _D(dwExpLow), _D(dwExpHigh), _D(wCharStatus),
			_D(lpCharName), _D(lpCharPortrait));
	D2GSCBLeaveGame(0, wGameId, wCharClass, dwCharLevel, dwExpLow,
		dwExpHigh, wCharStatus, lpCharName, lpCharPortrait, TRUE,
		0, 0, accountName, 0, 0);
	return;
}


static void __fastcall LeaveGame104(LPGAMEDATA lpGameData, WORD wGameId,
				WORD wCharClass, DWORD dwCharLevel, DWORD dwExpLow,
				DWORD dwExpHigh, WORD wCharStatus, LPCSTR lpCharName,
				LPCSTR lpCharPortrait, PLAYERDATA PlayerData)
{
	CHAR accountName[MAX_ACCTNAME_LEN];

	ZeroMemory(accountName, sizeof(accountName));
	D2GSGetAccountName(lpCharName, accountName, sizeof(accountName));
	DebugEventCallback("LeaveGame104", 10, _D(lpGameData), _D(wGameId),
			_D(wCharClass), _D(dwCharLevel), _D(dwExpLow), _D(dwExpHigh),
			_D(wCharStatus), _D(lpCharName), _D(lpCharPortrait), _D(PlayerData));
	D2GSCBLeaveGame(lpGameData, wGameId, wCharClass, dwCharLevel, dwExpLow,
		dwExpHigh, wCharStatus, lpCharName, lpCharPortrait, TRUE,
		0, 0, accountName, PlayerData, 0);
	return;
}


extern void __fastcall GetDatabaseCharacter(LPGAMEDATA lpGameData, LPCSTR lpCharName,
						DWORD dwClientId, LPCSTR lpAccountName)
{
	DebugEventCallback("GetDatabaseCharacter", 4, _D(lpGameData), _D(lpCharName),
				_D(dwClientId), _D(lpAccountName));

	D2GSCBGetDatabaseCharacter(lpGameData, lpCharName, dwClientId, lpAccountName);
	return;
}


static void __fastcall GetDatabaseCharacter100(LPCSTR lpCharName, DWORD dwClientId)
{
	CHAR accountName[MAX_ACCTNAME_LEN];

	ZeroMemory(accountName, sizeof(accountName));
	D2GSGetAccountName(lpCharName, accountName, sizeof(accountName));
	DebugEventCallback("GetDatabaseCharacter100", 2, _D(lpCharName),
		_D(dwClientId));
	D2GSCBGetDatabaseCharacter(0, lpCharName, dwClientId, accountName);
	return;
}


static void __fastcall GetDatabaseCharacter104(LPGAMEDATA lpGameData,
				LPCSTR lpCharName, DWORD dwClientId)
{
	CHAR accountName[MAX_ACCTNAME_LEN];

	ZeroMemory(accountName, sizeof(accountName));
	D2GSGetAccountName(lpCharName, accountName, sizeof(accountName));
	DebugEventCallback("GetDatabaseCharacter104", 3, _D(lpGameData),
		_D(lpCharName), _D(dwClientId));
	D2GSCBGetDatabaseCharacter(lpGameData, lpCharName, dwClientId, accountName);
	return;
}


extern void __fastcall SaveDatabaseCharacter(LPGAMEDATA lpGameData, LPCSTR lpCharName,
					LPCSTR lpAccountName, LPVOID lpSaveData,
					DWORD dwSize, PLAYERDATA PlayerData)
{
	DebugEventCallback("SaveDatabaseCharacter",6, _D(lpGameData), _D(lpCharName),
			_D(lpAccountName), _D(lpSaveData), _D(dwSize), _D(PlayerData));
	D2GSCBSaveDatabaseCharacter(lpGameData, lpCharName,
			lpAccountName, lpSaveData, dwSize, PlayerData);
	return;
}


static void __fastcall SaveDatabaseCharacter100(LPCSTR lpCharName,
				LPCSTR lpAccountName, LPVOID lpSaveData,
				DWORD dwSize, PLAYERDATA PlayerData)
{
	DebugEventCallback("SaveDatabaseCharacter100", 5, _D(lpCharName),
			_D(lpAccountName), _D(lpSaveData), _D(dwSize), _D(PlayerData));
	D2GSCBSaveDatabaseCharacter(0, lpCharName, lpAccountName, lpSaveData,
		dwSize, PlayerData);
	return;
}


extern void __cdecl	ServerLogMessage(DWORD dwCount, LPCSTR lpFormat, ...)
{
	va_list     ap;

	va_start(ap,lpFormat);
	LogAP("ServerLogMessage", lpFormat, ap);
	va_end(ap);
	return;
}


extern void __fastcall EnterGame(WORD wGameId, LPCSTR lpCharName, WORD wCharClass, 
				DWORD dwCharLevel, DWORD dwReserved)

{
	DebugEventCallback("EnterGame",5, _D(wGameId), _D(lpCharName), _D(wCharClass),
			_D(dwCharLevel) , _D(dwReserved));
	D2GSCBEnterGame(wGameId, lpCharName, wCharClass, dwCharLevel, dwReserved);
	return;
}


extern BOOL __fastcall FindPlayerToken(LPCSTR lpCharName, DWORD dwToken, WORD wGameId,
					LPSTR lpAccountName, LPPLAYERDATA lpPlayerData)
{
	DebugEventCallback("FindPlayerToken",5, _D(lpCharName), _D(dwToken), 
			_D(wGameId), _D(lpAccountName), _D(lpPlayerData));
	return D2GSCBFindPlayerToken(lpCharName, dwToken, wGameId, lpAccountName, lpPlayerData);
}


extern void __fastcall UnlockDatabaseCharacter(LPGAMEDATA lpGameData, LPCSTR lpCharName,
						LPCSTR lpAccountName)
{
	DebugEventCallback("UnlockDatabaseCharacter", 3,  _D(lpGameData), _D(lpCharName),
							_D(lpAccountName));
	return;
}


static void __fastcall UnlockDatabaseCharacter100(LPCSTR lpCharName,
				LPCSTR lpAccountName)
{
	DebugEventCallback("UnlockDatabaseCharacter100", 2, _D(lpCharName),
		_D(lpAccountName));
	D2GSUnlockChar(lpAccountName, lpCharName);
	return;
}


static void __fastcall UnlockDatabaseCharacter104(LPGAMEDATA lpGameData,
				LPCSTR lpCharName)
{
	CHAR accountName[MAX_ACCTNAME_LEN];

	ZeroMemory(accountName, sizeof(accountName));
	D2GSGetAccountName(lpCharName, accountName, sizeof(accountName));
	DebugEventCallback("UnlockDatabaseCharacter104", 2, _D(lpGameData),
		_D(lpCharName));
	D2GSUnlockChar(accountName, lpCharName);
	return;
}


extern void __fastcall RelockDatabaseCharacter(LPGAMEDATA lpGameData, LPCSTR lpCharName,
						LPCSTR lpAccountName)
{
	DebugEventCallback("RelockDatabaseCharacter", 3,  _D(lpGameData), _D(lpCharName),
							_D(lpAccountName));
	return;
}


extern void __fastcall UpdateCharacterLadder(LPCSTR lpCharName, WORD wCharClass, 
					DWORD dwCharLevel, DWORD dwCharExpLow, 
					DWORD dwCharExpHigh,  WORD wCharStatus,
					PLAYERMARK PlayerMark)
{
	DebugEventCallback("UpdateCharacterLadder", 7, _D(lpCharName), _D(wCharClass),
			_D(dwCharLevel), _D(dwCharExpLow), _D(dwCharExpHigh),
			_D(wCharStatus), _D(PlayerMark));
	D2GSUpdateCharacterLadder(lpCharName, wCharClass, dwCharLevel, dwCharExpLow,
		dwCharExpHigh, wCharStatus);
	return;
}


static void __fastcall UpdateCharacterLadder100(LPCSTR lpCharName,
				WORD wCharClass, DWORD dwCharLevel, DWORD dwCharExpLow,
				DWORD dwCharExpHigh, WORD wCharStatus)
{
	DebugEventCallback("UpdateCharacterLadder100", 6, _D(lpCharName),
			_D(wCharClass), _D(dwCharLevel), _D(dwCharExpLow),
			_D(dwCharExpHigh), _D(wCharStatus));
	D2GSUpdateCharacterLadder(lpCharName, wCharClass, dwCharLevel,
		dwCharExpLow, dwCharExpHigh, wCharStatus);
	return;
}


extern void __fastcall UpdateGameInformation(WORD wGameId, LPCSTR lpCharName, 
					WORD wCharClass, DWORD dwCharLevel)
{
	DebugEventCallback("UpdateGameInformation", 4, _D(lpCharName), 
			_D(wCharClass), _D(dwCharLevel));
	D2GSCBUpdateGameInformation(wGameId, lpCharName, wCharClass, dwCharLevel);
	return;
}


extern GAMEDATA __fastcall SetGameData(void)
{
	DebugEventCallback("SetGameData", 0);
	return (GAMEDATA) 0x87654321;
}


extern void __fastcall SaveDatabaseGuild(DWORD dwReserved1, DWORD dwReserved2,
					DWORD dwReserved3)

{
	DebugEventCallback("SaveDatabaseGuild",3, _D(dwReserved1), _D(dwReserved2),
				_D(dwReserved3));
	return;
}


extern void __fastcall ReservedCallback1(DWORD dwReserved1, DWORD dwReserved2)
{
	DebugEventCallback("ReservedCallback1",2, _D(dwReserved1), _D(dwReserved2));
	return;
}


extern void __fastcall ReservedCallback2(DWORD dwReserved1, DWORD dwReserved2, 
					DWORD dwReserved3)
{
	DebugEventCallback("ReservedCallback2",3, _D(dwReserved1), _D(dwReserved2),
			_D(dwReserved3));
	return;
}
	

static void __fastcall ReservedCallback2_109b(DWORD dwReserved1, DWORD dwReserved2)
{
	DebugEventCallback("ReservedCallback2_109b",2, _D(dwReserved1),
		_D(dwReserved2));
	return;
}


extern void __fastcall LoadComplete(WORD wGameId, LPCSTR lpCharName, BOOL bExpansion)
{
	DebugEventCallback("LoadComplete",3, _D(wGameId), _D(lpCharName),
			_D(bExpansion));
	//D2GSLoadComplete(wGameId, lpCharName, bExpansion);
	return;
}


extern PEVENTCALLBACKTABLE EventCallbackTableInit(D2GSCALLBACKABI callbackAbi)
{
	gEventCallbackTable.fpCloseGame=CloseGame;
	if (callbackAbi == D2GS_CALLBACK_ABI_100 ||
			callbackAbi == D2GS_CALLBACK_ABI_101)
		gEventCallbackTable.fpLeaveGame=LeaveGame100;
	else if (callbackAbi == D2GS_CALLBACK_ABI_104)
		gEventCallbackTable.fpLeaveGame=LeaveGame104;
	else if (callbackAbi == D2GS_CALLBACK_ABI_109B)
		gEventCallbackTable.fpLeaveGame=LeaveGame109b;
	else
		gEventCallbackTable.fpLeaveGame=LeaveGame;
	if (callbackAbi == D2GS_CALLBACK_ABI_100 ||
			callbackAbi == D2GS_CALLBACK_ABI_101) {
		gEventCallbackTable.fpGetDatabaseCharacter=GetDatabaseCharacter100;
		gEventCallbackTable.fpSaveDatabaseCharacter=SaveDatabaseCharacter100;
	} else {
		if (callbackAbi == D2GS_CALLBACK_ABI_104)
			gEventCallbackTable.fpGetDatabaseCharacter=GetDatabaseCharacter104;
		else
			gEventCallbackTable.fpGetDatabaseCharacter=GetDatabaseCharacter;
		gEventCallbackTable.fpSaveDatabaseCharacter=SaveDatabaseCharacter;
	}
	gEventCallbackTable.fpServerLogMessage=ServerLogMessage;
	gEventCallbackTable.fpEnterGame=EnterGame;
	gEventCallbackTable.fpFindPlayerToken=FindPlayerToken;
	if (callbackAbi == D2GS_CALLBACK_ABI_100 ||
			callbackAbi == D2GS_CALLBACK_ABI_101)
		gEventCallbackTable.fpUnlockDatabaseCharacter=UnlockDatabaseCharacter100;
	else if (callbackAbi == D2GS_CALLBACK_ABI_104)
		gEventCallbackTable.fpUnlockDatabaseCharacter=UnlockDatabaseCharacter104;
	else
		gEventCallbackTable.fpUnlockDatabaseCharacter=UnlockDatabaseCharacter;
	gEventCallbackTable.fpRelockDatabaseCharacter=RelockDatabaseCharacter;
	if (callbackAbi == D2GS_CALLBACK_ABI_100 ||
			callbackAbi == D2GS_CALLBACK_ABI_101 ||
			callbackAbi == D2GS_CALLBACK_ABI_104)
		gEventCallbackTable.fpUpdateCharacterLadder=UpdateCharacterLadder100;
	else
		gEventCallbackTable.fpUpdateCharacterLadder=UpdateCharacterLadder;
	gEventCallbackTable.fpUpdateGameInformation=UpdateGameInformation;
	gEventCallbackTable.fpSetGameData=SetGameData;
	gEventCallbackTable.fpReserved1=ReservedCallback1;
	if (callbackAbi == D2GS_CALLBACK_ABI_109B ||
			callbackAbi == D2GS_CALLBACK_ABI_101)
		gEventCallbackTable.fpReserved2=ReservedCallback2_109b;
	else
		gEventCallbackTable.fpReserved2=ReservedCallback2;
	gEventCallbackTable.fpSaveDatabaseGuild=SaveDatabaseGuild;
	gEventCallbackTable.fpLoadComplete=LoadComplete;
	return &gEventCallbackTable;
}
