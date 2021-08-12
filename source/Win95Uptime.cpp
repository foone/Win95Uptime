// Win95Uptime.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"

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

int idTimer = -1; 
HWND hwndPB;
HFONT hFont;
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
      CW_USEDEFAULT, 0, 480, 200, NULL, NULL, hInstance, NULL);

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
				SetTimer(hWnd, idTimer = 1, 1000, NULL);
				GetClientRect(hWnd, &rcClient); 

				cyVScroll = GetSystemMetrics(SM_CYVSCROLL); 
				hwndPB = CreateWindowEx(0, PROGRESS_CLASS, (LPTSTR) NULL, 
								WS_CHILD | WS_VISIBLE, rcClient.left, 
								rcClient.bottom - cyVScroll, 
								rcClient.right, cyVScroll, 
								hWnd, (HMENU) 0, hInst, NULL);
			    SendMessage(hwndPB, PBM_SETRANGE, 0, MAKELPARAM(0, 256));
    
				SendMessage(hwndPB, PBM_SETSTEP, (WPARAM) 1, 0); 
				hFont = CreateFont(24,0,0,0,0,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,"Comic Sans MS");
				if(hFont == NULL){
					hFont=(HFONT)GetStockObject(ANSI_VAR_FONT); 
				}

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
				SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
				// Set Burger King Color
				SetBkColor(hdc, GetSysColor(COLOR_3DFACE));


				// Select the variable stock font into the specified device context. 
				if (hOldFont = (HFONT)SelectObject(hdc, hFont)) 
				{
					DWORD ticks=GetTick();
					DWORD TTL=(~ticks)+1;

					char buffer[200],numberbuffer[100],crashbuffer[100];
					format_commas(ticks,numberbuffer);
					format_commas(TTL,crashbuffer); 
					int days = ticks/1000/60/60/24;
					int hours = (ticks/1000/60/60) % 24;
					int minutes = (ticks/1000/60) % 60;
					int crash_days = TTL/1000/60/60/24;
					int crash_hours = (TTL/1000/60/60) % 24;
					int crash_minutes = (TTL/1000/60) % 60;
					sprintf(buffer,"%s milliseconds since boot\n%d days %02dh:%02dm\n%s ms until CRASH TIME\nTTL: %d days %02dh:%02dm\n%s",
						numberbuffer,days,hours,minutes,crashbuffer,crash_days,crash_hours,crash_minutes, patch_status);
					DrawText(hdc, buffer, strlen(buffer), &rt, DT_CENTER);
					
					// Restore the original font.        
					SelectObject(hdc, hOldFont); 
				}
			}
			EndPaint(hWnd, &ps);
			break;
		case WM_TIMER:
			InvalidateRect(hWnd,NULL,TRUE);
			SendMessage(hwndPB, PBM_SETPOS, GetTick()>>24, 0);
			return 0;
		case WM_DESTROY:
			KillTimer(hWnd, 1);
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
