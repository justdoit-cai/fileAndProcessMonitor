#include "framework.h"
#include "myView.h"
#include <commctrl.h>
#include <strsafe.h>
#include <atlstr.h>
#include <stdio.h>
#include "shlwapi.h"
#include "shlobj.h"
#include "tools.h"
#include "myFileMonitor.h"
#include "myProcessMonitor.h"
#include <thread>
#include "myState.h"


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PWSTR lpCmdLine, int nCmdShow) {

    MSG  msg;
    WNDCLASSW wc = { 0 };
    wc.lpszClassName = L"main";
    wc.hInstance = hInstance;
    wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
    wc.lpfnWndProc = WndProc;
    wc.hCursor = LoadCursor(0, IDC_ARROW);

    RegisterClassW(&wc);
    CreateWindowW(wc.lpszClassName, L"进程/目录文件监控工具",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        200, 100, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, hInstance, 0);

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg,
    WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_PAINT:
        drawLine(hwnd);
        break;
    case WM_CREATE:
        drawLabel(hwnd);
        drawButton(hwnd);
        drawEdit(hwnd);
        drawList(hwnd);
        startDisplayAllProcess();
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_BTN1_CALLBACK) {
            btn1_callback();
        }
        if (LOWORD(wParam) == ID_BTN2_CALLBACK) {
            btn2_callback();
        }
        if (LOWORD(wParam) == ID_BTN3_CALLBACK) {
            btn3_callback();
        }
        if (LOWORD(wParam) == ID_BTN4_CALLBACK) {
            btn4_callback();
        }
        if (LOWORD(wParam) == ID_BTN5_CALLBACK) {
            btn5_callback();
        }
        if (LOWORD(wParam) == ID_BTN6_CALLBACK) {
            btn6_callback();
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void drawLine(HWND hwnd) {
    HDC         hdc;
    PAINTSTRUCT ps;
    hdc = BeginPaint(hwnd, &ps);

    MoveToEx(hdc, WINDOW_WIDTH / 2, 0, NULL);  //起点
    LineTo(hdc, WINDOW_WIDTH / 2, WINDOW_HEIGHT); //终点
    MoveToEx(hdc, 0, 35, NULL);  //起点
    LineTo(hdc, WINDOW_WIDTH, 35); //终点

    EndPaint(hwnd, &ps);
}

void drawLabel(HWND hwnd) {
    // label标签
    HWND hwndStatic1, hwndStatic2, hwndStatic3, hwndStatic4, hwndStatic5, hwndStatic6, hwndStatic7, hwndStatic8, hwndStatic9;
    hwndStatic1 = CreateWindowW(WC_STATICW, NULL, WS_CHILD | WS_VISIBLE,
        250, 10, 120, 25, hwnd, (HMENU)IDC_STATIC, NULL, NULL);
    SetWindowTextW(hwndStatic1, L"目录文件监控");
    hwndStatic2 = CreateWindowW(WC_STATICW, NULL, WS_CHILD | WS_VISIBLE,
        850, 10, 120, 25, hwnd, (HMENU)IDC_STATIC, NULL, NULL);
    SetWindowTextW(hwndStatic2, L"进程文件监控");
    hwndStatic3 = CreateWindowW(WC_STATICW, NULL, WS_CHILD | WS_VISIBLE,
        50, 105, 120, 25, hwnd, (HMENU)IDC_STATIC, NULL, NULL);
    SetWindowTextW(hwndStatic3, L"过滤文件前缀");
    hwndStatic4 = CreateWindowW(WC_STATICW, NULL, WS_CHILD | WS_VISIBLE,
        340, 105, 120, 25, hwnd, (HMENU)IDC_STATIC, NULL, NULL);
    SetWindowTextW(hwndStatic4, L"过滤文件后缀");
    hwndStatic5 = CreateWindowW(WC_STATICW, NULL, WS_CHILD | WS_VISIBLE,
        260, 175, 120, 25, hwnd, (HMENU)IDC_STATIC, NULL, NULL);
    SetWindowTextW(hwndStatic5, L"输出日志");

    //
    hwndStatic6 = CreateWindowW(WC_STATICW, NULL, WS_CHILD | WS_VISIBLE,
        WINDOW_WIDTH / 8 * 5 - 70, 55, 120, 25, hwnd, (HMENU)IDC_STATIC, NULL, NULL);
    SetWindowTextW(hwndStatic6, L"目标进程pid");
    hwndStatic7 = CreateWindowW(WC_STATICW, NULL, WS_CHILD | WS_VISIBLE,
        WINDOW_WIDTH / 8 * 5 - 70, 105, 120, 25, hwnd, (HMENU)IDC_STATIC, NULL, NULL);
    SetWindowTextW(hwndStatic7, L"目标进程名");
    hwndStatic8 = CreateWindowW(WC_STATICW, NULL, WS_CHILD | WS_VISIBLE,
        850, 150, 120, 25, hwnd, (HMENU)IDC_STATIC, NULL, NULL);
    SetWindowTextW(hwndStatic8, L"当前系统进程");
    hwndStatic9 = CreateWindowW(WC_STATICW, NULL, WS_CHILD | WS_VISIBLE,
        850, 420, 120, 25, hwnd, (HMENU)IDC_STATIC, NULL, NULL);
    SetWindowTextW(hwndStatic9, L"输出日志");
}

void drawEdit(HWND hwnd) {
    int x1_window1 = WINDOW_WIDTH / 8;
    int x2_window1 = WINDOW_WIDTH / 8 * 3;
    int y_window1 = 100;
    hEdit1 = CreateWindowW(WC_EDITW, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
        x1_window1, y_window1, 100, 25, hwnd, (HMENU)ID_EDIT1, NULL, NULL);
    hEdit2 = CreateWindowW(WC_EDITW, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
        x2_window1, y_window1, 100, 25, hwnd, (HMENU)ID_EDIT2, NULL, NULL);

    int x_window2 = WINDOW_WIDTH / 8 * 5 + 40;
    int y1_window2 = 55;
    int y2_window2 = 105;
    hEdit3 = CreateWindowW(WC_EDITW, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
        x_window2, y1_window2, 100, 25, hwnd, (HMENU)ID_EDIT1, NULL, NULL);
    hEdit4 = CreateWindowW(WC_EDITW, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
        x_window2, y2_window2, 100, 25, hwnd, (HMENU)ID_EDIT1, NULL, NULL);
}

/*
https://blog.csdn.net/qq_31178679/article/details/125904122
https://juejin.cn/post/7209201396847968311
处理水平滚动条
https://blog.csdn.net/zeng622peng/article/details/5547064
*/
void drawList(HWND hwnd) {
    // 
    hwndList1 = CreateWindowW(WC_LISTBOXW, NULL, WS_CHILD
        | WS_VISIBLE |// 是否可见（一般必配）
        LBS_NOTIFY | // 每当用户单击列表框项 (LBN_SELCHANGE) 时，列表框将通知代码发送到父窗口，双击 (LBN_DBLCLK) 的项目，或取消所选内容 (LBN_SELCANCEL) 。
        WS_BORDER | // 边框加黑
        LBS_DISABLENOSCROLL | WS_HSCROLL | WS_VSCROLL, // 配置滚动条
        10, 200, 580, 561, hwnd,
        (HMENU)IDC_LIST1, NULL, NULL);

    // 
    hwndList2 = CreateWindowW(WC_LISTBOXW, NULL, WS_CHILD
        | WS_VISIBLE |// 是否可见（一般必配）
        LBS_NOTIFY | // 每当用户单击列表框项 (LBN_SELCHANGE) 时，列表框将通知代码发送到父窗口，双击 (LBN_DBLCLK) 的项目，或取消所选内容 (LBN_SELCANCEL) 。
        WS_BORDER | // 边框加黑
        //LBS_SORT | // 按照字母排序
        LBS_DISABLENOSCROLL | WS_HSCROLL | WS_VSCROLL, // 配置滚动条
        610, 180, 565, 230, hwnd,
        (HMENU)IDC_LIST2, NULL, NULL);

    // 
    hwndList3 = CreateWindowW(WC_LISTBOXW, NULL, WS_CHILD
        | WS_VISIBLE |// 是否可见（一般必配）
        LBS_NOTIFY | // 每当用户单击列表框项 (LBN_SELCHANGE) 时，列表框将通知代码发送到父窗口，双击 (LBN_DBLCLK) 的项目，或取消所选内容 (LBN_SELCANCEL) 。
        WS_BORDER | // 边框加黑
        LBS_DISABLENOSCROLL | WS_HSCROLL | WS_VSCROLL, // 配置滚动条
        610, 450, 565, 310, hwnd,
        (HMENU)IDC_LIST3, NULL, NULL);
}

void insertList(HWND hwndList, WCHAR* content) {
    HDC hdc;
    SIZE s;
    hdc = GetDC(hwndList);
    GetTextExtentPoint32(hdc, content, lstrlen(content), &s); // 获取content的尺寸
    if (s.cx > (LONG)SendMessage(hwndList, LB_GETHORIZONTALEXTENT, 0, 0)) { // 动态修改水平滚动条
        SendMessage(hwndList, LB_SETHORIZONTALEXTENT, (WPARAM)s.cx + 100, 0);
    }
    SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)content);
    ReleaseDC(hwndList, hdc);
}

