#include <windows.h>

// 宽字符输出函数，替代 wprintf
BOOL ConsolePrint(LPCWSTR szText)
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
        return FALSE;

    DWORD dwWritten;
    DWORD len = lstrlenW(szText);
    return WriteConsoleW(hOut, szText, len, &dwWritten, NULL);
}

// 宽字符输入函数，读取一行，替代 fgetws/wscanf
BOOL ConsoleInput(LPWSTR szBuf, DWORD dwBufSize)
{
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE)
        return FALSE;

    // 开启行输入、自动回显
    DWORD dwOldMode;
    GetConsoleMode(hIn, &dwOldMode);
    SetConsoleMode(hIn, dwOldMode | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);

    DWORD dwRead;
    BOOL ret = ReadConsoleW(hIn, szBuf, dwBufSize - 1, &dwRead, NULL);

    // 恢复原有控制台模式
    SetConsoleMode(hIn, dwOldMode);

    // ReadConsoleW 会把回车(\r\n)一并读入，替换成字符串结束符
    if (ret && dwRead > 0)
    {
        szBuf[dwRead] = L'\0';
        // 剔除末尾换行符
        for (DWORD i = 0; i < dwRead; i++)
        {
            if (szBuf[i] == L'\r' || szBuf[i] == L'\n')
            {
                szBuf[i] = L'\0';
                break;
            }
        }
    }
    return ret;
}

int main()
{
    WCHAR buf[256];

    // 输出提示
    ConsolePrint(L"input: \n");
    // 读取用户输入
    if (ConsoleInput(buf, ARRAYSIZE(buf)))
    {
        ConsolePrint(L"your input is: ");
        ConsolePrint(buf);
        ConsolePrint(L"\n");
    }
    else
    {
        ConsolePrint(L"failed!\n");
    }

    return 0;
}
