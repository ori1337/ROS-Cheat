
#include <stdio.h>
#include <time.h>
#include "Mmsystem.h"
#include <windows.h>
#include <fstream>
#include <string>
#include <vector>
#include "CLogger.h"

#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx9.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3d9.h"
#pragma comment(lib, "C:\\Program Files (x86)\\Microsoft DirectX SDK (June 2010)\\Lib\\x86\\d3d9.lib")
#pragma comment(lib, "winmm.lib")
#include <iostream>      //For I/O Functions
#include<stdlib.h>  //For System Pause
//dx sdk 
#if defined _M_X64
#pragma comment(lib, "C:\\Program Files (x86)\\Microsoft DirectX SDK (June 2010)\\Lib\\x64\\d3dx9.lib") 
#pragma comment(lib, "libMinHook-x64-v90-mt.lib") 
#elif defined _M_IX86
#pragma comment(lib, "C:\\Program Files (x86)\\Microsoft DirectX SDK (June 2010)\\Lib\\x86\\d3dx9.lib")
#pragma comment(lib, "detours\\detours.lib") 
#endif
#include "detours\\detours.h"
using namespace std;

#pragma warning (disable: 4244) //
#pragma warning (disable: 4996)
#define _CRT_SECURE_NO_DEPRECATE

//==========================================================================================================================

HMODULE Hand;
LPDIRECT3DDEVICE9 pDevice;

UINT Stride;

D3DVIEWPORT9 Viewport; //use this Viewport
float ScreenCX;
float ScreenCY;

LPD3DXFONT Font; //font

IDirect3DVertexShader9* vShader;
UINT vSize;

D3DVERTEXBUFFER_DESC vdesc;

bool InitOnce = true;
LPDIRECT3DTEXTURE9 Red, Green, Blue, Yellow;

int countnum = 0;

static BOOL screenshot_taken = FALSE;
const int NUM_SECONDS = 5;

//==========================================================================================================================

//features
//Groups
//
int misc_group = 0;
int aim_group = 0;
int Wallhacks_group = 0;
//visuals
int wallhack = 3;				//wallhack
int esp = 10;					//esp
int nograss = 1;				//nograss
int nofog = 1;				//nofog
int fastrun = 1;
int nofalldmg = 1;
int walkthrough = 0;			//walkthrough
int underwater = 1;				//underwater
int gravemode = 0;				//gravemode
int autoclimb = 0;				//autoclimb
int jumphack = 1;               //JUMPHACK
int flyhack = 0;                //flyhack
int FASTPARACUTE = 0;           //FASTPARACUTE
int antispectate = 0;           //antispectate trick 
int crosshair = 1;				//crosshair

//new Glow and shit  
int GlowItems = 1;
int WhiteItems = 1;
//speeders
int superCarSuperSpeed = 0;

								//aimbot settings
int aimbot = 1;
int aimkey = 1;
DWORD Daimkey = VK_RBUTTON;		//aimkey
int aimsens = 1;				//aim sensitivity, makes aim smoother
int aimfov = 7;					//aim field of view in % 
int aimheight = 3;				//aim height value, high value aims higher


int zoomhack = 0;				//zoomhack
int wireframe = 0;              //Memory wireframe
int memorychams = 1;            //Memory Chams
int memorywh = 1;               //Memory Wallhack

								//autoshoot settings
int autoshoot = 0;
unsigned int asdelay = 49;		//use x-999 (shoot for xx millisecs, looks more legit)
bool IsPressed = false;			//
DWORD astime = timeGetTime();	//autoshoot timer
								//==========================================================================================================================

								// getdir & log
char dlldir[320];
char* GetDirFile(char *name)
{
	static char pldir[320];
	strcpy_s(pldir, dlldir);
	strcat_s(pldir, name);
	return pldir;
}

void Log(const char *fmt, ...)
{
	if (!fmt)	return;

	char		text[4096];
	va_list		ap;
	va_start(ap, fmt);
	vsprintf_s(text, fmt, ap);
	va_end(ap);

	ofstream logfile(GetDirFile((PCHAR)"log.txt"), ios::app);
	if (logfile.is_open() && text)	logfile << text << endl;
	logfile.close();
}



//==========================================================================================================================

//==========================================================================================================================

