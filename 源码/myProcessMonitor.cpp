#include "myProcessMonitor.h"
#include "tools.h"
#include "myState.h"


/*
通过进程的pid获取进程的名称
*/
std::string getProcessName(int pid) {
    // 遍历进程
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    BOOL bRet = Process32First(hSnap, &pe32);
    while (bRet) {
        bRet = Process32Next(hSnap, &pe32);
        if (pe32.th32ProcessID == pid) {
            return wchar2string(pe32.szExeFile);
        }
    }
    return "";
}

/*
通过进程的名称获取进程的pid
*/
int getProcessPid(std::string name) {
    std::map<DWORD, std::string> ret;

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    BOOL bRet = Process32First(hSnap, &pe32);

    while (bRet) {
        if (name == wchar2string(pe32.szExeFile)) {
            return pe32.th32ProcessID;
        }
        bRet = Process32Next(hSnap, &pe32);
    }
    return -1;
}

/*
根据进程pid获取指定进程所有打开的文件句柄
(实践的和ProcessExplorer的结果完全一致，顺序也一致，大概率ProcessExplorer的源码就是这样的)
但是不知道为什么有的句柄无法遍历到，应该是权限问题
https://www.ired.team/miscellaneous-reversing-forensics/windows-kernel-internals/get-all-open-handles-and-kernel-object-address-from-userland
*/
std::vector<std::string> getProcessAllOpenFileByPid(int pid) {
    fNtQuerySystemInformation NtQuerySystemInformation = (fNtQuerySystemInformation)GetProcAddress(GetModuleHandle(L"ntdll"), "NtQuerySystemInformation");

    std::vector<std::string> ret;
    ULONG returnLength = 0;
    PSYSTEM_HANDLE_INFORMATION handleTableInformation = (PSYSTEM_HANDLE_INFORMATION)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, SystemHandleInformationSize);

    // 这里的SystemHandleInformationSize尽量大一点，不然后面可能会报错0xC0000005: 读取位置 xxx时发生访问冲突，导致崩溃。
    NtQuerySystemInformation(SystemHandleInformation, handleTableInformation, SystemHandleInformationSize, &returnLength);

    for (int i = 0; i < handleTableInformation->NumberOfHandles; i++) { // 遍历全部句柄
        SYSTEM_HANDLE_TABLE_ENTRY_INFO handleInfo = handleTableInformation->Handles[i];

        if (handleInfo.UniqueProcessId != pid) { // 筛选目标进程的句柄
            continue;
        }
        
        HANDLE dupHandle = duplicateAnotherProcessHandle(handleInfo.UniqueProcessId, (HANDLE)handleInfo.HandleValue);

        if (dupHandle == nullptr) { // 无法访问目标句柄
            continue;
        }

        std::string handleType = getHandleType(dupHandle); // 获取句柄类型
        std::string handleName = getHandleName(dupHandle); // 获取句柄名称

        if (handleType != "File") { // 不为文件句柄
            continue;
        }
        ret.push_back(translateDeviceNameToDiskFilename(handleName));
    }


    // https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapalloc
    // https://blog.csdn.net/fuck487/article/details/70144172
    //delete handleTableInformation;
    HeapFree(GetProcessHeap(), 0, handleTableInformation); // 不然会内存溢出
    return ret;
}


/*
根据进程名获取某个进程打开的文件
*/
std::vector<std::string> getProcessAllOpenFileByName(std::string name) {
    int pid = getProcessPid(name);
    return getProcessAllOpenFileByPid(pid);
}
/*
获取所有的进程（进程名+进程pid）
*/
std::map<DWORD, std::string> getAllProcess() {
    std::map<DWORD, std::string> ret;

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    BOOL bRet = Process32First(hSnap, &pe32);

    while (bRet) {
        ret.insert(std::make_pair(pe32.th32ProcessID, wchar2string(pe32.szExeFile)));
        bRet = Process32Next(hSnap, &pe32);
    }

    return ret;
}

/*
将某个进程的某个handle复制到当前进程(因为每个进程的句柄表是不一样的)，这样才能查询目标handle的信息。
不能保证每个句柄都可以成功复制，但是文件句柄是可以成功复制的，其它句柄可能存在权限问题而导致无法复制
（TODO: 这里暂时还不知道怎么解决权限问题）。
*/
HANDLE duplicateAnotherProcessHandle(int pid, HANDLE sourceHandle) {
    fNtDuplicateObject NtDuplicateObject = (fNtDuplicateObject)GetProcAddress(GetModuleHandle(L"ntdll"), "NtDuplicateObject");

    HANDLE dupHandle = nullptr;
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (hProcess == INVALID_HANDLE_VALUE) {
        printf("pid: %u, ERROR in OpenProcess()\n", pid);
    }
    if (!NT_SUCCESS(NtDuplicateObject(
        hProcess, // 某个指定进程的handle
        sourceHandle, // 要复制的某个handle（这个handle需要属于上面指定进程）
        GetCurrentProcess(), // 将接收新句柄的目标进程的句柄
        &dupHandle,  // 指向 HANDLE 变量的指针，例程将新的复制句柄写入其中
        0,
        0,
        0
    ))) {
        /*printf("pid: %u, ERROR in duplicateAnotherProcessHandle()\n", pid);*/
    }
    return dupHandle;
}


/*
根据handle获取handle的类型
*/
std::string getHandleType(HANDLE handle) {
    fNtQueryObject NtQueryObject = (fNtQueryObject)GetProcAddress(GetModuleHandle(L"ntdll"), "NtQueryObject");

    ULONG uRetLength = 0;
    char* Buffer = new char[0x1000]();
    NtQueryObject(handle, ObjectTypeInformation, Buffer, 0x1000, &uRetLength);

    PPUBLIC_OBJECT_TYPE_INFORMATION pBuffer = reinterpret_cast<PPUBLIC_OBJECT_TYPE_INFORMATION>(Buffer);  //查询类型信息

    return wchar2string(pBuffer->TypeName.Buffer);
}

/*
根据handle获取handle的值（这里只要file handle，但是包括非普通文件，如\Device\NamedPipe等特殊文件）
*/
std::string getHandleName(HANDLE handle) {
    if (handle == INVALID_HANDLE_VALUE || handle == nullptr) {
        printf("ERROR invalid handle value\n");
    }

    fNtQueryObject NtQueryObject = (fNtQueryObject)GetProcAddress(GetModuleHandle(L"ntdll"), "NtQueryObject");

    ULONG returnLength = 0;
    PVOID objectNameInfo = malloc(0x1000);

    if (!NT_SUCCESS(NtQueryObject(handle, ObjectNameInformation, objectNameInfo, 0x1000, &returnLength))) {
        printf("ERROR first in NtQueryObject\n");
    } {
        objectNameInfo = realloc(objectNameInfo, returnLength);
        if (!NT_SUCCESS(NtQueryObject(
            handle,
            ObjectNameInformation,
            objectNameInfo,
            returnLength,
            nullptr
        ))) {
            printf("ERROR second in NtQueryObject\n");
            return "can't get target handle name";
        }
    }

    UNICODE_STRING objectName = *(PUNICODE_STRING)objectNameInfo;
    if (objectName.Length == 0) {
        return "unamed";
    }
    else {
        return wchar2string(objectName.Buffer);
    }
}

