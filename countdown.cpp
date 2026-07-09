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
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include "setup_UTF-8.txt"

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "WINMM.LIB")


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

void WORDreplace(wchar_t* dst, DWORD maxlen, const wchar_t* format, ...) {
	va_list nums;
    va_start(nums, format);
    DWORD i = 0;
	for (DWORD j = 0; format[j] != L'\0' && i < maxlen; j++) {
		if (format[j] == L'%') {
			if (format[++j] == L'%') dst[i++] = L'%';
			else {
				WORD num = (WORD)va_arg(nums, int);
				if (num == 0) {
					if (format[j] == L'?') dst[i++] = L'0';
					else {
						DWORD end = format[j] - L'0' + i;
						if (end >= maxlen) return;
						while (i < end) dst[i++] = L'0';
					}
				} else {
					BYTE maxbuf = format[j] - L'0', k = 0;
					if (format[j] == L'?') maxbuf = 9;
					else if (i + maxbuf >= maxlen) return;
					wchar_t* reversed = (wchar_t*)__builtin_alloca(maxbuf * sizeof(wchar_t));
					while (num > 0) {
						reversed[k++] = num % 10 + L'0';
						num /= 10;
					}
					if (format[j] != L'?') while (k < maxbuf) reversed[k++] = L'0';
					else if (i + k >= maxlen) return;
					while (k > 0) dst[i++] = reversed[--k];
				}
			}
		} else dst[i++] = format[j];
	}
	va_end(nums);
	dst[i] = L'\0';
}

HDC hMemDC;
HBITMAP hBitmap;
DWORD startTick;

void updateCountdown(HWND hwnd) {
    SYSTEMTIME date;
    GetLocalTime(&date);
    wchar_t time_str[strBufferSize];
    if (GetTickCount() < startTick + startDuration) WORDreplace(
		time_str, strBufferSize, STR_ON_START, daysdiff(date, DAY_OF_JIHAD)
	);
	else {
		date = datediff(date, DAY_OF_JIHAD);
		WORDreplace(
			time_str, strBufferSize, STR_DISPLAY,
			date.wYear, date.wMonth, date.wDay, date.wHour, date.wMinute, date.wSecond, date.wMilliseconds
		);
	}
	
    RECT rect = { 0, 0, windowWidth, windowHeight };
    HBRUSH hBrush = CreateSolidBrush(backgroundColor);
    FillRect(hMemDC, &rect, hBrush);
    DeleteObject(hBrush);
    SetBkMode(hMemDC, TRANSPARENT);
    SetTextColor(hMemDC, fontColor);
    SelectObject(hMemDC, hFont);
    DrawTextW(hMemDC, time_str, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    DeleteObject(hFont);
    HDC hScreenDC = GetDC(NULL);
    BLENDFUNCTION blend = { AC_SRC_OVER, NULL, fontOpacity, 0 };
    POINT p00 = { 0, 0 };
    SIZE windowSize = { windowWidth, windowHeight };
    UpdateLayeredWindow(hwnd, hScreenDC, NULL, &windowSize, hMemDC, &p00, transparentColor, &blend, ULW_COLORKEY);
    ReleaseDC(NULL, hScreenDC);
}

LRESULT CALLBACK timerProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    	case WM_CREATE: {
            HDC hScreenDC = GetDC(NULL);
            hMemDC = CreateCompatibleDC(hScreenDC);
            hBitmap = CreateCompatibleBitmap(hScreenDC, windowWidth, windowHeight);
            SelectObject(hMemDC, hBitmap);
            ReleaseDC(NULL, hScreenDC);
            SetTimer(hwnd, 1, DWORD(1000.f / FPS), NULL);
        	updateCountdown(hwnd);
        	break;
        }
    	case WM_TIMER: updateCountdown(hwnd); break;
    	case WM_DESTROY: {
    		KillTimer(hwnd, 1);
        	if (hMemDC) DeleteDC(hMemDC);
        	if (hBitmap) DeleteObject(hBitmap);
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

void sayNum(DWORD num) {
	BYTE singleNums[10], i = 0, j = 0;
	if (num != 0) while (num > 0) {
		singleNums[i++] = num % 10;
		num /= 10;
	}
	wchar_t* nameBuffer;
	if (i == 0) {
		nameBuffer = (wchar_t*)__builtin_alloca(sizeof(wchar_t) << 1);
		nameBuffer[j++] = L'0';
	} else {
		nameBuffer = (wchar_t*)__builtin_alloca(i * sizeof(wchar_t) << 1);
		bool continue0 = 0;
		while (i > 0) {
			if (singleNums[--i] == 0) {
				continue0 = 1;
				continue;
			} else if (j != 0 && continue0) nameBuffer[j++] = L'0';
			nameBuffer[j] = L'0' + singleNums[i];
			switch (i) {
				case 3: case 4: case 7: case 8: {
					if (nameBuffer[j] == L'2' && ((j > 0 && nameBuffer[j - 1] != L's') || j == 0)) nameBuffer[j] = L'L';
					switch (i) {
						case 8: nameBuffer[++j] = L'y'; break;
						case 4: nameBuffer[++j] = L'w'; break;
						case 3: case 7: {
							nameBuffer[++j] = L'q';
							if (i == 7 && singleNums[4] | singleNums[5] | singleNums[6] == 0) nameBuffer[++j] = L'w';
							break;
						}
					}
					break;
				}
				case 2: case 6: {
					nameBuffer[++j] = L'b';
					if (i == 6 && singleNums[4] | singleNums[5] == 0) nameBuffer[++j] = L'w';
					break;
				}
				case 1: case 5: case 9: {
					if (nameBuffer[j] == L'1' && j == 0) nameBuffer[j] = L's';
					else nameBuffer[++j] = L's';
					if (singleNums[i - 1] == 0) {
						if (i == 5) nameBuffer[++j] = L'w';
						else if (i == 9) nameBuffer[++j] = L'y';
					}
					break;
				}
			}
			j++;
			continue0 = 0;
		}
	}
	wchar_t pathBuffer[] = L".\\audio\\~.wav";
	WORD replaceID = 0;
	while (pathBuffer[replaceID] != L'\0') {
		if (pathBuffer[replaceID] == L'~') {
			for (i = 0; i < j; i++) {
				pathBuffer[replaceID] = nameBuffer[i];
				PlaySoundW(pathBuffer, NULL, SND_FILENAME | SND_SYNC);
			}
			break;
		}
	}
}

void sayWarning() {
	SYSTEMTIME now;
    GetLocalTime(&now);
    PlaySoundW(L".\\audio\\head.wav", NULL, SND_FILENAME | SND_SYNC);
    sayNum(daysdiff(date, DAY_OF_JIHAD));
    PlaySoundW(L".\\audio\\tail.wav", NULL, SND_FILENAME | SND_SYNC);
}

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE, LPSTR, int ncmdshow) {
	ShowWindow(GetConsoleWindow(), SW_HIDE);
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
    sayWarning();
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ShowWindow(GetConsoleWindow(), SW_SHOW);
    Sleep(1000);
    return 0;
}