// Parameters:
//
//   float4 CameraPos;
//   float4 FogInfo;
//   float4 PointLightAttr[5];
//   float4 ShadowLightAttr[5];
//   row_major float4x4 texTrans0;
//   row_major float4x4 world;
//   row_major float4x4 wvp;
//
//
// Registers:
//
//   Name            Reg   Size
//   --------------- ----- ----
//   PointLightAttr  c0       5
//   world           c5       4
//   ShadowLightAttr c9       4
//   wvp             c13      4
//   texTrans0       c17      3
//   FogInfo         c20      1
//   CameraPos       c21      1

//calc distance
float GetDistance(float Xx, float Yy, float xX, float yY)
{
	return sqrt((yY - Yy) * (yY - Yy) + (xX - Xx) * (xX - Xx));
}
void DrawRect(LPDIRECT3DDEVICE9 pDevice, FLOAT x, FLOAT y, FLOAT w, FLOAT h, D3DCOLOR color)
{
	D3DRECT rec = { (LONG)x, (LONG)y, (LONG)x + (LONG)w, (LONG)y + (LONG)h };
	pDevice->Clear(1, &rec, D3DCLEAR_TARGET, color, 0, 0);
}
struct WeaponEspInfo_t
{
	float pOutX, pOutY, RealDistance, vSizeod;
	float CrosshairDistance;
};
std::vector<WeaponEspInfo_t>WeaponEspInfo;

//w2s for weapons
void AddWeapons(LPDIRECT3DDEVICE9 Device)
{
	D3DXMATRIX matrix;
	Device->GetVertexShaderConstantF(13, matrix, 4);

	D3DXVECTOR3 pOut, pIn(0, (float)aimheight, 0);//-3?
	float distance = pIn.x * matrix._14 + pIn.y * matrix._24 + pIn.z * matrix._34 + matrix._44;
	D3DXVec3TransformCoord(&pOut, &pIn, &matrix);

	pOut.x = Viewport.X + (1.0f + pOut.x) *Viewport.Width / 2.0f;
	pOut.y = Viewport.Y + (1.0f - pOut.y) *Viewport.Height / 2.0f;

	WeaponEspInfo_t pWeaponEspInfo = { static_cast<float>(pOut.x), static_cast<float>(pOut.y), static_cast<float>(distance*0.1f), static_cast<float>(vSize) };
	WeaponEspInfo.push_back(pWeaponEspInfo);
}

//==========================================================================================================================

IDirect3DPixelShader9* oldsShader;
void DrawBox(IDirect3DDevice9 *pDevice, float x, float y, float w, float h, D3DCOLOR Color)
{
	struct Vertex
	{
		float x, y, z, ht;
		DWORD Color;
	}
	V[4] = { { x, y + h, 0.0f, 0.0f, Color },{ x, y, 0.0f, 0.01f, Color },
	{ x + w, y + h, 0.0f, 0.0f, Color },{ x + w, y, 0.0f, 0.0f, Color } };
	pDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	pDevice->GetPixelShader(&oldsShader);

	pDevice->SetTexture(0, Blue);
	pDevice->SetPixelShader(0);

	// mix texture color
	pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	// mix texture alpha 
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	//pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	//pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	//pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);

	pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, V, sizeof(Vertex));

	pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE);

	pDevice->SetPixelShader(oldsShader);
}

void DrawP(LPDIRECT3DDEVICE9 Device, int baseX, int baseY, int baseW, int baseH, D3DCOLOR Cor)
{
	D3DRECT BarRect = { baseX, baseY, baseX + baseW, baseY + baseH };
	Device->Clear(1, &BarRect, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, Cor, 0, 0);
}

void DrawCornerBox(LPDIRECT3DDEVICE9 Device, int x, int y, int w, int h, int borderPx, DWORD borderColor)
{
	DrawP(Device, x - (w / 2), (y - h + borderPx), w / 3, borderPx, borderColor); //bottom 
	DrawP(Device, x - (w / 2) + w - w / 3, (y - h + borderPx), w / 3, borderPx, borderColor); //bottom 
	DrawP(Device, x - (w / 2), (y - h + borderPx), borderPx, w / 3, borderColor); //left 
	DrawP(Device, x - (w / 2), (y - h + borderPx) + h - w / 3, borderPx, w / 3, borderColor); //left 
	DrawP(Device, x - (w / 2), y, w / 3, borderPx, borderColor); //top 
	DrawP(Device, x - (w / 2) + w - w / 3, y, w / 3, borderPx, borderColor); //top 
	DrawP(Device, (x + w - borderPx) - (w / 2), (y - h + borderPx), borderPx, w / 3, borderColor);//right 
	DrawP(Device, (x + w - borderPx) - (w / 2), (y - h + borderPx) + h - w / 3, borderPx, w / 3, borderColor);//right 
}

