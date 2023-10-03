#pragma once
#include <string>
#include <Windows.h>
#include <atomic>

/*
https://www.cnblogs.com/invisible2/p/6905892.html
*/
extern std::string targetDir, excludeDir;
extern HWND hEdit1, hEdit2, hEdit3, hEdit4;
extern HWND hwndList1, hwndList2, hwndList3;
extern std::atomic_flag flag1;
extern std::atomic_flag flag2;	
extern std::atomic_flag flag3;	
extern WCHAR targetPid[MAX_PATH], targetProcess[MAX_PATH];