void drawButton(HWND hwnd) {
    int buttonWidth = WINDOW_WIDTH / 8;
    int buttonHeight = 35;
    int x1 = WINDOW_WIDTH / 16;
    int x2 = WINDOW_WIDTH / 16 * 5;
    int y = 50;
    CreateWindowW(L"Button", L"选择监控的目录",
        WS_VISIBLE | WS_CHILD,
        x1, y, buttonWidth, buttonHeight, hwnd, (HMENU)ID_BTN1_CALLBACK, NULL, NULL);
    CreateWindowW(L"Button", L"选择排除的目录",
        WS_VISIBLE | WS_CHILD,
        x2, y, buttonWidth, buttonHeight, hwnd, (HMENU)ID_BTN2_CALLBACK, NULL, NULL);

    y = 135;
    CreateWindowW(L"Button", L"启动",
        WS_VISIBLE | WS_CHILD,
        x1, y, buttonWidth, buttonHeight, hwnd, (HMENU)ID_BTN3_CALLBACK, NULL, NULL);
    CreateWindowW(L"Button", L"暂停",
        WS_VISIBLE | WS_CHILD,
        x2, y, buttonWidth, buttonHeight, hwnd, (HMENU)ID_BTN4_CALLBACK, NULL, NULL);

    int x = WINDOW_WIDTH / 16 * 13;
    int y1 = 50, y2 = 100;
    CreateWindowW(L"Button", L"启动",
        WS_VISIBLE | WS_CHILD,
        x, y1, buttonWidth, buttonHeight, hwnd, (HMENU)ID_BTN5_CALLBACK, NULL, NULL);
    CreateWindowW(L"Button", L"暂停",
        WS_VISIBLE | WS_CHILD,
        x, y2, buttonWidth, buttonHeight, hwnd, (HMENU)ID_BTN6_CALLBACK, NULL, NULL);
}