HRESULT DrawString(LPD3DXFONT Font, INT X, INT Y, DWORD dColor, CONST PCHAR cString, ...)
{
	HRESULT hRet;

	CHAR buf[512] = { NULL };
	va_list ArgumentList;
	va_start(ArgumentList, cString);
	_vsnprintf_s(buf, sizeof(buf), sizeof(buf) - strlen(buf), cString, ArgumentList);
	va_end(ArgumentList);

	RECT rc[2];
	SetRect(&rc[0], X, Y, X, 0);
	SetRect(&rc[1], X, Y, X + 50, 50);

	hRet = D3D_OK;

	if (SUCCEEDED(hRet))
	{
		Font->DrawTextA(NULL, buf, -1, &rc[0], DT_NOCLIP, 0xFF000000);
		hRet = Font->DrawTextA(NULL, buf, -1, &rc[1], DT_NOCLIP, dColor);
	}

	return hRet;
}

HRESULT DrawCenteredString(LPD3DXFONT Font, INT X, INT Y, DWORD dColor, CONST PCHAR cString, ...)
{
	HRESULT hRet;

	CHAR buf[512] = { NULL };
	va_list ArgumentList;
	va_start(ArgumentList, cString);
	_vsnprintf_s(buf, sizeof(buf), sizeof(buf) - strlen(buf), cString, ArgumentList);
	va_end(ArgumentList);

	RECT rc[2];
	SetRect(&rc[0], X, Y, X, 0);
	SetRect(&rc[1], X, Y, X + 2, 2);

	hRet = D3D_OK;

	if (SUCCEEDED(hRet))
	{
		Font->DrawTextA(NULL, buf, -1, &rc[0], DT_NOCLIP | DT_CENTER, 0xFF000000);
		hRet = Font->DrawTextA(NULL, buf, -1, &rc[1], DT_NOCLIP | DT_CENTER, dColor);
	}

	return hRet;
}

HRESULT GenerateTexture(IDirect3DDevice9 *pDevice, IDirect3DTexture9 **ppD3Dtex, DWORD colour32)
{
	if (FAILED(pDevice->CreateTexture(8, 8, 1, 0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, ppD3Dtex, NULL)))
		return E_FAIL;

	WORD colour16 = ((WORD)((colour32 >> 28) & 0xF) << 12)
		| (WORD)(((colour32 >> 20) & 0xF) << 8)
		| (WORD)(((colour32 >> 12) & 0xF) << 4)
		| (WORD)(((colour32 >> 4) & 0xF) << 0);

	D3DLOCKED_RECT d3dlr;
	(*ppD3Dtex)->LockRect(0, &d3dlr, 0, 0);
	WORD *pDst16 = (WORD*)d3dlr.pBits;

	for (int xy = 0; xy < 8 * 8; xy++)
		*pDst16++ = colour16;

	(*ppD3Dtex)->UnlockRect(0);

	return S_OK;
}

class D3DTLVERTEX
{
public:
	FLOAT X, Y, X2, Y2;
	DWORD Color;
};

IDirect3DPixelShader9* oldlShader;
void DrawLine(IDirect3DDevice9* pDevice, float X, float Y, float X2, float Y2, float Width, D3DCOLOR Color, bool AntiAliased)
{
	D3DTLVERTEX qV[2] = {
		{ (float)X , (float)Y, 0.0f, 1.0f, Color },
	{ (float)X2 , (float)Y2 , 0.0f, 1.0f, Color },
	};
	const DWORD D3DFVF_TL = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;

	pDevice->SetFVF(D3DFVF_TL);

	pDevice->GetPixelShader(&oldlShader);

	//pDevice->SetTexture(0, Yellow);
	pDevice->SetTexture(0, NULL);
	pDevice->SetPixelShader(0);

	pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, (AntiAliased ? TRUE : FALSE));

	pDevice->DrawPrimitiveUP(D3DPT_LINELIST, 2, qV, sizeof(D3DTLVERTEX));

	pDevice->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE);

	pDevice->SetPixelShader(oldlShader);
}

