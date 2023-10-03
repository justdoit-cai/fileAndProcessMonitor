#pragma once
#include <Windows.h>
#include <string>
#include <map>
#include <time.h>
#include "resource.h"
#include <algorithm>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void drawLine(HWND hwnd);
void drawLabel(HWND hwnd);
void drawList(HWND hwnd);
std::string selectDir();
void drawButton(HWND hwnd);
void btn1_callback();
void btn2_callback();
void btn3_callback();
void btn4_callback();
void btn5_callback();
void btn6_callback();
void drawEdit(HWND hwnd);
void insertList(HWND hwndList, WCHAR* content);
void startDisplayAllProcess();
void displayAllProcess();
void startDisplayProcessOpenFilesByPid(int pid);
void stopDisplayProcessOpenFilesByPid();
void displayProcessOpenFilesByPid(int pid);
void startDisplayProcessOpenFilesByName(std::string name);
void stopDisplayProcessOpenFilesByName();
void displayProcessOpenFilesByName(std::string name);

#define IDC_LIST1 -1
#define IDC_LIST2 0
#define IDC_LIST3 1
#define ID_BTN1_CALLBACK 3
#define ID_BTN2_CALLBACK 4
#define ID_BTN3_CALLBACK 5
#define ID_BTN4_CALLBACK 6
#define ID_BTN5_CALLBACK 7
#define ID_BTN6_CALLBACK 8
#define ID_EDIT1 9
#define ID_EDIT2 10
#define ID_EDIT3 11
#define ID_EDIT4 12
#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800