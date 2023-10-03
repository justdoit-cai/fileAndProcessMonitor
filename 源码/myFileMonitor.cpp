#include "myFileMonitor.h"
#include "myView.h"
#include "myState.h"
#include "tools.h"
#include <iostream>
#include <windows.h>

void fileMonitor(WCHAR* target, WCHAR* exclude, WCHAR* prefix, WCHAR* suffix)
{
    int        nBufferSize = 1024;
    char* buffer = new char[nBufferSize];
    HANDLE  hDirectoryHandle = NULL;


    hDirectoryHandle = ::CreateFileW(
        target,
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ
        | FILE_SHARE_WRITE
        | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS
        | FILE_FLAG_OVERLAPPED,
        NULL);

    if (hDirectoryHandle == INVALID_HANDLE_VALUE)
        return;

    WCHAR msg[MAX_PATH] = L"start monitor...";
    insertList(hwndList1, msg);

    while (1)
    {
        DWORD dwBytes = 0;

        memset(buffer, 0, nBufferSize);

        if (!::ReadDirectoryChangesW( // 这里会阻塞
            hDirectoryHandle,
            buffer,
            nBufferSize,
            true,
            FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_FILE_NAME,
            &dwBytes,
            NULL,
            NULL) || GetLastError() == ERROR_INVALID_HANDLE)
        {
            break;
        }

        if (flag1.test_and_set()) { // 设置线程退出标志
            delete buffer;
            CloseHandle(hDirectoryHandle);
            return;
        }
        flag1.clear();

        if (!dwBytes)
        {
            wsprintf(msg, L"Buffer overflow~~");
            insertList(hwndList1, msg);
        }

        PFILE_NOTIFY_INFORMATION record = (PFILE_NOTIFY_INFORMATION)buffer;
        DWORD cbOffset = 0;
        do
        {
            switch (record->Action)
            {
            case FILE_ACTION_ADDED:
                if (isTarget(record->FileName,target, exclude, prefix, suffix)) {
                    wsprintf(msg, L"create: %s", record->FileName);
                    insertList(hwndList1, msg);
                }
                break;
            case FILE_ACTION_REMOVED:
                if (isTarget(record->FileName, target, exclude, prefix, suffix)) {
                    wsprintf(msg, L"delete: %s", record->FileName);
                    insertList(hwndList1, msg);
                }
                break;
            case FILE_ACTION_MODIFIED:
                if (isTarget(record->FileName, target, exclude, prefix, suffix)) {
                    wsprintf(msg, L"modify: %s", record->FileName);
                    insertList(hwndList1, msg);
                }
                break;
            case FILE_ACTION_RENAMED_OLD_NAME:
                if (isTarget(record->FileName, target, exclude, prefix, suffix)) {
                    wsprintf(msg, L"rename(old): %s", record->FileName);
                    insertList(hwndList1, msg);
                }
                break;

            case FILE_ACTION_RENAMED_NEW_NAME:
                if (isTarget(record->FileName, target, exclude, prefix, suffix)) {
                    wsprintf(msg, L"rename(new): %s", record->FileName);
                    insertList(hwndList1, msg);
                }
                break;

            default:
                break;
            }
            cbOffset = record->NextEntryOffset;
            record = (PFILE_NOTIFY_INFORMATION)((LPBYTE)record + cbOffset);
        } while (cbOffset);
    }

    delete buffer;
    delete target;
    delete exclude;
    delete prefix;
    delete suffix;

    if (hDirectoryHandle)
        CloseHandle(hDirectoryHandle);
}

bool isTarget(WCHAR* file,WCHAR* target, WCHAR* exclude, WCHAR* prefix, WCHAR* suffix) {
    wsprintf(target, L"%s\\", target);
    WCHAR *path = fullpath(target, file);
    if (!((prefix[0] == '\0' || startsWith(file, prefix)) &&
         (suffix[0] == '\0' || endsWith(file, suffix)))) {
        delete path;
        return false;
    }
    if (exclude[0] == '\0') {
        delete path;
        return true;
    }
    if (startsWith(path, exclude)) {
        delete path;
        return false;
    }
    else {
        delete path;
        return true;
    }
}

WCHAR* fullpath(WCHAR* targetDir, WCHAR* file) {
    WCHAR* fullname = new WCHAR[MAX_PATH];
    wsprintf(fullname, L"%s\\%s", targetDir, file);
    return fullname;;
}