# fileAndProcessMonitor
## 功能

1. 通过 `Windows` 的 `ReadDirectoryChangesW` 这个 `API` 监控目标目录下的文件变化，这里提供排除目录，指定文件前缀，文件后缀的功能。`ReadDirectoryChangesW` 这个 `api` 也是 `python` 的 `watchdog` 库实现 `windows` 文件监控的底层原理。
   
2. 监测当前系统的进程启动和退出情况，不足的是这里是通过获取当前系统的进程列表，和之前的进程列表对比实现的。而不是 `hook` 进程启动（实在不会），因此这里无法监测到瞬间就运行完毕了的进程。`python` 的 `WMI` 库也是通过循环不断对比进程列表实现的监测进程。

3. 监测某个进程当前打开了哪些文件。这个功能通过 `Windows` 的 `NtQuerySystemInformation` 这个函数 `api` ，使用 `16` 号功能，遍历当前系统的所有句柄，然后通过 `NtQueryObject` 这个 `api` 得到某个句柄的类型和值，来获取某个进程所有的文件句柄的值。需要注意的是，`NtQuerySystemInformation` 这个函数不能获取全部的句柄，某些句柄获取不到可能是因为权限的问题，因为水平有限暂时还没有解决。还有 `NtQueryObject` 这个 `api` 也不是能获取所有句柄的值，但是这里测试发现文件句柄的值还是可以全部获取到的，也能够满足这里的需求。而且经过实验发现我这个代码获取到的文件句柄和微软提供的工具 `Process-explorer` 查看的进程文件句柄是一样的，顺序也是一样的，大概率这个工具也是这么实现的。

## 设计思路

### 文件监控 
文件的监控利用的是 `windows` 原生提供的 `ReadDirectoryChangesW` 函数。核心代码如下：
这里不足的就是由于这个函数底层实现的机制，导致创建文件会产生两次消息。

```cpp
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
				wsprintf(msg, L"%d. create: %s", ++tot, record->FileName);
				insertList(hwndList1, msg);
			}
			break;
		case FILE_ACTION_REMOVED:
			if (isTarget(record->FileName, target, exclude, prefix, suffix)) {
				wsprintf(msg, L"%d. delete: %s", ++tot, record->FileName);
				insertList(hwndList1, msg);
			}
			break;
		case FILE_ACTION_MODIFIED:
			if (isTarget(record->FileName, target, exclude, prefix, suffix)) {
				wsprintf(msg, L"%d. modify: %s", ++tot, record->FileName);
				insertList(hwndList1, msg);
			}
			break;
		case FILE_ACTION_RENAMED_OLD_NAME:
			if (isTarget(record->FileName, target, exclude, prefix, suffix)) {
				wsprintf(msg, L"%d. rename(old): %s", ++tot, record->FileName);
				insertList(hwndList1, msg);
			}
			break;

		case FILE_ACTION_RENAMED_NEW_NAME:
			if (isTarget(record->FileName, target, exclude, prefix, suffix)) {
				wsprintf(msg, L"%d. rename(new): %s", ++tot, record->FileName);
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
```

### 进程监控

进程监控是这个软件的核心难点。获取所有的进程句柄不难，但是获取某个进程的信息就有点麻烦。这里经过搜索发现了 `NtQuerySystemInformation` 函数提供了这个功能。但是由于权限的问题，我们也只能获取部分进程的信息，这点我还不知道怎么解决。还有需要注意的是，`NtQuerySystemInformation` 这个函数无法直接使用，而是需要从 `dll` 中导出后才可以用。导出函数的方法如下：

```cpp
using fNtQuerySystemInformation = NTSTATUS(WINAPI*)(
    ULONG SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
    );
// ...
fNtQuerySystemInformation NtQuerySystemInformation = (fNtQuerySystemInformation)GetProcAddress(GetModuleHandle(L"ntdll"), "NtQuerySystemInformation");
```

这个函数使用的方式如下：

> 这里还需要一个注意点，就是句柄需要复制之后才能获取到信息。


```cpp
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
```

## 有待完善的地方

1. 通过 `hook` 系统函数，实现真正的实时监控进程的瞬时启动，从而瞬间结束的进程也可以监测到。据说 `Windows` 的进程启动都会经过 `OpenProcess` 这个函数，因此我们可以监测这个函数的调用来监测进程的启动。但是我不会。

2. `NtQuerySystemInformation` 只能得到部分句柄，系统的进程句柄无法获取到，还有进程的部分句柄也获取不到，但是这里普通进程的文件句柄是获取的齐全的。最好还是可以获取到系统进程的文件句柄。

3. 由于我 `C` 语言基础不好，导致可能代码中出现了很多内存泄露的问题，让程序在运行的时候内存占用会不断增加，但是暂时测试增加的速度比较慢，还可以接受。我也懒得完善了。这个项目只是提供一个思路参考。

## 参考资料

```
# 监测windows的句柄和进程微软提供两个工具：process-explorer 和 openprocmon
# process-explorer的下载(可以获取进程当前打开了哪些句柄)
https://learn.microsoft.com/en-us/sysinternals/downloads/process-explorer
# openprocmon 的源码
https://github.com/progmboy/openprocmon
# 使用NtQuerySystemInformation遍历系统句柄的参考代码（部分句柄无法获取）
https://www.ired.team/miscellaneous-reversing-forensics/windows-kernel-internals/get-all-open-handles-and-kernel-object-address-from-userland
```
