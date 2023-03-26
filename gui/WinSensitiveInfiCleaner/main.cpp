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
#include <commctrl.h> 
#include <windows.h>

#define ID_CLEANUP_BTN 1
#define ID_INPUT_BOX 2
#define ID_OUTPUT_BOX 3
#define ID_BROWSE_BTN 4
#define ID_TOOLBAR 5

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
void RunSensitiveInfoCleaner();

HWND inputBox, outputBox;
HWND hToolbar;

WNDPROC pfnOrigInputBoxProc;
WNDPROC pfnOrigOutputBoxProc;

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
                  WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 800, 600, nullptr,
                  nullptr, nullptr, nullptr);

    MSG msg = {0};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

LRESULT CALLBACK EditBoxSubclassProc(HWND hWnd, UINT msg, WPARAM wParam,
                                     LPARAM lParam)
{
    if (msg == WM_KEYDOWN && wParam == 'A' &&
        (GetKeyState(VK_CONTROL) & 0x8000)) {
        SendMessage(hWnd, EM_SETSEL, 0, -1);
        return 0;
    }
    if (hWnd == inputBox)
        return CallWindowProc(pfnOrigInputBoxProc, hWnd, msg, wParam, lParam);
    else
        return CallWindowProc(pfnOrigOutputBoxProc, hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_CREATE: {
        // Create the toolbar window
        HWND hToolBar = CreateWindowEx(
            0, TOOLBARCLASSNAME, NULL,
            WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT |
                TBSTYLE_LIST,
            0, 0, 0, 0, hWnd, (HMENU)ID_TOOLBAR, nullptr, nullptr);

        // Send the TB_BUTTONSTRUCTSIZE message, which is required for backward
        // compatibility.
        SendMessage(hToolBar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

        // Create buttons for the toolbar
        TBBUTTON tbButtons[2] = {{0,
                                  ID_CLEANUP_BTN,
                                  TBSTATE_ENABLED,
                                  BTNS_AUTOSIZE,
                                  {0},
                                  0,
                                  (INT_PTR)L"Cleanup"},
                                 {1,
                                  ID_BROWSE_BTN,
                                  TBSTATE_ENABLED,
                                  BTNS_AUTOSIZE,
                                  {0},
                                  0,
                                  (INT_PTR)L"Browse..."}};

        // Add buttons to the toolbar
        SendMessage(hToolBar, TB_ADDBUTTONS,
                    sizeof(tbButtons) / sizeof(TBBUTTON), (LPARAM)&tbButtons);

        // Add a separator after the last button
        TBBUTTON sep = {0, 0, TBSTATE_ENABLED, BTNS_SEP, {0}, 0, 0};
        SendMessage(hToolBar, TB_ADDBUTTONS, 1, (LPARAM)&sep);

        int toolbarHeight = 0;
        RECT toolbarRect;
        if (GetWindowRect(hToolbar, &toolbarRect)) {
            toolbarHeight = toolbarRect.bottom - toolbarRect.top;
        }

        inputBox = CreateWindowW(L"edit", L"",
                                 WS_VISIBLE | WS_CHILD | WS_BORDER |
                                     ES_MULTILINE | WS_VSCROLL,
                                 0, toolbarHeight, 800, 400, hWnd,
                                 (HMENU)ID_INPUT_BOX, nullptr, nullptr);

        outputBox = CreateWindowW(L"edit", L"",
                                  WS_VISIBLE | WS_CHILD | WS_BORDER |
                                      ES_MULTILINE | WS_VSCROLL,
                                  900, toolbarHeight, 800, 400, hWnd,
                                  (HMENU)ID_OUTPUT_BOX, nullptr, nullptr);

        HFONT hFont = CreateFont(12, 0, 0, 0, FW_NORMAL, 0, 0, 0,
                                 DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                 CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                 DEFAULT_PITCH | FF_SWISS, L"SimSun");

        SendMessage(inputBox, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(outputBox, WM_SETFONT, (WPARAM)hFont, TRUE);

        pfnOrigInputBoxProc = (WNDPROC)SetWindowLongPtr(
            inputBox, GWLP_WNDPROC, (LONG_PTR)EditBoxSubclassProc);
        pfnOrigOutputBoxProc = (WNDPROC)SetWindowLongPtr(
            outputBox, GWLP_WNDPROC, (LONG_PTR)EditBoxSubclassProc);
    } break;

    case WM_COMMAND: {
        if (LOWORD(wp) == ID_CLEANUP_BTN) {
            RunSensitiveInfoCleaner();
        } else if (LOWORD(wp) == ID_BROWSE_BTN) {
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

    case WM_SIZE: {
        int nWidth = LOWORD(lp);
        int nHeight = HIWORD(lp);

        int nInputBoxWidth = static_cast<int>(nWidth * 0.5);
        int nOutputBoxWidth = static_cast<int>(nWidth * 0.5);
        int nMiddleWidth = nWidth - nInputBoxWidth - nOutputBoxWidth;

        int nButtonHeight = 25;
        int nTopButtonY = (nHeight - 2 * nButtonHeight) / 2;

        int toolbarHeight = 0;
        RECT toolbarRect;
        if (GetWindowRect(hToolbar, &toolbarRect)) {
            toolbarHeight = toolbarRect.bottom - toolbarRect.top;
        }

        HWND hToolBar = GetDlgItem(hWnd, ID_TOOLBAR);
        RECT rcToolBar;
        GetWindowRect(hToolBar, &rcToolBar);
        int nToolBarHeight = rcToolBar.bottom - rcToolBar.top;

        // Move and resize inputBox and outputBox
        MoveWindow(inputBox, 0, nToolBarHeight, nInputBoxWidth,
                   nHeight - nToolBarHeight, TRUE);
        MoveWindow(outputBox, nInputBoxWidth + nMiddleWidth, nToolBarHeight,
                   nOutputBoxWidth, nHeight - nToolBarHeight, TRUE);
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
