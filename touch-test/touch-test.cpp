// touch-test.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "touch-test.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
	RedirectIOToConsole();

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TOUCHTEST, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TOUCHTEST));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TOUCHTEST));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TOUCHTEST);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

char *GetErrorMessage()
{
	static char msgbuf[1024];
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), msgbuf, sizeof(msgbuf), NULL);
	return msgbuf;
}

bool touching = false;
POINTS touchpos;
HHOOK hMessageHook;

bool isTouchMouseEvent()
{
	const LONG_PTR cSignatureMask = 0xFFFFFF00;
	const LONG_PTR cFromTouch = 0xFF515700;
	return (GetMessageExtraInfo() & cSignatureMask) == cFromTouch;
}

LRESULT CALLBACK MessageHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	MSG &msg = *(MSG *)lParam;

	if (msg.message == WM_GESTURE) {
		HGESTUREINFO hgi = (HGESTUREINFO)lParam;
		GESTUREINFO gi;
		ZeroMemory(&gi, sizeof(gi));
		gi.cbSize = sizeof(gi);

		GetGestureInfo(hgi, &gi);

		if (gi.dwID == GID_PAN) {
			msg.message = WM_MOUSEWHEEL;
			MAKEPOINTS(msg.lParam) = touchpos;
			msg.wParam = 120;

			CloseGestureInfoHandle(hgi);
		}
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
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
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	//RegisterTouchWindow(hWnd, 0);
	//BOOL disable = false;
	//SetWindowFeedbackSetting(hWnd, FEEDBACK_TOUCH_PRESSANDHOLD, 0, sizeof(BOOL), &disable);
	//SetWindowFeedbackSetting(hWnd, FEEDBACK_TOUCH_RIGHTTAP, 0, sizeof(BOOL), &disable);

	{
		GESTURECONFIG gc = { 0, 0, GC_ALLGESTURES };
		if (!SetGestureConfig(hWnd, 0, 1, &gc, sizeof(gc)))
			printf("dang %s", GetErrorMessage());
	}
	{
		GESTURECONFIG gc = { GID_PAN, GC_PAN, 0 };
		if (!SetGestureConfig(hWnd, 0, 1, &gc, sizeof(gc)))
			printf("dang %s", GetErrorMessage());
	}

	hMessageHook = SetWindowsHookEx(WH_GETMESSAGE, MessageHook, NULL, GetCurrentThreadId());
	if (!hMessageHook)
		printf("Failed to install hook: %s\n", GetErrorMessage());

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

#define MAXTOUCH 5
POINT points[MAXTOUCH];
int npoints = 0;

//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int blah = 0;
	switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
			RECT client;
			GetClientRect(hWnd, &client);

			FillRect(hdc, &client, CreateSolidBrush(RGB(255, 255, 255)));

			for (int i = 0; i < npoints; i++) {
				SelectObject(hdc, CreateSolidBrush(RGB(0, 0, 0)));
				int x = points[i].x;
				int y = points[i].y;
				Ellipse(hdc, x - 10, y - 10, x + 10, y + 10);
			}

            EndPaint(hWnd, &ps);
        }
        break;
	case WM_TOUCH:
		TOUCHINPUT inputs[MAXTOUCH];
		npoints = min(MAXTOUCH, LOWORD(wParam));
		//printf("TOUCH %i\n", npoints);
		if (GetTouchInputInfo((HTOUCHINPUT)lParam, npoints, inputs, sizeof(TOUCHINPUT))) {
			for (int i = 0; i < npoints; i++) {
				points[i].x = TOUCH_COORD_TO_PIXEL(inputs[i].x);
				points[i].y = TOUCH_COORD_TO_PIXEL(inputs[i].y);
				ScreenToClient(hWnd, points + i);
			}
			CloseTouchInputHandle((HTOUCHINPUT)lParam);
		}
		RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
		break;
	case WM_LBUTTONDOWN:
		printf("DOWN  %i %4i %4i\n", isTouchMouseEvent(), LOWORD(lParam), HIWORD(lParam));
	case WM_MOUSEMOVE:
		if ((wParam & MK_LBUTTON)) {// && !isTouchMouseEvent()) {
			printf("MOUSE %i %4i %4i\n", isTouchMouseEvent(), LOWORD(lParam), HIWORD(lParam));
			npoints = 1;
			points[0].x = LOWORD(lParam);
			points[0].y = HIWORD(lParam);
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
		}
		break;
	case WM_LBUTTONUP:
		printf("UP    %i %4i %4i\n", isTouchMouseEvent(), LOWORD(lParam), HIWORD(lParam));
		npoints = 0;
		RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
		break;
	case WM_GESTURE:
	{
		static int prevY = 0;
		int nowY, delta = 0;
		HGESTUREINFO hgi = (HGESTUREINFO)lParam;
		GESTUREINFO gi;
		ZeroMemory(&gi, sizeof(gi));
		gi.cbSize = sizeof(gi);

		GetGestureInfo(hgi, &gi);
		if (gi.dwID == GID_BEGIN || gi.dwID == GID_END)
			return DefWindowProc(hWnd, message, wParam, lParam);

		switch (gi.dwID) {
		case GID_PAN:
			nowY = gi.ptsLocation.y;
			if (!(gi.dwFlags & GF_BEGIN))
				delta = nowY - prevY;
			prevY = nowY;
			printf("PAN %i\n", delta);
			break;
		default:
			printf("GESTURE\n");
			break;
		}
		CloseGestureInfoHandle(hgi);
		break;
	}
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
	case WM_TABLET_QUERYSYSTEMGESTURESTATUS:
		printf("FRAGE\n");
		return TABLET_DISABLE_PRESSANDHOLD;

	case WM_MOUSEWHEEL:
		printf("wheeeee %x\n", HIWORD(wParam));
		break;

	case WM_VSCROLL:
		printf("scroll\n");
		break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
