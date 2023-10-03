#include "myState.h"

std::string targetDir = "", excludeDir = "";
HWND hEdit1, hEdit2, hEdit3, hEdit4;
HWND hwndList1, hwndList2, hwndList3;
std::atomic_flag flag1 = ATOMIC_FLAG_INIT;
std::atomic_flag flag2 = ATOMIC_FLAG_INIT;
std::atomic_flag flag3 = ATOMIC_FLAG_INIT;
WCHAR targetPid[MAX_PATH] = L"", targetProcess[MAX_PATH] = L"";