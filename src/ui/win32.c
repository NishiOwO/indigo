#include <indigo.h>
#include <indigores.h>

#include <windows.h>
#include <commctrl.h>

static HINSTANCE hInst;
static HANDLE	 hUIThread;
static HANDLE	 hUIReady;

static void _IndigoShowSplash(BOOL splash);

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

	IndigoMainUIRoutine();

	SetEvent(hUIReady);

	while((bret = GetMessage(&msg, NULL, 0, 0)) != 0) {
		if(bret == -1) break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

typedef struct main {
	HWND tree;
} main_t;
static LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	if(msg == WM_CLOSE) {
		DestroyWindow(hWnd);
	} else if(msg == WM_COMMAND) {
		int m = LOWORD(wp);
		if(m == ID_MENU_FILE_QUIT) DestroyWindow(hWnd);
		if(m == ID_MENU_HELP_VERSION) _IndigoShowSplash(FALSE);
	} else if(msg == WM_DESTROY) {
		main_t* m = (main_t*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

		DestroyWindow(m->tree);
		free(m);

		PostQuitMessage(0);
	} else if(msg == WM_CREATE) {
		main_t* m = malloc(sizeof(*m));
		m->tree	  = CreateWindow(WC_TREEVIEW, "", WS_CHILD | WS_BORDER | WS_VISIBLE | TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT, 0, 0, 0, 0, hWnd, 0, hInst, NULL);

		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)m);
	} else if(msg == WM_SIZE) {
		main_t* m = (main_t*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

		MoveWindow(m->tree, 0, 0, 200, HIWORD(lp), TRUE);
	} else {
		return DefWindowProc(hWnd, msg, wp, lp);
	}
	return 0;
}

typedef struct splash {
	HDC	hDC;
	HBITMAP hBitmap;
} splash_t;
static LRESULT CALLBACK SplashWndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	if(msg == WM_DESTROY) {
		splash_t* s = (splash_t*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

		DeleteDC(s->hDC);
		DeleteObject(s->hBitmap);
		free(s);
	} else if(msg == WM_CREATE) {
		RECT	  rc;
		HDC	  dc;
		DWORD	  d = (DWORD)GetWindowLongPtr(hWnd, GWL_STYLE);
		splash_t* s = malloc(sizeof(*s));

		GetClientRect(hWnd, &rc);

		dc = GetDC(hWnd);

		s->hBitmap = CreateCompatibleBitmap(dc, rc.right - rc.left, rc.bottom - rc.top);
		s->hDC	   = CreateCompatibleDC(dc);
		SelectObject(s->hDC, s->hBitmap);

		ReleaseDC(hWnd, dc);

		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)s);

		if(!(d & WS_CAPTION)) {
			SetTimer(hWnd, 100, 3 * 1000, NULL);
		}
	} else if(msg == WM_TIMER || msg == WM_LBUTTONDOWN) {
		DestroyWindow(hWnd);
	} else if(msg == WM_ERASEBKGND) {
	} else if(msg == WM_PAINT) {
		PAINTSTRUCT ps;
		RECT	    rc;
		HDC	    hDC;
		char	    buf[128];
		splash_t*   s = (splash_t*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

		GetClientRect(hWnd, &rc);

		ShowBitmapSize(s->hDC, "SPLASH", 0, 0, rc.right - rc.left, rc.bottom - rc.top);

		sprintf(buf, "Version %s", VERSION);
		SetBkMode(s->hDC, TRANSPARENT);
		SetTextAlign(s->hDC, TA_BOTTOM | TA_RIGHT);
		SetTextColor(s->hDC, RGB(0xff, 0xff, 0xff));
		TextOut(s->hDC, rc.right - rc.left - 8, rc.bottom - rc.top - 8, buf, strlen(buf));

		hDC = BeginPaint(hWnd, &ps);
		StretchBlt(hDC, 0, 0, rc.right - rc.left, rc.bottom - rc.top, s->hDC, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SRCCOPY);
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

	if(!InitClass("IndigoMain", "INDIGOMENU", MainWndProc)) return 0;
	if(!InitClass("IndigoSplash", NULL, SplashWndProc)) return 0;

	InitCommonControls();

	IndigoMainRoutine();

	hUIReady = CreateEvent(NULL, FALSE, FALSE, NULL);

	hUIThread = CreateThread(NULL, 0, UIThread, NULL, 0, &id);

	WaitForSingleObject(hUIReady, INFINITE);
	CloseHandle(hUIReady);

	WaitForSingleObject(hUIThread, INFINITE);
}

static void _IndigoShowSplash(BOOL splash) {
	HWND	  desk = GetDesktopWindow();
	RECT	  rc;
	HWND	  hWnd;
	const int ww = 640;
	const int wh = 400;
	int	  dw;
	int	  dh;

	GetClientRect(desk, &rc);
	dw = rc.right - rc.left;
	dh = rc.bottom - rc.top;

	if(splash) {
		hWnd = CreateWindowEx(WS_EX_TOPMOST, "IndigoSplash", "Indigo Splash", WS_POPUP, (dw - ww) / 2, (dh - wh) / 2, ww, wh, NULL, 0, hInst, NULL);
	} else {
		SetRect(&rc, 0, 0, ww, wh);
		AdjustWindowRect(&rc, WS_CAPTION, FALSE);

		hWnd = CreateWindowEx(WS_EX_TOPMOST, "IndigoSplash", "Indigo Version", WS_CAPTION, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, 0, hInst, NULL);
	}
	if(hWnd == NULL) return;

	ShowWindow(hWnd, SW_NORMAL);
	UpdateWindow(hWnd);
}

void IndigoShowSplash(void) { _IndigoShowSplash(TRUE); }

void IndigoShowMain(void) {
	HWND	  hWnd;
	const int ww = 800;
	const int wh = 600;
	int	  dw;
	int	  dh;

	hWnd = CreateWindow("IndigoMain", "Indigo", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, ww, wh, NULL, 0, hInst, NULL);
	if(hWnd == NULL) return;

	ShowWindow(hWnd, SW_NORMAL);
	UpdateWindow(hWnd);
}