void btn1_callback() {
    targetDir = selectDir();
}

void btn2_callback() {
    excludeDir = selectDir();
}

void btn3_callback() {
    std::string _prefix, _suffix, msg;
    WCHAR *prefix = new WCHAR[MAX_PATH], *suffix = new WCHAR[MAX_PATH];
    if (targetDir == "") {
        MessageBox(NULL, L"请先选择监控的目标目录", TEXT("提示消息"), MB_OK);
        return;
    }
    GetWindowTextW(hEdit1, prefix, 250);
    GetWindowTextW(hEdit2, suffix, 250);
    _prefix = wchar2char(prefix);
    _suffix = wchar2char(suffix);
    msg = "启动目录监控(目标目录:" + targetDir + ",排除目录:" + excludeDir + ",过滤前缀:" + _prefix + ",过滤后缀:" + _suffix + ")";
    MessageBox(NULL, string2wchar(msg), TEXT("提示消息"), MB_OK);

    flag1.clear();
    std::thread t(fileMonitor, string2wchar(targetDir), string2wchar(excludeDir) , prefix, suffix);
    t.detach();
}

void btn4_callback() {
    MessageBox(NULL, L"停止目录监控", TEXT("提示消息"), MB_OK);
    flag1.test_and_set();
    WCHAR msg[] = L"stop monitor...";
    insertList(hwndList1, msg);
}