//==========================================================================================================================

void SaveCfg()
{
	ofstream fout;
	fout.open(GetDirFile((PCHAR)"rosd3d.ini"), ios::trunc);
	fout << "wallhack " << wallhack << endl;
	fout << "esp " << esp << endl;
	fout << "aimbot " << aimbot << endl;
	fout << "aimkey " << aimkey << endl;
	fout << "aimsens " << aimsens << endl;
	fout << "aimfov " << aimfov << endl;
	fout << "aimheight " << aimheight << endl;
	fout << "autoshoot " << autoshoot << endl;
	fout << "nograss " << nograss << endl;
	fout << "nofog " << nofog << endl;
	fout << "walkthrough " << walkthrough << endl;
	fout << "underwater " << underwater << endl;
	fout << "gravemode " << gravemode << endl;
	fout << "autoclimb " << autoclimb << endl;
	fout << "jumphack " << jumphack << endl;
	fout << "FASTPARACUTE " << FASTPARACUTE << endl;
	fout << "crosshair " << crosshair << endl;
	fout << "flyhack " << flyhack << endl;
	fout.close();
}

void LoadCfg()
{
	ifstream fin;
	string Word = "";
	fin.open(GetDirFile((PCHAR)"rosd3d.ini"), ifstream::in);
	fin >> Word >> wallhack;
	fin >> Word >> esp;
	fin >> Word >> aimbot;
	fin >> Word >> aimkey;
	fin >> Word >> aimsens;
	fin >> Word >> aimfov;
	fin >> Word >> aimheight;
	fin >> Word >> autoshoot;
	fin >> Word >> nograss;
	fin >> Word >> nofog;
	fin >> Word >> walkthrough;
	fin >> Word >> underwater;
	fin >> Word >> gravemode;
	fin >> Word >> autoclimb;
	fin >> Word >> jumphack;
	fin >> Word >> FASTPARACUTE;
	fin >> Word >> crosshair;
	fin >> Word >> flyhack;
	fin.close();
}

//==========================================================================================================================

// menu stuff

int menuselect = 0;
int Current = true;

int PosX = 30;
int PosY = 27;

int ShowMenu = false; //off by default

POINT Pos;

//LPD3DXFONT Font; //font

int CheckTab(int x, int y, int w, int h)
{
	if (ShowMenu)
	{
		GetCursorPos(&Pos);
		ScreenToClient(GetForegroundWindow(), &Pos);
		if (Pos.x > x && Pos.x < x + w && Pos.y > y && Pos.y < y + h)
		{
			if (GetAsyncKeyState(VK_LBUTTON) & 1)
			{
				//return 1; //disabled mouse selection in menu
			}
			return 2;
		}
	}
	return 0;
}

void WriteText(int x, int y, DWORD color, char *text)
{
	RECT rect;
	SetRect(&rect, x, y, x, y);
	Font->DrawText(0, text, -1, &rect, DT_NOCLIP | DT_LEFT, color);
}

void lWriteText(int x, int y, DWORD color, char *text)
{
	RECT rect;
	SetRect(&rect, x, y, x, y);
	Font->DrawText(0, text, -1, &rect, DT_NOCLIP | DT_RIGHT, color);
}

void Category(LPDIRECT3DDEVICE9 pDevice, char *text)
{
	if (ShowMenu)
	{
		int Check = CheckTab(PosX + 44, (PosY + 51) + (Current * 15), 190, 10);
		DWORD ColorText;

		ColorText = D3DCOLOR_ARGB(255, 255, 0, 255);

		if (Check == 2)
			ColorText = D3DCOLOR_ARGB(255, 255, 255, 255);

		if (menuselect == Current)
			ColorText = D3DCOLOR_ARGB(255, 255, 255, 255);

		WriteText(PosX + 44, PosY + 50 + (Current * 15) - 1, ColorText, text);
		lWriteText(PosX + 236, PosY + 50 + (Current * 15) - 1, ColorText, (PCHAR)"[-]");
		Current++;
	}
}

