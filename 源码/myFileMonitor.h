#pragma once
#include <Windows.h>
#include <fileapi.h>
#include <string>
#include <iostream>
#include <atomic>

void fileMonitor(WCHAR* file, WCHAR* exclude, WCHAR* prefix, WCHAR* suffix);
bool isTarget(WCHAR* file, WCHAR* target ,WCHAR* exclude, WCHAR* prefix, WCHAR* suffix);
WCHAR* fullpath(WCHAR* targetDir, WCHAR* file);