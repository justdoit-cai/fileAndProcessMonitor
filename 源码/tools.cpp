#include "tools.h"
/*
wchar_t转char*(https://blog.csdn.net/sinat_35261315/article/details/72636712)
*/
char* wchar2char(wchar_t* pWCStrKey) {
    //第一次调用确认转换后单字节字符串的长度，用于开辟空间
    int pSize = WideCharToMultiByte(CP_OEMCP, 0, pWCStrKey, wcslen(pWCStrKey), NULL, 0, NULL, NULL);
    char* pCStrKey = new char[pSize + 1];
    //第二次调用将双字节字符串转换成单字节字符串
    WideCharToMultiByte(CP_OEMCP, 0, pWCStrKey, wcslen(pWCStrKey), pCStrKey, pSize, NULL, NULL);
    pCStrKey[pSize] = '\0';
    return pCStrKey;
}

/*
string转wchar_t
*/
wchar_t* string2wchar(std::string pKey)
{
    const char* pCStrKey = pKey.c_str();
    //第一次调用返回转换后的字符串长度，用于确认为wchar_t*开辟多大的内存空间
    int pSize = MultiByteToWideChar(CP_OEMCP, 0, pCStrKey, strlen(pCStrKey) + 1, NULL, 0);
    wchar_t* pWCStrKey = new wchar_t[pSize];
    //第二次调用将单字节字符串转换成双字节字符串
    MultiByteToWideChar(CP_OEMCP, 0, pCStrKey, strlen(pCStrKey) + 1, pWCStrKey, pSize);
    return pWCStrKey;
}

std::string wchar2string(WCHAR* p) {
    return wchar2char(p);
}


/*
设备名转盘符无法直接转换，需要通过枚举出所有设备名的盘符，通过其对应关系找到对应的盘符。
键值对：\Device\HarddiskVolume4 -> C:
*/
std::map<std::string, std::string> getDeviceAndDiskMap() {
    std::map<std::string, std::string> ret;
    TCHAR disk = 0;
    TCHAR diskSymbol[] = _T("C:");

    for (disk = 'C'; disk <= 'Z'; disk++) {
        diskSymbol[0] = disk;
        TCHAR device[MAX_PATH] = { 0 };
        QueryDosDevice(diskSymbol, device, MAX_PATH);
        if (device[0] != 0) {
            ret.insert(std::make_pair(wchar2string(device), wchar2string(diskSymbol)));
        }
    }
    return ret;
}

/*
将\Device\HarddiskVolume6\Windows的文件路径转为C:\Windows盘符格式
*/
std::string translateDeviceNameToDiskFilename(std::string sourceFilename) {
    if (sourceFilename.substr(0, 22) != "\\Device\\HarddiskVolume") { // 无法转换为盘符的路径直接原样返回
        return sourceFilename;
    }
    std::map<std::string, std::string> m = getDeviceAndDiskMap();
    int idx = sourceFilename.find('\\', 8);
    std::string prefix = sourceFilename.substr(0, idx);
    auto i = m.find(prefix);
    if (i == m.end()) {
        return sourceFilename; // 这里无法正常转换
    }
    else {
        return m[prefix] + sourceFilename.substr(idx);
    }
}

bool startsWith(WCHAR* a, WCHAR* b) {
    std::string _a = wchar2char(a);
    std::string _b = wchar2char(b);
    if (_a.find(_b) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool endsWith(WCHAR* a, WCHAR* b) {
    std::string _a = wchar2char(a);
    std::string _b = wchar2char(b);
    int _a_len = _a.length();
    int _b_len = _b.length();
    if (_a.find(_b) == _a_len - _b_len) { 
        return true;
    }
    else {
        return false;
    }
}

bool equal(WCHAR* a, WCHAR* b) {
    return wchar2char(a) == wchar2char(b);
}

WCHAR* getNow() {
    SYSTEMTIME sys;
    GetLocalTime(&sys);
    WCHAR *now = new WCHAR[MAX_PATH];
    wsprintf(now, L"%02d:%02d:%02d", sys.wHour, sys.wMinute, sys.wSecond);
    return now;
}