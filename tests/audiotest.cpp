#include <windows.h>
#include <stdio.h>
#include <mmsystem.h>

void sayNum(DWORD num) {
	BYTE singleNums[10], i = 0, j = 0;
	DWORD num_copy = num;
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
	nameBuffer[j] = L'\0';
	wprintf(L"%u: %ls\n", num_copy, nameBuffer);
	/*
	wchar_t pathBuffer[14];
	for (i = 0; i < j; i++) {
		// swprintf(pathBuffer, L".\\audio\\%c.wav", nameBuffer[i]);
		PlaySoundW(pathBuffer, NULL, SND_FILENAME | SND_SYNC);
		// wprintf(L"%ls\n", pathBuffer);
	}
	// wprintf(L"#######################################\n");
	*/
}

int main() {
	const DWORD nums[] = {
		1, 2, 9, 0, 10, 14, 20, 36, 100, 200, 310, 502, 2000, 6000, 4100, 4030, 4005, 4602, 4800, 1234, 20000, 70000,
		70002, 90100, 100000, 120000, 200000, 234001, 2000000, 2100000, 3000000, 5678999, 12345678, 20000000, 30000000,
		123456789, 1234567890, 200000000, 300000000, 2000000000, 3000000000, 3100000000, 4000040000, 4294967295
	};
	for (BYTE i = 0; i < sizeof(nums) / sizeof(DWORD); i++) sayNum(nums[i]);
	return 0;
}
