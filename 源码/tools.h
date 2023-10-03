#pragma once
#include <Windows.h>
#include <string>
#include <map>
#include <vector>
#include <tchar.h>
#include <time.h>

char* wchar2char(wchar_t* pWCStrKey);
wchar_t* string2wchar(std::string pKey);
std::string wchar2string(WCHAR* a);
std::map<std::string, std::string> getDeviceAndDiskMap();
std::string translateDeviceNameToDiskFilename(std::string);
bool startsWith(WCHAR* a, WCHAR* b);
bool endsWith(WCHAR* a, WCHAR* b);
bool equal(WCHAR* a, WCHAR* b);
WCHAR* getNow();