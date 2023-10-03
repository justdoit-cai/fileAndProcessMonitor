#include "myProcessMonitor.h"
#include "tools.h"
#include "myState.h"


/*
ͨ�����̵�pid��ȡ���̵�����
*/
std::string getProcessName(int pid) {
    // ��������
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
ͨ�����̵����ƻ�ȡ���̵�pid
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
���ݽ���pid��ȡָ���������д򿪵��ļ����
(ʵ���ĺ�ProcessExplorer�Ľ����ȫһ�£�˳��Ҳһ�£������ProcessExplorer��Դ�����������)
���ǲ�֪��Ϊʲô�еľ���޷���������Ӧ����Ȩ������
https://www.ired.team/miscellaneous-reversing-forensics/windows-kernel-internals/get-all-open-handles-and-kernel-object-address-from-userland
*/
std::vector<std::string> getProcessAllOpenFileByPid(int pid) {
    fNtQuerySystemInformation NtQuerySystemInformation = (fNtQuerySystemInformation)GetProcAddress(GetModuleHandle(L"ntdll"), "NtQuerySystemInformation");

    std::vector<std::string> ret;
    ULONG returnLength = 0;
    PSYSTEM_HANDLE_INFORMATION handleTableInformation = (PSYSTEM_HANDLE_INFORMATION)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, SystemHandleInformationSize);

    // �����SystemHandleInformationSize������һ�㣬��Ȼ������ܻᱨ��0xC0000005: ��ȡλ�� xxxʱ�������ʳ�ͻ�����±�����
    NtQuerySystemInformation(SystemHandleInformation, handleTableInformation, SystemHandleInformationSize, &returnLength);

    for (int i = 0; i < handleTableInformation->NumberOfHandles; i++) { // ����ȫ�����
        SYSTEM_HANDLE_TABLE_ENTRY_INFO handleInfo = handleTableInformation->Handles[i];

        if (handleInfo.UniqueProcessId != pid) { // ɸѡĿ����̵ľ��
            continue;
        }
        
        HANDLE dupHandle = duplicateAnotherProcessHandle(handleInfo.UniqueProcessId, (HANDLE)handleInfo.HandleValue);

        if (dupHandle == nullptr) { // �޷�����Ŀ����
            continue;
        }

        std::string handleType = getHandleType(dupHandle); // ��ȡ�������
        std::string handleName = getHandleName(dupHandle); // ��ȡ�������

        if (handleType != "File") { // ��Ϊ�ļ����
            continue;
        }
        ret.push_back(translateDeviceNameToDiskFilename(handleName));
    }


    // https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapalloc
    // https://blog.csdn.net/fuck487/article/details/70144172
    //delete handleTableInformation;
    HeapFree(GetProcessHeap(), 0, handleTableInformation); // ��Ȼ���ڴ����
    return ret;
}


/*
���ݽ�������ȡĳ�����̴򿪵��ļ�
*/
std::vector<std::string> getProcessAllOpenFileByName(std::string name) {
    int pid = getProcessPid(name);
    return getProcessAllOpenFileByPid(pid);
}
/*
��ȡ���еĽ��̣�������+����pid��
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
��ĳ�����̵�ĳ��handle���Ƶ���ǰ����(��Ϊÿ�����̵ľ�����ǲ�һ����)���������ܲ�ѯĿ��handle����Ϣ��
���ܱ�֤ÿ����������Գɹ����ƣ������ļ�����ǿ��Գɹ����Ƶģ�����������ܴ���Ȩ������������޷�����
��TODO: ������ʱ����֪����ô���Ȩ�����⣩��
*/
HANDLE duplicateAnotherProcessHandle(int pid, HANDLE sourceHandle) {
    fNtDuplicateObject NtDuplicateObject = (fNtDuplicateObject)GetProcAddress(GetModuleHandle(L"ntdll"), "NtDuplicateObject");

    HANDLE dupHandle = nullptr;
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (hProcess == INVALID_HANDLE_VALUE) {
        printf("pid: %u, ERROR in OpenProcess()\n", pid);
    }
    if (!NT_SUCCESS(NtDuplicateObject(
        hProcess, // ĳ��ָ�����̵�handle
        sourceHandle, // Ҫ���Ƶ�ĳ��handle�����handle��Ҫ��������ָ�����̣�
        GetCurrentProcess(), // �������¾����Ŀ����̵ľ��
        &dupHandle,  // ָ�� HANDLE ������ָ�룬���̽��µĸ��ƾ��д������
        0,
        0,
        0
    ))) {
        /*printf("pid: %u, ERROR in duplicateAnotherProcessHandle()\n", pid);*/
    }
    return dupHandle;
}


/*
����handle��ȡhandle������
*/
std::string getHandleType(HANDLE handle) {
    fNtQueryObject NtQueryObject = (fNtQueryObject)GetProcAddress(GetModuleHandle(L"ntdll"), "NtQueryObject");

    ULONG uRetLength = 0;
    char* Buffer = new char[0x1000]();
    NtQueryObject(handle, ObjectTypeInformation, Buffer, 0x1000, &uRetLength);

    PPUBLIC_OBJECT_TYPE_INFORMATION pBuffer = reinterpret_cast<PPUBLIC_OBJECT_TYPE_INFORMATION>(Buffer);  //��ѯ������Ϣ

    return wchar2string(pBuffer->TypeName.Buffer);
}

/*
����handle��ȡhandle��ֵ������ֻҪfile handle�����ǰ�������ͨ�ļ�����\Device\NamedPipe�������ļ���
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

