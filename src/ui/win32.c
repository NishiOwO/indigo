#include <indigo.h>

#include <windows.h>

static HINSTANCE hInst;

static void ShowBitmapSize(HDC hdc, const char* name, int x, int y, int w, int h) {
	HBITMAP hBitmap = LoadBitmap(hInst, name);
	BITMAP	bmp;
	HDC	hmdc;
	GetObject(hBitmap, sizeof(bmp), &bmp);
	hmdc = CreateCompatibleDC(hdc);
	SelectObject(hmdc, hBitmap);
	if(w == 0 && h == 0) {
		StretchBlt(hdc, x, y, bmp.bmWidth, bmp.bmHeight, hmdc, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
	} else {
		StretchBlt(hdc, x, y, w, h, hmdc, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
	}
	DeleteDC(hmdc);
	DeleteObject(hBitmap);
}

static DWORD WINAPI UIThread(LPVOID lp) {
	BOOL bret;
	MSG  msg;
	while((bret = GetMessage(&msg, NULL, 0, 0)) != 0) {
		if(bret == -1) break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

static LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	if(msg == WM_CLOSE) {
		DestroyWindow(hWnd);
	} else if(msg == WM_DESTROY) {
		PostQuitMessage(0);
	} else {
		return DefWindowProc(hWnd, msg, wp, lp);
	}
	return 0;
}

static LRESULT CALLBACK SplashWndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	if(msg == WM_CLOSE) {
		DestroyWindow(hWnd);
	} else if(msg == WM_DESTROY) {
		PostQuitMessage(0);
	} else if(msg == WM_ERASEBKGND) {
	} else if(msg == WM_PAINT) {
		PAINTSTRUCT ps;
		HDC	    hDC = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
	} else {
		return DefWindowProc(hWnd, msg, wp, lp);
	}
	return 0;
}

static BOOL InitClass(const char* name, const char* menu, WNDPROC proc) {
	WNDCLASSEX wc;
	wc.cbSize	 = sizeof(wc);
	wc.style	 = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc	 = proc;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = hInst;
	wc.hIcon	 = LoadIcon(hInst, "INDIGO");
	wc.hCursor	 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = GetSysColorBrush(COLOR_MENU);
	wc.lpszMenuName	 = menu;
	wc.lpszClassName = name;
	wc.hIconSm	 = LoadIcon(hInst, "INDIGO");

	return RegisterClassEx(&wc);
}

int WINAPI WinMain(HINSTANCE hCurInst, HINSTANCE hPrevInst, LPSTR lpsCmdLine, int nCmdShow) {
	DWORD id;
	hInst = hCurInst;

	if(!InitClass("IndigoMain", NULL, MainWndProc)) return 0;
	if(!InitClass("IndigoSplash", NULL, SplashWndProc)) return 0;

	IndigoMainRoutine();

	WaitForSingleObject(CreateThread(NULL, 0, UIThread, NULL, 0, &id), INFINITE);
}

void IndigoShowSplash(void) {
	HWND	  desk = GetDesktopWindow();
	RECT	  rc;
	HWND	  hWnd;
	const int ww = 640;
	const int wh = 480;
	int	  dw;
	int	  dh;

	GetClientRect(desk, &rc);
	dw = rc.right - rc.left;
	dh = rc.bottom - rc.top;

	hWnd = CreateWindowEx(WS_EX_TOPMOST, "IndigoSplash", "Indigo Splash", WS_POPUP, (dw - ww) / 2, (dh - wh) / 2, ww, wh, NULL, 0, hInst, NULL);
	if(hWnd == NULL) return;

	ShowWindow(hWnd, SW_NORMAL);
	UpdateWindow(hWnd);
}
