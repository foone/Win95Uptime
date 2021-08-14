// Win95Uptime.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Win95Uptime.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];								// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];								// The title bar text

// Foward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int idTimer = 1; 
HWND hwndPB;
HFONT hFont;
HBITMAP hbmpStopwatch;
int scroll_start = 0;
DWORD last_text_update=0;
DWORD last_stopwatch_update=0;
const DWORD stopwatch_update_frequency=250;
const DWORD text_update_divisor = 4; 
int last_stopwatch_frame=0;

int sw_dx=-2, sw_dy=-2;
int sw_x=0,sw_y=0;

// TODO: Initialize this from the resource's default checked/unchecked setting?
bool screensaver_mode = false; 

const char * patch_status; 

#define REG_PATH "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\Updates\\UPD216641.95"


bool IsPatched(){
	HKEY temp_key;
	DWORD result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,REG_PATH,0,KEY_READ, &temp_key);
	if(result == ERROR_SUCCESS){
		RegCloseKey(temp_key);
		return true;
	}else{
		return false;
	}
}

const char *GetPatchStatus(){
	OSVERSIONINFOA version;
	version.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
	if(GetVersionEx(&version)!=0){
		if(version.dwPlatformId == VER_PLATFORM_WIN32_NT){
			return "This system doesn't need the clock patch.";
		}
	}

	if(IsPatched()){
		return "This system is patched.";
	}else{
		return "This system is NOT patched.";
	}
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	MSG msg;
	HACCEL hAccelTable;

	patch_status = GetPatchStatus();

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WIN95UPTIME, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_WIN95UPTIME);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex; 

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_WIN95UPTIME);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_3DFACE+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_WIN95UPTIME;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_CLOCK);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU,
      CW_USEDEFAULT, 0, 480, 220, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }
   InitCommonControls(); 
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

void format_commas(DWORD number, char *out){
	char buffer[100];
	sprintf(buffer,"%u",number);
	strrev(buffer);
	int slen=strlen(buffer);
	memset(out,0,slen+1);
	char *p=out;
	for(int i=0;i<slen;i++){
		*(p++)=buffer[i];
		if((i%3) == 2 && i+1!=slen){
			*(p++)=',';
		}
	}
	*(p++)='\0';
	strrev(out);
}

DWORD GetTick(){
	return GetTickCount()+0;
}

void GetCrashTime(DWORD ticks, char *buffer, int num){
	time_t current_time =0;
	time(&current_time);
	current_time-=(ticks/1000);
	current_time+=4294967; // 2**32 milliseconds from boot
	struct tm* now = localtime(&current_time);
	strftime(buffer, num,"%B %d at %I:%M %p", now);
}