void btn5_callback() {
    GetWindowTextW(hEdit3, targetPid, 250);
    GetWindowTextW(hEdit4, targetProcess, 250);
    if (targetPid[0] == '\0' && targetProcess[0] == '\0') {
        MessageBox(NULL, L"请先选择目标pid或者目标进程名", TEXT("提示消息"), MB_OK);
        return;
    }
    WCHAR msg[MAX_PATH];
    WCHAR* now = getNow();
    if (targetPid[0] != '\0') { // pid处理
        wsprintf(msg, L"[%s]: 开始监控进程(pid=%s)", now, targetPid);
        insertList(hwndList3, msg);
        std::map<DWORD, std::string> m = getAllProcess();
        bool isFind = false;
        for (auto i : m) {
            if (std::to_string(i.first) == wchar2string(targetPid)) {
                isFind = true;
                startDisplayProcessOpenFilesByPid(i.first);

                delete now;
                return;
            }
        }
        if (isFind == false) {
            wsprintf(msg, L"[%s]: ERROR!目标进程未在运行中!", now);
            insertList(hwndList3, msg);
            delete now;
            return;
        }
    }
    else { // 进程名处理
        wsprintf(msg, L"[%s]: 开始监控进程(进程名=%s)", now, targetProcess);
        insertList(hwndList3, msg);
        startDisplayProcessOpenFilesByName(wchar2string(targetProcess));
    }

    delete now;
}

void btn6_callback() {
    WCHAR msg[MAX_PATH];
    WCHAR* now = getNow();
    stopDisplayProcessOpenFilesByName();
    stopDisplayProcessOpenFilesByPid();
    wsprintf(msg, L"[%s]: 停止监控进程", now);
    insertList(hwndList3, msg);

    delete now;
}

void startDisplayAllProcess() {
    std::thread t(displayAllProcess);
    t.detach();
}

void displayAllProcess() {
    std::map<DWORD, std::string> m = getAllProcess();
    WCHAR target[1000] = L"";
    WCHAR row[1000] = L"";
    WCHAR log[1000] = L"";
    WCHAR format[1000] = L"pid: %d    =>    process: %s";
    int rowCount;

    // 初始化
    for (auto i : m) {
        wsprintf(row, format, i.first, string2wchar(i.second));
        insertList(hwndList2, row);
    }

    while (1) {
        //Sleep(0);
        m = getAllProcess();
        rowCount = SendMessage(hwndList2, LB_GETCOUNT, 0, 0);
        // 判断m中哪些不在list中
        for (auto i : m) {

            wsprintf(target, format, i.first, string2wchar(i.second));
            std::string tmp_i = wchar2char(target);
            bool isFind = false;
            for (int j = 0; j < rowCount; ++j) {
                SendMessage(hwndList2, LB_GETTEXT, j, (LPARAM)row); // 获取列表中指定的条目
                std::string tmp_j = wchar2char(row);
                if (tmp_i == tmp_j) { // 找到了就不插入
                    isFind = true;
                    break;
                }
            }
            if (isFind == false) { // 如果没找到就插入
                insertList(hwndList2, target);
                // 记录日志 
                wsprintf(log, L"pid: %d    =>    process: %s 启动", i.first, string2wchar(i.second));
                insertList(hwndList3, log);
            }
        }
        // 判断list哪些不在m中
        for (int i = 0; i < rowCount; ++i) {
            SendMessage(hwndList2, LB_GETTEXT, i, (LPARAM)row);
            std::string tmp_i = wchar2char(row);
            bool isFind = false;
            for (auto j : m) {
                wsprintf(target, format, j.first, string2wchar(j.second));
                std::string tmp_j = wchar2char(target);
                if (tmp_i == tmp_j) { // 找到了就不删除
                    isFind = true;
                    break;
                }
            }
            if (isFind == false) { // 如果没找到就删除列表中的某个行
                SendMessage(hwndList2, LB_DELETESTRING, i, 0); // 删除列表中指定的条目
                // 别忘了删了之后要rowCount--
                rowCount--;
                // 记录日志
                wsprintf(log, L"%s 退出", row);
                insertList(hwndList3, log);
            }
        }
    }
}

void startDisplayProcessOpenFilesByPid(int pid) {
    flag2.clear();
    std::thread t(displayProcessOpenFilesByPid, pid);
    t.detach();
}

