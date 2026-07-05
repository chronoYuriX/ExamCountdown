/******************************************************************************************************
 * DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE                                                        *
 * Version 2, December 2004                                                                           *
 *                                                                                                    *
 * Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>                                                   *
 *                                                                                                    *
 * Everyone is permitted to copy and distribute verbatim or modified copies of this license document, *
 * and changing it is allowed as long as the name is changed.                                         *
 *                                                                                                    *
 * DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE                                                        *
 * TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION                                    *
 *                                                                                                    *
 * 0. You just DO WHAT THE FUCK YOU WANT TO.                                                          *
 ******************************************************************************************************
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include "setup_UTF-8.txt"


const WORD ABSYEAR = 1601;
const SYSTEMTIME DAY_OF_JIHAD = {
// [year] [month] [day of week] [day] [hour] [minute] [second] [milliseconds]
    2028,  6,      NULL,         7,    9,     0,       0,       0
};
inline bool is366days(WORD year) {
	return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}
WORD getdays(WORD year, WORD month) {
	const WORD days[12] = {31, 28, 31, 30, 31 ,30, 31, 31, 30, 31, 30, 31};
	if (month == 2 && is366days(year)) return 29;
	return days[month - 1];
}
DWORD getABSdays(SYSTEMTIME date) {
    DWORD days = 0;
    for (WORD year = ABSYEAR; year < date.wYear; year++)  days += is366days(year) ? 366 : 365;
    for (WORD month = 1; month < date.wMonth; month++) days += getdays(date.wYear, month);
    days += date.wDay - 1;
    return days;
}
DWORD daysdiff(SYSTEMTIME from, SYSTEMTIME to) {
	return getABSdays(to) - getABSdays(from);
}
SYSTEMTIME datediff(SYSTEMTIME now, SYSTEMTIME until) {
    SYSTEMTIME diff;
	if (until.wMilliseconds < now.wMilliseconds) {
		now.wSecond++; diff.wMilliseconds = 1000 - now.wMilliseconds + until.wMilliseconds;
	} else diff.wMilliseconds = until.wMilliseconds - now.wMilliseconds;
	if (until.wSecond < now.wSecond) {
		now.wMinute++; diff.wSecond       = 60   - now.wSecond       + until.wSecond;
	} else diff.wSecond       = until.wSecond       - now.wSecond;
	if (until.wMinute < now.wMinute) {
		now.wHour++;   diff.wMinute       = 60   - now.wMinute       + until.wMinute;
	} else diff.wMinute       = until.wMinute       - now.wMinute;
	if (until.wHour < now.wHour) {
		now.wDay++;    diff.wHour         = 24   - now.wHour         + until.wHour;
	} else diff.wHour         = until.wHour         - now.wHour;
	if (until.wDay < now.wDay) {
		now.wMonth++;
		diff.wDay = getdays(now.wYear, now.wMonth) - now.wDay + until.wDay;
	} else diff.wDay          = until.wDay          - now.wDay;
	if (until.wMonth < now.wMonth) {
		now.wYear++;   diff.wMonth        = 12   - now.wMonth        + until.wMonth;
	} else diff.wSecond       = until.wSecond       - now.wSecond;
	if (until.wYear < now.wYear) { SYSTEMTIME zero = { 0 }; return zero; }
	diff.wYear = until.wYear - now.wYear;
	return diff;
}



HDC g_hdcMem;
HBITMAP g_hbm;
DWORD startTick;

void UpdateWindowContent(HWND hwnd) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    wchar_t timeStr[strBufferSize];
    if (GetTickCount() < startTick + startDuration) swprintf(
		timeStr, strBufferSize, STR_ON_START, daysdiff(st, DAY_OF_JIHAD)
	);
	else {
		st = datediff(st, DAY_OF_JIHAD);
		swprintf(
			timeStr, strBufferSize, STR_DISPLAY,
			st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds
		);
	}
	
    RECT rect = {0, 0, windowWidth, windowHeight};
    HBRUSH hBrush = CreateSolidBrush(backgroundColor);
    FillRect(g_hdcMem, &rect, hBrush);
    DeleteObject(hBrush);
    SetBkMode(g_hdcMem, TRANSPARENT);
    SetTextColor(g_hdcMem, fontColor);
    SelectObject(g_hdcMem, hFont);
    DrawTextW(g_hdcMem, timeStr, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    DeleteObject(hFont);
    HDC hdcScreen = GetDC(NULL);
    BLENDFUNCTION blend = {};
    blend.BlendOp = AC_SRC_OVER;
    blend.SourceConstantAlpha = 220;
    blend.AlphaFormat = 0;
    POINT ptZero = {0, 0};
    SIZE sizeWnd = {windowWidth, windowHeight};
    UpdateLayeredWindow(hwnd, hdcScreen, NULL, &sizeWnd,
                        g_hdcMem, &ptZero, transparentColor, &blend, ULW_COLORKEY);
    ReleaseDC(NULL, hdcScreen);
}

LRESULT CALLBACK timerProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    	case WM_CREATE: {
            HDC hdcScreen = GetDC(NULL);
            g_hdcMem = CreateCompatibleDC(hdcScreen);
            g_hbm = CreateCompatibleBitmap(hdcScreen, windowWidth, windowHeight);
            SelectObject(g_hdcMem, g_hbm);
            ReleaseDC(NULL, hdcScreen);
            SetTimer(hwnd, 1, DWORD(1000.f / FPS), NULL);
        	UpdateWindowContent(hwnd);
        	break;
        }
    	case WM_TIMER: UpdateWindowContent(hwnd); break;
    	case WM_DESTROY: {
    		KillTimer(hwnd, 1);
        	if (g_hdcMem) DeleteDC(g_hdcMem);
        	if (g_hbm) DeleteObject(g_hbm);
        	PostQuitMessage(0);
			break;
		}
	}
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void randClassNameW(wchar_t* des, const wchar_t* header, WORD namelen) {
	wcscpy(des, header);
	DWORD pid = GetCurrentProcessId(), tick = GetTickCount();
    srand(pid ^ tick);
	wchar_t tmpchr;
	for (WORD i = wcslen(header); i < namelen; i++) {
	    tmpchr = rand() % 62;
        if (tmpchr < 26) tmpchr += L'A';
        else if (tmpchr < 52) tmpchr += L'a' - 26;
        else tmpchr += L'0' - 52;
        des[i] = tmpchr;
    }
    des[namelen] = L'\0';
}

inline int getWindowLocX() {
	switch (windowLocation) {
		case 1: case 4: case 7: return windowDistanceX;
		case 2: case 5: case 8: return (GetSystemMetrics(SM_CXSCREEN) >> 1) - (windowWidth >> 1) + windowDistanceX;
		case 3: case 6: case 9: return GetSystemMetrics(SM_CXSCREEN) - windowWidth - windowDistanceX;
	}
}
inline int getWindowLocY() {
	switch (windowLocation) {
		case 1: case 2: case 3: return windowDistanceY;
		case 4: case 5: case 6: return (GetSystemMetrics(SM_CYSCREEN) >> 1) - (windowHeight >> 1) + windowDistanceY;
		case 7: case 8: case 9: return GetSystemMetrics(SM_CYSCREEN) - windowHeight - windowDistanceY;
	}
}

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE, LPSTR, int ncmdshow) {
	startTick = GetTickCount();
    wchar_t randname[64];
    randClassNameW(randname, L"CYX_COUNTDOWN_", 63);
    WNDCLASSW wc = {};
    wc.lpfnWndProc   = timerProc;
    wc.hInstance     = hinstance;
    wc.lpszClassName = randname;
    RegisterClassW(&wc);
    HWND hwnd = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW, randname, L"Countdown", WS_POPUP,
        getWindowLocX(), getWindowLocY(), windowWidth, windowHeight, // Location Here!
		NULL, NULL, hinstance, NULL
    );
    ShowWindow(hwnd, ncmdshow);
    if (windowTopMost) SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