void AddItem(LPDIRECT3DDEVICE9 pDevice, char *text, int &var, char **opt, int MaxValue)
{
	if (ShowMenu)
	{
		int Check = CheckTab(PosX + 44, (PosY + 51) + (Current * 15), 190, 10);
		DWORD ColorText;

		if (var)
		{
			DrawBox(pDevice, PosX+44, PosY+51 + (Current * 15), 190, 10, D3DCOLOR_ARGB(200, 11, 200, 20));
			ColorText = D3DCOLOR_ARGB(255, 255, 214, 51);
		}
		if (var == 0)
		{
			DrawBox(pDevice, PosX+44, PosY+51 + (Current * 15), 190, 10, D3DCOLOR_ARGB(45, 11, 45, 20));
			ColorText = D3DCOLOR_ARGB(255, 255, 71, 26);
		}

		if (Check == 1)
		{
			var++;
			if (var > MaxValue)
				var = 0;
		}

		if (Check == 2)
			ColorText = D3DCOLOR_ARGB(255, 255, 255, 255);

		if (menuselect == Current)
		{
			static int lasttick_right = GetTickCount64();
			static int lasttick_left = GetTickCount64();
			if (GetAsyncKeyState(VK_RIGHT) && GetTickCount64() - lasttick_right > 100)
			{
				lasttick_right = GetTickCount64();
				var++;
				if (var > MaxValue)
					var = 0;
			}
			else if (GetAsyncKeyState(VK_LEFT) && GetTickCount64() - lasttick_left > 100)
			{
				lasttick_left = GetTickCount64();
				var--;
				if (var < 0)
					var = MaxValue;
			}
		}

		if (menuselect == Current)
			ColorText = D3DCOLOR_ARGB(255, 255, 255, 255);


		WriteText(PosX + 44, PosY + 50 + (Current * 15) - 1, D3DCOLOR_ARGB(255, 50, 50, 50), text);
		WriteText(PosX + 45, PosY + 51 + (Current * 15) - 1, ColorText, text);

		lWriteText(PosX + 236, PosY + 50 + (Current * 15) - 1, D3DCOLOR_ARGB(255, 100, 100, 100), opt[var]);
		lWriteText(PosX + 237, PosY + 51 + (Current * 15) - 1, ColorText, opt[var]);
		Current++;
	}
}
void AddItemGroup(LPDIRECT3DDEVICE9 pDevice, char *text, int &var, char **opt, int MaxValue)
{
	if (ShowMenu)
	{
		int Check = CheckTab(PosX + 44, (PosY + 51) + (Current * 15), 190, 10);
		DWORD ColorText;

		if (var)
		{
			DrawBox(pDevice, PosX+44, PosY+51 + (Current * 15), 190, 10, D3DCOLOR_XRGB(255, 0, 0));
			ColorText = D3DCOLOR_ARGB(255, 255, 214, 51);
		}
		if (var == 0)
		{
			DrawBox(pDevice, PosX+44, PosY+51 + (Current * 15), 190, 10, D3DCOLOR_XRGB(51, 51, 255));
			ColorText = D3DCOLOR_ARGB(255, 255, 71, 26);
		}

		if (Check == 1)
		{
			var++;
			if (var > MaxValue)
				var = 0;
		}

		if (Check == 2)
			ColorText = D3DCOLOR_ARGB(255, 255, 255, 255);

		if (menuselect == Current)
		{
			static int lasttick_right = GetTickCount64();
			static int lasttick_left = GetTickCount64();
			if (GetAsyncKeyState(VK_RIGHT) && GetTickCount64() - lasttick_right > 100)
			{
				lasttick_right = GetTickCount64();
				var++;
				if (var > MaxValue)
					var = 0;
			}
			else if (GetAsyncKeyState(VK_LEFT) && GetTickCount64() - lasttick_left > 100)
			{
				lasttick_left = GetTickCount64();
				var--;
				if (var < 0)
					var = MaxValue;
			}
		}

		if (menuselect == Current)
			ColorText = D3DCOLOR_ARGB(255, 255, 255, 255);


		WriteText(PosX + 44, PosY + 50 + (Current * 15) - 1, D3DCOLOR_ARGB(255, 50, 50, 50), text);
		WriteText(PosX + 45, PosY + 51 + (Current * 15) - 1, ColorText, text);

		lWriteText(PosX + 236, PosY + 50 + (Current * 15) - 1, D3DCOLOR_ARGB(255, 100, 100, 100), opt[var]);
		lWriteText(PosX + 237, PosY + 51 + (Current * 15) - 1, ColorText, opt[var]);
		Current++;
	}
}
void AddGroup(LPDIRECT3DDEVICE9 pDevice, char *txt, int &var, char **opt, int maxval)
{
	AddItemGroup(pDevice, txt, var, opt, maxval);
}
void AddText(LPDIRECT3DDEVICE9 pDevice, char *text, int &var, int MaxValue)
{
	if (ShowMenu)
	{
		int Check = CheckTab(PosX + 44, (PosY + 51) + (Current * 15), 190, 10);
		DWORD ColorText;
		if (menuselect == Current)
			ColorText = D3DCOLOR_XRGB(51, 255, 255);

		WriteText(PosX + 44, PosY + 50 + (Current * 15) - 1, D3DCOLOR_XRGB(51, 255, 255), text);
		WriteText(PosX + 45, PosY + 51 + (Current * 15) - 1, ColorText, text);

		//lWriteText(PosX + 236, PosY + 50 + (Current * 15) - 1, D3DCOLOR_ARGB(255, 100, 100, 100), opt[var]);
		//lWriteText(PosX + 237, PosY + 51 + (Current * 15) - 1, ColorText, opt[var]);
		Current++;
	}
}
//==========================================================================================================================