void displayProcessOpenFilesByPid(int pid) {
    std::vector<std::string> fileList = getProcessAllOpenFileByPid(pid);
    WCHAR msg[MAX_PATH] = L"";
    WCHAR* now = getNow();
    if (fileList.size() == 0) {
        wsprintf(msg, L"目标进程暂未打开文件或者你无权访问目标进程");
        insertList(hwndList3, msg);
    }
    for (auto i : fileList) {
        auto _i = string2wchar(translateDeviceNameToDiskFilename(i));
        wsprintf(msg, L"打开文件%s", _i);
        insertList(hwndList3, msg);

        delete _i;
    }
    while (1) {
        Sleep(1000);
        std::vector<std::string> newFileList = getProcessAllOpenFileByPid(pid);
        if (flag2.test_and_set()) {
            return;
        }
        flag2.clear();
        for (auto i : fileList) {
            auto idx = find(newFileList.begin(), newFileList.end(), i);
            if (idx == newFileList.end()) { // not find
                auto _i = string2wchar(translateDeviceNameToDiskFilename(i));
                wsprintf(msg, L"关闭文件%s", _i);
                insertList(hwndList3, msg);

                delete _i;
            }
        }
        for (auto i : newFileList) {
            auto idx = find(fileList.begin(), fileList.end(), i);
            if (idx == fileList.end()) { // not find
                auto _i = string2wchar(translateDeviceNameToDiskFilename(i));
                wsprintf(msg, L"打开文件%s", _i);
                insertList(hwndList3, msg);

                delete _i;
            }
        }
        fileList.assign(newFileList.begin(), newFileList.end());
    }

    delete now;
}

void startDisplayProcessOpenFilesByName(std::string name) {
    flag3.clear();
    std::thread t(displayProcessOpenFilesByName, name);
    t.detach();
}

void displayProcessOpenFilesByName(std::string name) {
    WCHAR msg[MAX_PATH] = L"";
    bool isStart = false;
    int round = 0;
    while (!isStart) {
        std::map<DWORD, std::string> m = getAllProcess();
        if (flag3.test_and_set()) {
            return;
        }
        flag3.clear();
        for (auto i : m) {
            if (i.second == name) {
                wsprintf(msg, L"目标进程已启动，监视中...");
                insertList(hwndList3, msg);
                isStart = true;
                break;
            }
        }
        if (!isStart && !((round++) % 50)) { // 200 * 50 = 10秒输出一次
            WCHAR* now = getNow();
            wsprintf(msg, L"[%s]: 目标进程暂未启动，等待其启动中...", now);
            insertList(hwndList3, msg);
        }
        Sleep(200);
    }
    std::vector<std::string> fileList = getProcessAllOpenFileByName(name);
    if (fileList.size() == 0) {
        wsprintf(msg, L"目标进程暂未打开文件或者你无权访问目标进程");
        insertList(hwndList3, msg);
    }
    for (auto i : fileList) {
        auto _i = string2wchar(translateDeviceNameToDiskFilename(i));
        wsprintf(msg, L"打开文件%s", _i);
        insertList(hwndList3, msg);

        delete _i;
    }
    while (1) {
        Sleep(200);
        std::vector<std::string> newFileList = getProcessAllOpenFileByName(name);
        if (flag3.test_and_set()) {
            return;
        }
        flag3.clear();
        for (auto i : fileList) {
            auto idx = find(newFileList.begin(), newFileList.end(), i);
            if (idx == newFileList.end()) { // not find
                auto _i = string2wchar(translateDeviceNameToDiskFilename(i));
                wsprintf(msg, L"关闭文件%s", _i);
                insertList(hwndList3, msg);

                delete _i;
            }
        }
        for (auto i : newFileList) {
            auto idx = find(fileList.begin(), fileList.end(), i);
            if (idx == fileList.end()) { // not find
                auto _i = string2wchar(translateDeviceNameToDiskFilename(i));
                wsprintf(msg, L"打开文件%s", _i);
                insertList(hwndList3, msg);

                delete _i;
            }
        }
        fileList.assign(newFileList.begin(), newFileList.end());
    }
}

void stopDisplayProcessOpenFilesByPid() {
    flag2.test_and_set();
}

void stopDisplayProcessOpenFilesByName() {
    flag3.test_and_set();
}
/*
http://www.tkxiong.com/archives/1396
*/
std::string selectDir() {
    BROWSEINFO bifolder;
    LPITEMIDLIST idl;
    WCHAR filename[MAX_PATH];
    //初始化变量
    ZeroMemory(&bifolder, sizeof(BROWSEINFO));
    ZeroMemory(&filename, MAX_PATH);
    bifolder.pszDisplayName = filename;						// 存放目录路径缓冲区  
    bifolder.lpszTitle = TEXT("请选择文件夹");				// 标题  
    bifolder.ulFlags = BIF_NEWDIALOGSTYLE | BIF_EDITBOX;	// 新的样式,带编辑框  

    idl = SHBrowseForFolder(&bifolder);
    SHGetPathFromIDList(idl, filename);

    //弹框提示文件信息
    MessageBox(NULL, filename, TEXT("文件夹名称"), MB_OK);
    return wchar2char(filename);
}



