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

#define UNICODE
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#ifndef COUNTDOWN_NO_VISTA
	#define WINVER       0x0600 // Windows Vista
	#define _WIN32_WINNT 0x0600
#endif
#include <windows.h>
#include <mmsystem.h>
#include "setup_UTF-8.txt"

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "WINMM.LIB")


const DWORD displayCycle = displayCycle_A + displayCycle_B;
const WORD ABSYEAR = 1601;
const SYSTEMTIME DAY_OF_JIHAD = {
// [year]         [month] [day of week] [day] [hour] [minute] [second] [milliseconds]
    YEAR_OF_JIHAD, 6,      NULL,         7,    9,     0,       0,       0
};

inline bool is366days(WORD year) {
	return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}
WORD getdays(WORD year, WORD month) {
	const WORD days[12] = { 31, 28, 31, 30, 31 ,30, 31, 31, 30, 31, 30, 31 };
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
    SYSTEMTIME diff = { 0 };
    if (until.wMilliseconds < now.wMilliseconds) {
        now.wSecond++; diff.wMilliseconds = 1000 - now.wMilliseconds + until.wMilliseconds;
    } else diff.wMilliseconds = until.wMilliseconds - now.wMilliseconds;
    if (until.wSecond < now.wSecond) {
        now.wMinute++; diff.wSecond = 60 - now.wSecond + until.wSecond;
    } else diff.wSecond = until.wSecond - now.wSecond;
    if (until.wMinute < now.wMinute) {
        now.wHour++;   diff.wMinute = 60 - now.wMinute + until.wMinute;
    } else diff.wMinute = until.wMinute - now.wMinute;
    if (until.wHour < now.wHour) {
        now.wDay++;    diff.wHour = 24 - now.wHour + until.wHour;
    } else diff.wHour   = until.wHour - now.wHour;
    if (until.wDay < now.wDay) {
        if (now.wMonth == 1) { now.wMonth = 12; now.wYear--; } // Back to last year
		else now.wMonth--;
        diff.wDay = getdays(now.wYear, now.wMonth) - now.wDay + until.wDay;
    } else diff.wDay    = until.wDay - now.wDay;
    if (until.wMonth < now.wMonth) {
        now.wYear++;   diff.wMonth = 12 - now.wMonth + until.wMonth;
    } else diff.wMonth  = until.wMonth - now.wMonth;
    if (until.wYear < now.wYear) return { 0 };
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
void insertString(wchar_t* dst, DWORD maxlen, const wchar_t* origin, ...) {
	va_list strs;
    va_start(strs, origin);
    DWORD i = 0;
    for (DWORD j = 0; origin[j] != L'\0' && i < maxlen; j++) {
    	if (origin[j] == L'~') {
    		wchar_t* insert = va_arg(strs, wchar_t*);
			for (DWORD k = 0; insert[k] != L'\0'; k++) {
				if (i >= maxlen) break;
				dst[i++] = insert[k];
			}
		} else dst[i++] = origin[j];
	}
	va_end(strs);
	dst[i] = L'\0';
}
void connectString(wchar_t* origin, DWORD maxlen, const wchar_t* next, const wchar_t endl = L'\0') {
	DWORD i = 0, j = 0;
	bool startCopy = 0;
	for (; next[j] != L'\0' && i < maxlen; i++) {
		if (startCopy) origin[i] = next[j++];
		else if (origin[i] == L'\0') {
			startCopy = 1;
			i--;
		}
	}
	if (endl != L'\0' && i < maxlen) origin[i++] = endl;
	origin[i] = L'\0';
}

bool isCommand(const wchar_t* input, DWORD maxinput, BYTE section, const wchar_t* command) {
	bool continueSpace = 0;
	for (DWORD i = 0, j = 0, currentSection = 0; i < maxinput; i++) {
		switch (input[i]) {
			case L'\r': case L'\n': case L'\0': return (currentSection == section && command[j] == L'\0') ||
				(section > currentSection && command[0] == L'\0');
			case L' ': {
				if (currentSection == section && command[j] == L'\0') return 1;
				if (continueSpace || currentSection >= section) break;
				continueSpace = 1;
				break;
			} default: {
				if (continueSpace) {
					currentSection++;
					continueSpace = 0;
				} if (currentSection == section && input[i] != command[j++]) return 0;
			}
		}
	}
	return 0;
}

HDC hMemDC;
HBITMAP hBitmap;
DWORD startTick;
void updateCountdown(HWND hwnd) {
	static bool firstTickCycle = 1;
    SYSTEMTIME date; GetLocalTime(&date);
    DWORD tick = GetTickCount();
    wchar_t time_str[strBufferSize];
    if ((tick < startTick + startDuration) && firstTickCycle) WORDreplace(
		time_str, strBufferSize, STR_ON_START, daysdiff(date, DAY_OF_JIHAD));
	else {
		firstTickCycle = 0;
		SYSTEMTIME diff = datediff(date, DAY_OF_JIHAD);
		if (tick % displayCycle < displayCycle_A) WORDreplace(
			time_str, strBufferSize, STR_DISPLAY_A, YEAR_OF_JIHAD,
			diff.wYear, diff.wMonth, diff.wDay, diff.wHour, diff.wMinute, diff.wSecond, diff.wMilliseconds);
		else WORDreplace(
			time_str, strBufferSize, STR_DISPLAY_B, YEAR_OF_JIHAD,
			daysdiff(date, DAY_OF_JIHAD), diff.wHour, diff.wMinute, diff.wSecond, diff.wMilliseconds);
	}
    RECT rect = { 0, 0, windowWidth, windowHeight };
    HBRUSH hBrush = CreateSolidBrush(backgroundColor);
    FillRect(hMemDC, &rect, hBrush);
    DeleteObject(hBrush);
    SetBkMode(hMemDC, TRANSPARENT);
    SetTextColor(hMemDC, fontColor);
    SelectObject(hMemDC, hFont);
    DrawTextW(hMemDC, time_str, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
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
        } case WM_TIMER: updateCountdown(hwnd); break;
    	case WM_CLOSE: DestroyWindow(hwnd); break;
    	case WM_DESTROY: {
    		KillTimer(hwnd, 1);
        	if (hMemDC) DeleteDC(hMemDC);
        	if (hBitmap) DeleteObject(hBitmap);
        	if (hFont) DeleteObject(hFont);
        	PostQuitMessage(0);
        	break;
		} default: return DefWindowProcW(hwnd, msg, wParam, lParam);
	}
	return 0;
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

bool noSuchFile(const wchar_t* path) {
	DWORD fileAttributes = GetFileAttributesW(path);
	return fileAttributes == INVALID_FILE_ATTRIBUTES || (fileAttributes & FILE_ATTRIBUTE_DIRECTORY);
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
						}
					}
					break;
				} case 2: case 6: {
					nameBuffer[++j] = L'b';
					if (i == 6 && singleNums[4] | singleNums[5] == 0) nameBuffer[++j] = L'w';
					break;
				} case 1: case 5: case 9: {
					if (nameBuffer[j] == L'1' && j == 0) nameBuffer[j] = L's';
					else nameBuffer[++j] = L's';
					if (singleNums[i - 1] == 0) {
						if (i == 5) nameBuffer[++j] = L'w';
						else if (i == 9) nameBuffer[++j] = L'y';
					}
				}
			}
			j++;
			continue0 = 0;
		}
	}
	wchar_t pathBuffer[] = L".\\audio\\~.wav";
	WORD replaceID = 0;
	while (pathBuffer[replaceID] != L'\0') if (pathBuffer[++replaceID] == L'~') break;
	for (i = 0; i < j; i++) {
		pathBuffer[replaceID] = nameBuffer[i];
		if (noSuchFile(pathBuffer)) PlaySoundW(L".\\audio\\void.wav", NULL, SND_FILENAME | SND_SYNC);
		else PlaySoundW(pathBuffer, NULL, SND_FILENAME | SND_SYNC);
	}
}

