// WinSensitiveInfiCleaner.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "WinSensitiveInfiCleaner.h"

#include <fstream>
#include <string>
#include <thread>
#include <codecvt>
#include <locale>
#include <windows.h>

#define ID_CLEANUP_BTN 1
#define ID_INPUT_BOX 2
#define ID_OUTPUT_BOX 3

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
void RunSensitiveInfoCleaner();

HWND inputBox, outputBox;

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int ncmdshow)
{
    WNDCLASSW wc = { 0 };

    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hInstance = hInst;
    wc.lpszClassName = L"myWindowClass";
    wc.lpfnWndProc = WindowProcedure;

    if (!RegisterClassW(&wc))
        return -1;

    CreateWindowW(
        L"myWindowClass", L"Sensitive Info Cleaner",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        100, 100, 500, 400,
        nullptr, nullptr, nullptr, nullptr);

    MSG msg = { 0 };
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_CREATE: {
        CreateWindowW(
            L"button", L"Cleanup",
            WS_VISIBLE | WS_CHILD,
            200, 300, 100, 25,
            hWnd, (HMENU)ID_CLEANUP_BTN, nullptr, nullptr);

        inputBox = CreateWindowW(
            L"edit", L"",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | WS_VSCROLL,
            50, 50, 400, 100,
            hWnd, (HMENU)ID_INPUT_BOX, nullptr, nullptr);

        outputBox = CreateWindowW(
            L"edit", L"",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | WS_VSCROLL | ES_READONLY,
            50, 170, 400, 100,
            hWnd, (HMENU)ID_OUTPUT_BOX, nullptr, nullptr);
    } break;

    case WM_COMMAND: {
        if (LOWORD(wp) == ID_CLEANUP_BTN) {
            RunSensitiveInfoCleaner();
        }
    } break;

    case WM_DESTROY: {
        PostQuitMessage(0);
        return 0;
    }
    }
    return DefWindowProcW(hWnd, msg, wp, lp);
}

void RunSensitiveInfoCleaner()
{
    // Save inputBox content to input.txt in UTF-8 format
    int inputLength = GetWindowTextLengthW(inputBox) + 1;
    wchar_t* inputContentW = new wchar_t[inputLength];
    GetWindowTextW(inputBox, inputContentW, inputLength);

    std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
    std::string inputContentUtf8 = utf8_conv.to_bytes(inputContentW);
    delete[] inputContentW;

    std::ofstream inputFile("input.txt", std::ios::binary);
    inputFile << inputContentUtf8;
    inputFile.close();

    // Run SensitiveInfoCleaner.exe
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessA(
            "SensitiveInfoCleaner.exe", nullptr, nullptr, nullptr, FALSE, 0,
            nullptr, nullptr, &si, &pi)) {
        MessageBoxA(nullptr, "Failed to run SensitiveInfoCleaner.exe", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    // Wait for the process to complete
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // Wait for 1 second
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Read output.txt as UTF-8 and display content in outputBox
    std::ifstream outputFile("output.txt", std::ios::binary);
    std::string outputContentUtf8((std::istreambuf_iterator<char>(outputFile)), std::istreambuf_iterator<char>());
    outputFile.close();

    std::wstring outputContentW = utf8_conv.from_bytes(outputContentUtf8);

    SetWindowTextW(outputBox, outputContentW.c_str());
}