#include <windows.h>
#include <fstream>
#include <stdio.h>
#include <winbase.h>
#include <winternl.h>
#include <time.h>

#include <iostream>
#include <tlhelp32.h>
#include <Psapi.h>


DWORD FindPattern(DWORD dwStart, DWORD dwLen, BYTE* pszPatt, char pszMask[])
{
	unsigned int i = NULL;
	int iLen = strlen(pszMask) - 1;
	for (DWORD dwRet = dwStart; dwRet < dwStart + dwLen; dwRet++)
	{
		if (*(BYTE*)dwRet == pszPatt[i] || pszMask[i] == '?')
		{
			if (pszMask[i + 1] == '\0') return(dwRet - iLen); i++;
		}
		else i = NULL;
	} return NULL;
}


DWORD DwStartAddress, DwSize, DwIsInGame, recoil;
void SearchPatterns()
{
	Sleep(50);
	DwStartAddress = (DWORD)GetModuleHandle("ros.exe");
	do {
		DwStartAddress = (DWORD)GetModuleHandle("ros.exe");
		Sleep(50);
	} while (!DwStartAddress);
	DwSize = 0xF0000000;
	recoil = FindPattern(DwStartAddress, DwSize, (PBYTE)"\xF2\x0F\x5E\x05\x00\x00\x00\x00\x83\xEC\x08\xF2\x0F\x11\x04\x24", (PCHAR)"xxxx????xxxxxxxx");
}