// menu part
char *opt_OnOff[] = { (PCHAR)"[OFF]", (PCHAR)"[On]" };
char *opt_WhChams[] = { (PCHAR)"[OFF]", (PCHAR)"[On]",(PCHAR) "[Yellow]",(PCHAR) "[Red]", (PCHAR)"[Blue]",(PCHAR) "[Green]",(PCHAR) "[Purple]", (PCHAR)"[White]", (PCHAR)"[Yellow2]" };
char *opt_ZeroTen[] = { (PCHAR) "[0]",(PCHAR) "[1]",(PCHAR) "[2]",(PCHAR) "[3]",(PCHAR)(PCHAR)"[4]",(PCHAR) "[5]",(PCHAR) "[6]", (PCHAR)"[7]",(PCHAR) "[8]",(PCHAR) "[9]", (PCHAR)"[10]" };
char *opt_Keys[] = { (PCHAR)"[OFF]", (PCHAR)"[Shift]", (PCHAR)"[RMouse]", (PCHAR)"[LMouse]",(PCHAR) "[Ctrl]",(PCHAR) "[Alt]", (PCHAR)"[Space]",(PCHAR) "[X]",(PCHAR) "[C]" };
char *opt_aimfov[] = { (PCHAR)"[0]",(PCHAR) "[5%]", (PCHAR)"[10%]", (PCHAR)"[15%]",(PCHAR) "[20%]", (PCHAR)"[25%]",(PCHAR) "[30%]", (PCHAR)"[35%]",(PCHAR) "[40%]", (PCHAR)"[45%]" };
char *opt_autoshoot[] = { (PCHAR)(PCHAR) "[OFF]", (PCHAR)"[OnKeyDown]" };
char *opt_Crosshair[] = { (PCHAR)"[OFF]", (PCHAR)"[Yellow]"};
char *opt_ColorsGlow[] = { (PCHAR)"[Green]" , (PCHAR)"[Purple]" };

char *opt_MemoryWH[] = { (PCHAR)"[NUMPAD4/5]", (PCHAR)"[NUMPAD4/5]" };
char *opt_MemoryChams[] = { (PCHAR)"[NUMPAD+/-]", (PCHAR)"[NUMPAD+/-]" };
//char *opt_superCarSuperSpeed[] = { (PCHAR)"OFF", (PCHAR)"[On]" };

char *opt_AntiSpectate[] = { (PCHAR)"[NUMPAD1/2]", (PCHAR)"[NUMPAD1/2]" };