void DrawStopwatch(HDC hdc, int x, int y, unsigned int frame){
	unsigned int subframe = frame % 8; 
	HDC hdcStopwatchDC = CreateCompatibleDC(hdc);
	HGDIOBJ prev_bitmap = SelectObject(hdcStopwatchDC, hbmpStopwatch);
	BitBlt(hdc,x,y,STOPWATCH_W,STOPWATCH_H,hdcStopwatchDC,subframe*STOPWATCH_W,STOPWATCH_H,SRCAND);
	BitBlt(hdc,x,y,STOPWATCH_W,STOPWATCH_H,hdcStopwatchDC,subframe*STOPWATCH_W,0,SRCPAINT);

	SelectObject(hdcStopwatchDC, prev_bitmap);
	DeleteDC(hdcStopwatchDC);
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	TCHAR szHello[MAX_LOADSTRING];
	LoadString(hInst, IDS_HELLO, szHello, MAX_LOADSTRING);

	switch (message) 
	{
		case WM_CREATE:
			{
				RECT rcClient;
				int cyVScroll;
				// NOTE: Change this if the default is changed from OFF to ON
				SetTimer(hWnd, idTimer, 250, NULL);
				GetClientRect(hWnd, &rcClient); 

				cyVScroll = GetSystemMetrics(SM_CYVSCROLL); 
				scroll_start = rcClient.bottom - cyVScroll; 
				hwndPB = CreateWindowEx(0, PROGRESS_CLASS, (LPTSTR) NULL, 
								WS_CHILD | WS_VISIBLE, rcClient.left, 
								scroll_start, 
								rcClient.right, cyVScroll, 
								hWnd, (HMENU) 0, hInst, NULL);
			    SendMessage(hwndPB, PBM_SETRANGE, 0, MAKELPARAM(0, 256));
    
				SendMessage(hwndPB, PBM_SETSTEP, (WPARAM) 1, 0); 
				hFont = CreateFont(24,0,0,0,0,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,"Comic Sans MS");
				if(hFont == NULL){
					hFont=(HFONT)GetStockObject(ANSI_VAR_FONT); 
				}
				hbmpStopwatch = LoadBitmap(hInst, MAKEINTRESOURCE(STOPWATCH));

			}
			return 0;
		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			// Parse the menu selections:
			switch (wmId)
			{
				case IDM_ABOUT:
					DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
					break;
				case IDM_DVDSCREENSAVER:
					{
						MENUITEMINFO itemInfo;
						HMENU menu = GetMenu(hWnd);
						itemInfo.cbSize=sizeof(MENUITEMINFO);
						itemInfo.fMask = MIIM_STATE;
						if(menu!=NULL && GetMenuItemInfo(menu,wmId,FALSE,&itemInfo)!=0){
							itemInfo.fState ^= MFS_CHECKED;
							screensaver_mode = (itemInfo.fState & MFS_CHECKED) == MFS_CHECKED;
							if(screensaver_mode){
								// Initialize a random start position
								RECT rt;
								GetClientRect(hWnd, &rt);

								// A better way to do this would be to divide by the whole range, using floating point, then multiple by the output range.
								// But we're running on win95, so we might be on a 486SX with no FPU, so let's avoid floating point for now. 
								sw_x = rand() % (rt.right - STOPWATCH_W-1);
								sw_y = rand() % (scroll_start-STOPWATCH_H-4);
								SetTimer(hWnd, idTimer, 25, NULL);
							}else{
								// We don't need to run the timer as often if we're not animating.
								SetTimer(hWnd, idTimer, 250, NULL);
							}
							SetMenuItemInfo(menu,wmId,FALSE,&itemInfo);
						}
						break;
					}
				case IDM_EXIT:
					DestroyWindow(hWnd);
					break;
				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		case WM_PAINT:
			{
				hdc = BeginPaint(hWnd, &ps);
				RECT rt;
				GetClientRect(hWnd, &rt);
				HFONT hOldFont; 
				
				// Set text color to the default foreground text color
				SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));
				// Set Burger King Color
				SetBkColor(hdc, GetSysColor(COLOR_3DFACE));

				DWORD ticks=GetTick();
				if(ticks-last_stopwatch_update>=stopwatch_update_frequency){
					last_stopwatch_update=ticks;
					last_stopwatch_frame++;
					if(last_text_update == 0 || last_stopwatch_frame%text_update_divisor == 0 ){
						last_text_update = ticks;
					}
				}

				DWORD TTL=(~last_text_update)+1;
				if (hOldFont = (HFONT)SelectObject(hdc, hFont)) 
				{

					char buffer[300],numberbuffer[100],crashbuffer[100],crash_date[100];
					format_commas(last_text_update,numberbuffer);
					format_commas(TTL,crashbuffer); 
					int days = last_text_update/1000/60/60/24;
					int hours = (last_text_update/1000/60/60) % 24;
					int minutes = (last_text_update/1000/60) % 60;
					int crash_days = TTL/1000/60/60/24;
					int crash_hours = (TTL/1000/60/60) % 24;
					int crash_minutes = (TTL/1000/60) % 60;
					GetCrashTime(last_text_update, crash_date, 100);
					
					sprintf(buffer,"%s milliseconds since boot\n%d days %02dh:%02dm\n%s ms until CRASH TIME\nTTL: %d days %02dh:%02dm\nEstimated crash time: %s\n%s",
						numberbuffer,days,hours,minutes,crashbuffer,crash_days,crash_hours,crash_minutes, crash_date, patch_status);
					DrawText(hdc, buffer, strlen(buffer), &rt, DT_CENTER);
					
					// Restore the original font.        
					SelectObject(hdc, hOldFont); 
				}
				if(screensaver_mode){
					sw_x+=sw_dx;
					sw_y+=sw_dy;
					if(sw_y+STOPWATCH_H+3>scroll_start || sw_y<0){
						sw_dy=-sw_dy;
					}
					if(sw_x+STOPWATCH_W>rt.right || sw_x<0){
						sw_dx=-sw_dx;
					}
				}else{
					sw_x = rt.right-STOPWATCH_W;
					sw_y = scroll_start-STOPWATCH_H-3;
				}
				DrawStopwatch(hdc,sw_x,sw_y,last_stopwatch_frame);
			}
			EndPaint(hWnd, &ps);
			break;
		case WM_TIMER:
			InvalidateRect(hWnd,NULL,TRUE);
			SendMessage(hwndPB, PBM_SETPOS, GetTick()>>24, 0);
			return 0;
		case WM_DESTROY:
			KillTimer(hWnd, 1);
			DeleteObject(hbmpStopwatch);
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}



// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
				return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;
}