void sayWarning() {
	SYSTEMTIME now;
    GetLocalTime(&now);
    if (noSuchFile(L".\\audio\\head.wav")) PlaySoundW(L".\\audio\\void.wav", NULL, SND_FILENAME | SND_SYNC);
    else PlaySoundW(L".\\audio\\head.wav", NULL, SND_FILENAME | SND_SYNC);
    sayNum(daysdiff(now, DAY_OF_JIHAD));
    if (noSuchFile(L".\\audio\\tail.wav")) PlaySoundW(L".\\audio\\void.wav", NULL, SND_FILENAME | SND_SYNC);
    PlaySoundW(L".\\audio\\tail.wav", NULL, SND_FILENAME | SND_SYNC);
}

bool checkResources() {
	if (noSuchFile(L".\\audio\\void.wav")) {
		MessageBoxW(NULL, L"File missing: .\\audio\\void.wav\nAudio modules will be disabled.", L"ERROR 1.1",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	const wchar_t numWavs[] = L"012L3456789gsbqwy";
	wchar_t pathBuffer[] = L".\\audio\\~.wav", errorBuffer[1024] = L"%? files missing:\n";
	WORD replaceID = 0, missingFiles = 0;
	while (pathBuffer[replaceID] != L'\0') if (pathBuffer[++replaceID] == L'~') break;
	if (noSuchFile(L".\\audio\\head.wav")) {
		connectString(errorBuffer, 1024, L".\\audio\\head.wav", L'\n');
		missingFiles++;
	} if (noSuchFile(L".\\audio\\tail.wav")) {
		connectString(errorBuffer, 1024, L".\\audio\\tail.wav", L'\n');
		missingFiles++;
	}
	for (BYTE i = 0; numWavs[i] != L'\0'; i++) {
		pathBuffer[replaceID] = numWavs[i];
		if (noSuchFile(pathBuffer)) {
			connectString(errorBuffer, 1024, pathBuffer, L'\n');
			missingFiles++;
		}
	}
	if (missingFiles == 0) return 1;
	WORDreplace(errorBuffer, 1024, errorBuffer, missingFiles);
	MessageBoxW(GetConsoleWindow(), errorBuffer, L"ERROR 1.2", MB_ICONEXCLAMATION | MB_OK);
	return 1;
}

void runCountdown(HINSTANCE hinstance, int ncmdshow) {
	HANDLE hmutex = CreateMutexW(NULL, TRUE, L"CYX_COUNTDOWN_ACTIVATED"), hMapHWND = CreateFileMappingW(
        INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(HWND), L"Local\\CYX_COUNTDOWN_HWND"
    );
    if (hideCMD) ShowWindow(GetConsoleWindow(), SW_HIDE);
	bool activateAudio = checkResources();
	startTick = GetTickCount();
    wchar_t randname[64];
    randClassNameW(randname, L"CYX_COUNTDOWN_", 63);
    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc   = timerProc;
    wc.hInstance     = hinstance;
    wc.lpszClassName = randname;
    RegisterClassW(&wc);
    HWND hwnd = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW, randname, countdownWindowName, WS_POPUP,
        getWindowLocX(), getWindowLocY(), windowWidth, windowHeight, // Location Here!
		NULL, NULL, hinstance, NULL);
    if (hMapHWND) {
        HWND* phwnd = (HWND*)MapViewOfFile(hMapHWND, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(HWND));
        if (phwnd) {
            *phwnd = hwnd;
            UnmapViewOfFile(phwnd);
        }
    }
    ShowWindow(hwnd, ncmdshow);
    if (windowTopMost) SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    if (activateAudio) sayWarning();
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    ShowWindow(GetConsoleWindow(), SW_SHOW);
    ReleaseMutex(hmutex);
    CloseHandle(hmutex);
    if (hMapHWND) CloseHandle(hMapHWND);
}

struct IOPORT {
	HANDLE hin, hout, herr;
	IOPORT(): hin(GetStdHandle(STD_INPUT_HANDLE)), hout(GetStdHandle(STD_OUTPUT_HANDLE)),
			herr(GetStdHandle(STD_ERROR_HANDLE)) {
		DWORD currentMode;
    	GetConsoleMode(hin, &currentMode);
		SetConsoleMode(hin, currentMode | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
	}
	DWORD print(const wchar_t* str, DWORD len = ~0) {
		if (len == ~0) len = wcslen(str);
		DWORD written;
		WriteConsoleW(hout, str, len, &written, NULL);
		return written;
	}
	DWORD input(wchar_t* buffer, DWORD bufferSize) {
		DWORD read;
		ReadConsoleW(hin, buffer, bufferSize, &read, NULL);
		return read;
	}
};

namespace AUTOSTART {
	bool isAutoStart() {
		HKEY hkey = NULL;
    	LONG result = RegOpenKeyExW(
			HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &hkey);
    	if (result != ERROR_SUCCESS) return 0;
    	DWORD cbData = 0;
    	result = RegQueryValueExW(hkey, L"CYX_COUNTDOWN", NULL, NULL, NULL, &cbData);
    	if (result != ERROR_SUCCESS) {
        	RegCloseKey(hkey);
        	return 0;
    	}
    	wchar_t* regPath = (wchar_t*)__builtin_alloca(cbData);
    	DWORD dwType = REG_SZ;
    	result = RegQueryValueExW(hkey, L"CYX_COUNTDOWN", NULL, &dwType, (BYTE*)regPath, &cbData);
    	RegCloseKey(hkey);
    	if (result != ERROR_SUCCESS || dwType != REG_SZ) return 0;
    	wchar_t curPath[MAX_PATH] = { 0 };
    	GetModuleFileNameW(NULL, curPath, MAX_PATH);
    	return CompareStringW(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, regPath, -1, curPath, -1) == CSTR_EQUAL;
	}
	const BYTE CHECK = 0, REG = 1, REMOVE = 2, FAILED = 100, SUCCEED = 101, NO = 200, YES = 201;
}
BYTE configAutoStart(BYTE mode) { // no OP required!
	using namespace AUTOSTART;
	HKEY hkey = NULL;
    LONG result = RegOpenKeyExW(
        HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_WRITE, &hkey);
    if (result != ERROR_SUCCESS) return 0;
    switch (mode) {
    	case CHECK: return isAutoStart() ? YES : NO;
    	case REG: {
    		if (isAutoStart()) break;
    		wchar_t path[MAX_PATH] = { 0 };
    		GetModuleFileNameW(NULL, path, MAX_PATH);
    		result = RegSetValueExW(
				hkey, L"CYX_COUNTDOWN_", 0, REG_SZ, (const BYTE*)path, (wcslen(path) + 1) * sizeof(wchar_t));
			break;
		} case REMOVE: {
			if (isAutoStart()) result = RegDeleteValueW(hkey, L"CYX_COUNTDOWN_");
			else result = __MSABI_LONG(15842);
		}
	}
    RegCloseKey(hkey);
    return (result == ERROR_SUCCESS) ? SUCCEED : FAILED;
}

void runCMD() {
	HWND hCMD = GetConsoleWindow();
	if (hCMD == NULL) return;
	if (IsIconic(hCMD) || !IsWindowVisible(hCMD)) ShowWindow(hCMD, SW_SHOW);
	IOPORT io;
	io.print(L"Running CMD...\nCountdown\\> ");
	wchar_t inputBuffer[64] = L"__init";
	while (inputBuffer[0] != L'\0') {
		if (isCommand(inputBuffer, 64, 0, L"__init")) io.input(inputBuffer, 64);;
		if (isCommand(inputBuffer, 64, 0, L"exit")) break;
		if (isCommand(inputBuffer, 64, 0, L"shutdown")) {
			if (isCommand(inputBuffer, 64, 1, L"byname")) {
				HWND target = FindWindowW(NULL, countdownWindowName);
				if (target) {
					SendMessage(target, WM_CLOSE, 0, 0);
					io.print(L"Operation succeed!\n");
				} else io.print(L"Unknow error occurred. Couldn't close timer window...\n");
			} else if (isCommand(inputBuffer, 64, 1, L"") || isCommand(inputBuffer, 64, 1, L"bysharedhwnd")) {
				HANDLE hMapHWND = OpenFileMappingW(FILE_MAP_READ, FALSE, L"Local\\CYX_COUNTDOWN_HWND");
    			if (hMapHWND) {
        			HWND* phwnd = (HWND*)MapViewOfFile(hMapHWND, FILE_MAP_READ, 0, 0, sizeof(HWND));
        			if (phwnd) {
            			HWND target = *phwnd;
            			UnmapViewOfFile(phwnd);
            			if (IsWindow(target)) {
                			SendMessage(target, WM_CLOSE, 0, 0);
                			io.print(L"Operation succeed!\n");
            			} else io.print(L"Timer window already closed.\n");
        			}
        			CloseHandle(hMapHWND);
    			} else io.print(L"Unknow error occurred. Couldn't close timer window...\n");
			} else io.print(L"Command \"shutdown\" doesn't have this parameter.\n");
		} else if (isCommand(inputBuffer, 64, 0, L"autostart")) {
			if (isCommand(inputBuffer, 64, 1, L"reg")) {
				if (configAutoStart(AUTOSTART::REG) == AUTOSTART::SUCCEED) io.print(L"Registration succeed!\n");
				else io.print(L"Registration failed!\n");
			} else if (isCommand(inputBuffer, 64, 1, L"remove")) {
				if (configAutoStart(AUTOSTART::REMOVE) == AUTOSTART::SUCCEED) io.print(L"Removed!\n");
				else io.print(L"Failed to remove!\n");
			} else if (isCommand(inputBuffer, 64, 1, L"check")) {
				io.print(L"Registration mode: ");
				if (configAutoStart(AUTOSTART::CHECK) == AUTOSTART::YES) io.print(L"Registrated!\n");
				else io.print(L"Not registrated.\n");
			} else io.print(L"Command \"autostart\" doesn't have this parameter.\n");
		} else if (isCommand(inputBuffer, 64, 0, L"start")) {
			HANDLE hmutex = OpenMutexW(SYNCHRONIZE, FALSE, L"CYX_COUNTDOWN_ACTIVATED");
			if (hmutex == NULL) {
				wchar_t path[MAX_PATH];
            	GetModuleFileNameW(NULL, path, MAX_PATH);
            	STARTUPINFOW si = { sizeof(si) };
            	PROCESS_INFORMATION pi;
            	if (CreateProcessW(path, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
					io.print(L"Countdown started!\n");
				else io.print(L"Couldn't create countdown process...\n");
			} else io.print(L"Countdown already existed.\n");
		} else io.print(L"No such command...\n");
		io.print(L"Countdown\\> ");
		io.input(inputBuffer, 64);
	}
	io.print(L"Exiting CMD...\n");
}

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE, LPSTR, int ncmdshow) {
	#ifndef COUNTDOWN_NO_VISTA
		SetProcessDPIAware();
	#endif
	HANDLE hmutex = OpenMutexW(SYNCHRONIZE, FALSE, L"CYX_COUNTDOWN_ACTIVATED");
	if (hmutex == NULL) runCountdown(hinstance, ncmdshow);
	else {
		CloseHandle(hmutex);
		runCMD();
	}
	IOPORT io;
	io.print(L"Finish!\n");
	Sleep(1000);
	return 0;
}
