#include <windows.h>

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
	DWORD input(wchar_t* buffer, DWORD maxlen) {
		DWORD read;
		ReadConsoleW(hin, buffer, maxlen, &read, NULL);
		return read;
	}
};

namespace AUTOSTART {
	bool isAutoStart() {
		HKEY hkey = NULL;
    	LONG result = RegOpenKeyExW(
			HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &hkey
    	);
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
BYTE configAutoStart(BYTE mode) {
	using namespace AUTOSTART;
	HKEY hKey = NULL;
    LONG ret = RegOpenKeyExW(
        HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",  // no OP required!
        0, KEY_WRITE, &hKey
    );
    if (ret != ERROR_SUCCESS) return 0;
    switch (mode) {
    	case CHECK: return isAutoStart() ? YES : NO;
    	case REG: {
    		wchar_t szPath[MAX_PATH] = { 0 };
    		GetModuleFileNameW(NULL, szPath, MAX_PATH);
    		ret = RegSetValueExW(
				hKey, L"CYX_COUNTDOWN", 0, REG_SZ, (const BYTE*)szPath, (wcslen(szPath) + 1) * sizeof(wchar_t)
    		);
			break;
		}
		case REMOVE: ret = RegDeleteValueW(hKey, L"CYX_COUNTDOWN"); break;
	}
    RegCloseKey(hKey);
    return (ret == ERROR_SUCCESS) ? SUCCEED : FAILED;
}


int main() {
	IOPORT io;
	io.print(L"autostart? ");
	if (configAutoStart(AUTOSTART::CHECK) == AUTOSTART::YES) io.print(L"Yes!\n");
	else io.print(L"Nope.\n");
	
	if (configAutoStart(AUTOSTART::REG) == AUTOSTART::SUCCEED) io.print(L"Registration succeed!\n");
	else io.print(L"Registration failed!\n");
	
	io.print(L"autostart? ");
	if (configAutoStart(AUTOSTART::CHECK) == AUTOSTART::YES) io.print(L"Yes!\n");
	else io.print(L"Nope.\n");
	
	if (configAutoStart(AUTOSTART::REMOVE) == AUTOSTART::SUCCEED) io.print(L"Removed!\n");
	else io.print(L"Failed to remove!\n");
	
	io.print(L"autostart? ");
	if (configAutoStart(AUTOSTART::CHECK) == AUTOSTART::YES) io.print(L"Yes!\n");
	else io.print(L"Nope.\n");
	
	return 0;
}