void DrawMenu(LPDIRECT3DDEVICE9 pDevice)
{
	static int lasttick_insert = GetTickCount64();
	if (GetAsyncKeyState(VK_INSERT) && GetTickCount64() - lasttick_insert > 100)
	{
		lasttick_insert = GetTickCount64();
		ShowMenu = !ShowMenu;
		//save settings
		//SaveCfg();
	}

	if (ShowMenu)
	{
		static int lasttick_up = GetTickCount64();
		if (GetAsyncKeyState(VK_UP) && GetTickCount64() - lasttick_up > 75)
		{
			lasttick_up = GetTickCount64();
			menuselect--;
		}

		static int lasttick_down = GetTickCount64();
		if (GetAsyncKeyState(VK_DOWN) && GetTickCount64() - lasttick_down > 75)
		{
			lasttick_down = GetTickCount64();
			menuselect++;
		}

		Current = 1;

		WriteText(74.f, 47.0f, D3DCOLOR_ARGB(255, 255, 255, 000), (PCHAR)"All credits to UNKNOWNCHEATS");
		WriteText(74.f, 50.0f, D3DCOLOR_ARGB(255, 255, 255, 000), (PCHAR)"Please visit http://UNKNOWNCHEATS.ME/");
		WriteText(74.f, 62.0f, D3DCOLOR_ARGB(255, 255, 255, 000), (PCHAR)" My YouTube CH is \"gay null\", the multi hack is by UNKNOWNCHEATS.ME community");
		AddText(pDevice, (PCHAR) "[-==========-]", WhiteItems, 1);
		AddGroup(pDevice, (PCHAR)"[-=AIM SETTINGS=-]", aim_group, opt_OnOff, 1);
		if (aim_group) {
			AddText(pDevice, (PCHAR) "[-==========-]", WhiteItems, 1);
			AddItem(pDevice, (PCHAR) " Aimbot", aimbot, opt_OnOff, 1);
			AddItem(pDevice, (PCHAR) " Aimkey", aimkey, opt_Keys, 8);
			AddItem(pDevice, (PCHAR) " Aimsens", aimsens, opt_ZeroTen, 10);
			AddItem(pDevice, (PCHAR) " Aimfov", aimfov, opt_aimfov, 9);
			AddItem(pDevice, (PCHAR) " Aimheight", aimheight, opt_ZeroTen, 5);
			AddItem(pDevice, (PCHAR) " Autoshoot", autoshoot, opt_autoshoot, 1);
			AddText(pDevice, (PCHAR) "[-==========-]", WhiteItems, 1);
		}
		AddGroup(pDevice, (PCHAR)"[-=MISC SETTINGS=-]", misc_group, opt_OnOff, 1);
		if (misc_group) {
			AddText(pDevice, (PCHAR) "[-==========-]", WhiteItems, 1);
			AddItem(pDevice, (PCHAR) " Speed", fastrun, opt_OnOff, 1);
			AddItem(pDevice, (PCHAR) " NoFallDamage", nofalldmg, opt_OnOff, 1);
			AddItem(pDevice, (PCHAR) " Underwater", underwater, opt_OnOff, 1);
			AddItem(pDevice, (PCHAR) " Gravemode", gravemode, opt_OnOff, 1);
			AddItem(pDevice, (PCHAR) " Autoclimb (dead, also it shit)", autoclimb, opt_OnOff, 1);
			AddItem(pDevice, (PCHAR) " Walkthrough", walkthrough, opt_OnOff, 1);
			AddItem(pDevice, (PCHAR) " JUMPHACK", jumphack, opt_OnOff, 1);
			AddItem(pDevice, (PCHAR) " Antispectate NUMPAD1/2", antispectate, opt_OnOff, 1); //this is actually usless in menu i just put it to let you know it exist in hack
			AddItem(pDevice, (PCHAR) " FASTPARACUTE F2 on/off", FASTPARACUTE, opt_OnOff, 1); //same as antispectate usless in menu
			AddItem(pDevice, (PCHAR) " Crosshair", crosshair, opt_Crosshair, 5);
			AddItem(pDevice, (PCHAR) " super car + Super speed adder", superCarSuperSpeed, opt_OnOff, 1);
			AddText(pDevice, (PCHAR) "[-==========-]", WhiteItems, 1);
		}
		AddGroup(pDevice, (PCHAR)"[-=WALLHACKS SETTINGS=-]", misc_group, opt_OnOff, 1);
		if (Wallhacks_group) {
			AddText(pDevice, (PCHAR) "[-==========-]", WhiteItems, 1);
			AddItem(pDevice, (PCHAR) " Esp", esp, opt_ZeroTen, 10);
			AddItem(pDevice, (PCHAR) " Wallhack", wallhack, opt_WhChams, 2);
			AddItem(pDevice, (PCHAR) " No Fog", nofog, opt_OnOff, 1);
			AddItem(pDevice, (PCHAR) " No Grass", nograss, opt_OnOff, 1);
			AddItem(pDevice, (PCHAR) " ColorsGlow", GlowItems, opt_ColorsGlow, 1);
			AddItem(pDevice, (PCHAR) " White items", WhiteItems, opt_OnOff, 1);
			AddText(pDevice, (PCHAR) "[-==========-]", WhiteItems, 1);
		}
		if (menuselect >= Current)
			menuselect = 1;

		if (menuselect < 1)
			menuselect = 33;//Current;
	}
}

