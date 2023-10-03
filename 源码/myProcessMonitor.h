#pragma once
#include <iostream>
#include <Windows.h>
#include <winternl.h>
#include <string>
#include <stdio.h>
#include <tlhelp32.h>
#include <winbase.h>
#include <fileapi.h>
#include <processthreadsapi.h>
#include <timezoneapi.h>
#include <vector>
#include <map>
#include <array>
#include <tchar.h>
#define SystemHandleInformation 0x10
#define SystemHandleInformationSize 1024 * 1024 * 100
#define ObjectBasicInformation 0
#define ObjectNameInformation 1
#define ObjectTypeInformation 2


using fNtQueryObject = NTSTATUS(WINAPI*)(
    HANDLE Handle,
    ULONG ObjectInformationClass,
    PVOID ObjectInformation,
    ULONG ObjectInformationLength,
    PULONG ReturnLength
    );

using fNtQuerySystemInformation = NTSTATUS(WINAPI*)(
    ULONG SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
    );

using fNtDuplicateObject = NTSTATUS(NTAPI*)(
    HANDLE SourceProcessHandle,
    HANDLE SourceHandle,
    HANDLE TargetProcessHandle,
    PHANDLE TargetHandle,
    ACCESS_MASK DesiredAccess,
    ULONG Attributes,
    ULONG Options
    );

// handle information
typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO
{
    USHORT UniqueProcessId; // handle所属的进程pid
    USHORT CreatorBackTraceIndex;
    UCHAR ObjectTypeIndex;// handle的类型代号
    UCHAR HandleAttributes;
    USHORT HandleValue; // handle的值(通过HANDLE类型转换可以转换为handle)
    PVOID Object; // handle在的地址
    ULONG GrantedAccess; // 访问权限
} SYSTEM_HANDLE_TABLE_ENTRY_INFO, * PSYSTEM_HANDLE_TABLE_ENTRY_INFO;

// handle table information
typedef struct _SYSTEM_HANDLE_INFORMATION
{
    ULONG NumberOfHandles; // 全局所有handle的数量
    SYSTEM_HANDLE_TABLE_ENTRY_INFO Handles[1]; // 存储全局handle信息的数组
} SYSTEM_HANDLE_INFORMATION, * PSYSTEM_HANDLE_INFORMATION;

std::map<DWORD, std::string> getAllProcess();
HANDLE duplicateAnotherProcessHandle(int pid, HANDLE sourceHandle);
std::string getHandleType(HANDLE handle);
std::string getHandleName(HANDLE handle);
std::vector<std::string> getProcessAllOpenFileByPid(int pid);
std::vector<std::string> getProcessAllOpenFileByName(std::string name);
char* wchar2char(wchar_t* pWCStrKey);
wchar_t* string2wchar(std::string pKey);
std::map<std::string, std::string> getDeviceAndDiskMap();
std::string translateDeviceNameToDiskFilename(std::string);
std::string getProcessName(int pid);
int getProcessPid(std::string name);