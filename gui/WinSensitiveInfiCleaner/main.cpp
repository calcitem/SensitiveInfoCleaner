// WinSensitiveInfiCleaner.cpp : Defines the entry point for the application.
//

#include "WinSensitiveInfiCleaner.h"
#include "framework.h"

#include <codecvt>
#include <fstream>
#include <locale>
#include <shellapi.h>
#include <string>
#include <thread>
#include <windows.h>

#define ID_CLEANUP_BTN 1
#define ID_INPUT_BOX 2
#define ID_OUTPUT_BOX 3
#define ID_OPEN_EXE_FOLDER_BTN 4

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
void RunSensitiveInfoCleaner();

HWND inputBox, outputBox;

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args,
                   int ncmdshow)
{
    WNDCLASSW wc = {0};

    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hInstance = hInst;
    wc.lpszClassName = L"myWindowClass";
    wc.lpfnWndProc = WindowProcedure;

    if (!RegisterClassW(&wc))
        return -1;

    CreateWindowW(L"myWindowClass", L"Sensitive Info Cleaner",
                  WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 1800, 1000,
                  nullptr, nullptr, nullptr, nullptr);

    MSG msg = {0};
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
        CreateWindowW(L"button", L"Cleanup", WS_VISIBLE | WS_CHILD, 500, 500,
                      100, 25, hWnd, (HMENU)ID_CLEANUP_BTN, nullptr, nullptr);

        CreateWindowW(L"button", L"Browse...", WS_VISIBLE | WS_CHILD, 700, 500,
                      100, 25, hWnd, (HMENU)ID_OPEN_EXE_FOLDER_BTN, nullptr,
                      nullptr);

        inputBox = CreateWindowW(
            L"edit", L"",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | WS_VSCROLL, 50,
            50, 800, 400, hWnd, (HMENU)ID_INPUT_BOX, nullptr, nullptr);

        outputBox = CreateWindowW(L"edit", L"",
                                  WS_VISIBLE | WS_CHILD | WS_BORDER |
                                      ES_MULTILINE | WS_VSCROLL | ES_READONLY,
                                  900, 50, 800, 400, hWnd, (HMENU)ID_OUTPUT_BOX,
                                  nullptr, nullptr);

        HFONT hFont = CreateFont(12, 0, 0, 0, FW_NORMAL, 0, 0, 0,
                                 DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                 CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                 DEFAULT_PITCH | FF_SWISS, L"SimSun");

        SendMessage(inputBox, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(outputBox, WM_SETFONT, (WPARAM)hFont, TRUE);

    } break;

    case WM_COMMAND: {
        if (LOWORD(wp) == ID_CLEANUP_BTN) {
            RunSensitiveInfoCleaner();
        } else if (LOWORD(wp) == ID_OPEN_EXE_FOLDER_BTN) {
            WCHAR szExePath[MAX_PATH];
            GetModuleFileNameW(NULL, szExePath, MAX_PATH);
            std::wstring exePath(szExePath);
            std::wstring folderPath = exePath.substr(
                0, exePath.find_last_of(L"\\/"));

            ShellExecuteW(NULL, L"open", folderPath.c_str(), NULL, NULL,
                          SW_SHOW);
        }
    }

    break;

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
    wchar_t *inputContentW = new wchar_t[inputLength];
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

    if (!CreateProcessA("SensitiveInfoCleaner.exe", nullptr, nullptr, nullptr,
                        FALSE, 0, nullptr, nullptr, &si, &pi)) {
        MessageBoxA(nullptr, "Failed to run SensitiveInfoCleaner.exe", "Error",
                    MB_OK | MB_ICONERROR);
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
    std::string outputContentUtf8((std::istreambuf_iterator<char>(outputFile)),
                                  std::istreambuf_iterator<char>());
    outputFile.close();

    std::wstring outputContentW = utf8_conv.from_bytes(outputContentUtf8);

    SetWindowTextW(outputBox, outputContentW.c_str());

    // Get the output content as a wide string
    int outputLength = GetWindowTextLengthW(outputBox) + 1;
    wchar_t *outputContentForClipboardW = new wchar_t[outputLength];
    GetWindowTextW(outputBox, outputContentForClipboardW, outputLength);

    // Open the clipboard
    if (OpenClipboard(NULL)) {
        // Empty the clipboard
        EmptyClipboard();

        // Allocate memory to hold the output content (including the null
        // terminator)
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE,
                                   outputLength * sizeof(wchar_t));
        if (hMem) {
            wchar_t *pMem = (wchar_t *)GlobalLock(hMem);
            memcpy(pMem, outputContentForClipboardW,
                   outputLength * sizeof(wchar_t));
            GlobalUnlock(hMem);

            // Set the clipboard data and close the clipboard
            SetClipboardData(CF_UNICODETEXT, hMem);
        }
        CloseClipboard();
    }

    // Free memory
    delete[] outputContentForClipboardW;
}