//=====================================================================================================================

//=====================================================================================================================

void __fastcall PatchHook(void *adress, void *bytes, int size)
{
	DWORD dwProtect;
	VirtualProtect((void*)adress, size, PAGE_EXECUTE_READWRITE, &dwProtect);
	memcpy((void *)adress, (PBYTE)bytes, size);
	VirtualProtect((void*)adress, size, dwProtect, &dwProtect);
}

DWORD MODULEROS = (DWORD)GetModuleHandleA("ros.exe");
DWORD InGameAdrress = (MODULEROS + 0x1E1FF28);//Like no ones cares 
//scan with cheat engine type array of byte
//09 6C 6F ? 64 5F 6C 61 79 ? 72 ? 63 6F ? 66 69 67 ? 63 6F 6E 66 69 67 ? 70 61 74 ? 29 0A - 1E
// or scan with cheat engine type string
//load_layer_config(config_path) - 1F
DWORD* Verify = (DWORD*)(InGameAdrress);

//=====================================================================================================================
//CROSSHAIR
void CrossHair(LPDIRECT3DDEVICE9 pDevice, D3DCOLOR Colore) {
	D3DVIEWPORT9 Viewport;
	pDevice->GetViewport(&Viewport);
	DWORD ScreenX = Viewport.Width / 2;
	DWORD ScreenY = Viewport.Height / 2;
	D3DRECT rec2 = { ScreenX - 9, ScreenY, ScreenX + 9, ScreenY + 1 };
	D3DRECT rec3 = { ScreenX, ScreenY - 9, ScreenX + 1, ScreenY + 9 };
	pDevice->Clear(1, &rec2, D3DCLEAR_TARGET, Colore, 0, 0);
	pDevice->Clear(1, &rec3, D3DCLEAR_TARGET, Colore, 0, 0);
}
DWORD PatchPTR(DWORD Address, DWORD Ptr_1, DWORD Ptr_2, DWORD Ptr_3, DWORD Ptr_4, DWORD Ptr_5, DWORD Ptr_6, int i)
{
	DWORD Temp = 0;
	if (IsBadReadPtr((PDWORD)Address, 4))return 0;
	Temp = *(PDWORD)((DWORD)(Address)) + Ptr_1;
	if (IsBadReadPtr((PDWORD)Temp, 4))return 0;
	Temp = *(PDWORD)((DWORD)(Temp)) + Ptr_2;
	if (IsBadReadPtr((PDWORD)Temp, 4))return 0;
	Temp = *(PDWORD)((DWORD)(Temp)) + Ptr_3;
	if (IsBadReadPtr((PDWORD)Temp, 4))return 0;
	Temp = *(PDWORD)((DWORD)(Temp)) + Ptr_4;
	if (IsBadReadPtr((PDWORD)Temp, 4))return 0;
	Temp = *(PDWORD)((DWORD)(Temp)) + Ptr_5;
	if (i) {
		if (IsBadReadPtr((PDWORD)Temp, 4))return 0;
		Temp = *(PDWORD)((DWORD)(Temp)) + Ptr_6;
	}
	if (IsBadReadPtr((PDWORD)Temp, 4))return 0;
	return Temp;
}

float DwordToFloat(DWORD const Bit)
{
	return *reinterpret_cast<float const*>(&Bit);
}

DWORD FloatToDword(float const Num)
{
	return *reinterpret_cast<DWORD const*>(&Num);

}
//=====================================================================================================================