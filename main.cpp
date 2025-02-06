#include <windows.h>
#include <iostream>
#include <io.h>
#include <fcntl.h>
#include "rotation.h"
#include "resource.h"

#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001
#define IDC_CHECKBOX_MINIMIZE 1002
#define HOTKEY_ID_UP 1
#define HOTKEY_ID_RIGHT 2
#define HOTKEY_ID_DOWN 3
#define HOTKEY_ID_LEFT 4

NOTIFYICONDATA nid;

void ShowContextMenu(HWND hwnd, POINT pt) {
    HMENU hMenu = CreatePopupMenu();
    if (hMenu) {
        InsertMenu(hMenu, -1, MF_BYPOSITION, ID_TRAY_EXIT, TEXT("Exit"));
        SetForegroundWindow(hwnd);
        TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
        DestroyMenu(hMenu);
    }
}

void SaveCheckboxState(bool checked) {
    HKEY hKey;
    RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\MyApp", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
    DWORD value = checked ? 1 : 0;
    RegSetValueExW(hKey, L"StartMinimized", 0, REG_DWORD, (const BYTE*)&value, sizeof(value));
    RegCloseKey(hKey);
}

bool LoadCheckboxState() {
    HKEY hKey;
    DWORD value = 0;
    DWORD valueSize = sizeof(value);
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\MyApp", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExW(hKey, L"StartMinimized", NULL, NULL, (LPBYTE)&value, &valueSize);
        RegCloseKey(hKey);
    }
    return value != 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HBRUSH hBrush = CreateSolidBrush(RGB(45, 45, 48));
    static HBRUSH hBrushButton = CreateSolidBrush(RGB(30, 30, 30));

    switch (uMsg) {
        case WM_COMMAND:
            if (LOWORD(wParam) == ID_BUTTON_DEFAULT) {
                RotateScreen(0);
            } else if (LOWORD(wParam) == ID_BUTTON_90) {
                RotateScreen(1);
            } else if (LOWORD(wParam) == ID_BUTTON_180) {
                RotateScreen(2);
            } else if (LOWORD(wParam) == ID_BUTTON_270) {
                RotateScreen(3);
            } else if (LOWORD(wParam) == ID_TRAY_EXIT) {
                Shell_NotifyIcon(NIM_DELETE, &nid);
                PostQuitMessage(0);
            } else if (LOWORD(wParam) == IDC_CHECKBOX_MINIMIZE) {
                bool checked = SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED;
                SaveCheckboxState(checked);
            }
            break;
        case WM_HOTKEY:
            switch (wParam) {
                case HOTKEY_ID_UP:
                    RotateScreen(0);
                    break;
                case HOTKEY_ID_RIGHT:
                    RotateScreen(1);
                    break;
                case HOTKEY_ID_DOWN:
                    RotateScreen(2);
                    break;
                case HOTKEY_ID_LEFT:
                    RotateScreen(3);
                    break;
            }
            break;
        case WM_CTLCOLORBTN:
            {
                HDC hdcButton = (HDC)wParam;
                SetTextColor(hdcButton, RGB(255, 255, 255));
                SetBkColor(hdcButton, RGB(30, 30, 30));
                return (INT_PTR)hBrushButton;
            }
        case WM_CTLCOLORSTATIC:
            {
                HDC hdcStatic = (HDC)wParam;
                SetTextColor(hdcStatic, RGB(255, 255, 255));
                SetBkMode(hdcStatic, TRANSPARENT);
                return (INT_PTR)hBrush;
            }
        case WM_DRAWITEM:
            {
                LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;
                if (pDIS->CtlType == ODT_BUTTON) {
                    FillRect(pDIS->hDC, &pDIS->rcItem, hBrushButton);
                    SetTextColor(pDIS->hDC, RGB(255, 255, 255));
                    SetBkMode(pDIS->hDC, TRANSPARENT);

                    LPCWSTR arrow;
                    switch (pDIS->CtlID) {
                        case ID_BUTTON_DEFAULT:
                            arrow = L"\u2191";
                            break;
                        case ID_BUTTON_90:
                            arrow = L"\u2192";
                            break;
                        case ID_BUTTON_180:
                            arrow = L"\u2193";
                            break;
                        case ID_BUTTON_270:
                            arrow = L"\u2190";
                            break;
                        default:
                            arrow = L"";
                            break;
                    }

                    DrawTextW(pDIS->hDC, arrow, -1, &pDIS->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                    return TRUE;
                }
            }
            break;
        case WM_CLOSE:
            ShowWindow(hwnd, SW_HIDE);
            nid.cbSize = sizeof(NOTIFYICONDATA);
            nid.hWnd = hwnd;
            nid.uID = 1;
            nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
            nid.uCallbackMessage = WM_TRAYICON;
            nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APP_ICON));
            strcpy(nid.szTip, "Display Rotate");
            Shell_NotifyIcon(NIM_ADD, &nid);
            return 0;
        case WM_TRAYICON:
            if (lParam == WM_LBUTTONUP) {
                ShowWindow(hwnd, SW_SHOW);
                Shell_NotifyIcon(NIM_DELETE, &nid);
            } else if (lParam == WM_RBUTTONUP) {
                POINT pt;
                GetCursorPos(&pt);
                ShowContextMenu(hwnd, pt);
            }
            break;
        case WM_DESTROY:
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    ShowWindow(FindWindowA("ConsoleWindowClass", NULL), SW_HIDE);

    const wchar_t CLASS_NAME[] = L"ScreenRotationClass";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = CreateSolidBrush(RGB(45, 45, 48));
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));

    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"DRotate",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 130, 235,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    SetWindowTextW(hwnd, L"DRotate");

    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON)));
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON)));

    HWND hCheckbox = CreateWindowW(L"BUTTON", L"Start minimized", 
                                   WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
                                   10, 180, 200, 20, hwnd, (HMENU)IDC_CHECKBOX_MINIMIZE, 
                                   hInstance, NULL);

    HFONT hFontCheckbox = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                                      OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                                      DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

    SendMessage(hCheckbox, WM_SETFONT, (WPARAM)hFontCheckbox, TRUE);

    bool startMinimized = LoadCheckboxState();
    SendMessage(hCheckbox, BM_SETCHECK, startMinimized ? BST_CHECKED : BST_UNCHECKED, 0);

    HFONT hFontHotKeys = CreateFontW(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                                     OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                                     DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

    HWND hHotKeysLabel = CreateWindowW(L"STATIC", L"Hot keys:", WS_VISIBLE | WS_CHILD,
                                       10, 10, 100, 20, hwnd, (HMENU)IDC_STATIC_HOTKEY, hInstance, NULL);
    SendMessage(hHotKeysLabel, WM_SETFONT, (WPARAM)hFontHotKeys, TRUE);

    CreateWindowW(L"STATIC", L"CTRL+ALT+", WS_VISIBLE | WS_CHILD,
                  10, 47, 80, 20, hwnd, (HMENU)IDC_STATIC_HOTKEY, hInstance, NULL);
    CreateWindowW(L"BUTTON", L"\u2191", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
                  90, 40, 30, 30, hwnd, (HMENU)ID_BUTTON_DEFAULT, hInstance, NULL);

    CreateWindowW(L"STATIC", L"CTRL+ALT+", WS_VISIBLE | WS_CHILD,
                  10, 82, 80, 20, hwnd, (HMENU)IDC_STATIC_HOTKEY, hInstance, NULL);
    CreateWindowW(L"BUTTON", L"\u2192", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
                  90, 75, 30, 30, hwnd, (HMENU)ID_BUTTON_90, hInstance, NULL);

    CreateWindowW(L"STATIC", L"CTRL+ALT+", WS_VISIBLE | WS_CHILD,
                  10, 117, 80, 20, hwnd, (HMENU)IDC_STATIC_HOTKEY, hInstance, NULL);
    CreateWindowW(L"BUTTON", L"\u2193", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
                  90, 110, 30, 30, hwnd, (HMENU)ID_BUTTON_180, hInstance, NULL);

    CreateWindowW(L"STATIC", L"CTRL+ALT+", WS_VISIBLE | WS_CHILD,
                  10, 152, 80, 20, hwnd, (HMENU)IDC_STATIC_HOTKEY, hInstance, NULL);
    CreateWindowW(L"BUTTON", L"\u2190", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
                  90, 145, 30, 30, hwnd, (HMENU)ID_BUTTON_270, hInstance, NULL);

    if (startMinimized) {
        ShowWindow(hwnd, SW_HIDE);
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hwnd;
        nid.uID = 1;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = WM_TRAYICON;
        nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APP_ICON));
        strcpy(nid.szTip, "Display Rotate");
        Shell_NotifyIcon(NIM_ADD, &nid);
    } else {
        ShowWindow(hwnd, nCmdShow);
    }

    // Register the hotkeys
    RegisterHotKey(hwnd, HOTKEY_ID_UP, MOD_CONTROL | MOD_ALT, VK_UP);
    RegisterHotKey(hwnd, HOTKEY_ID_RIGHT, MOD_CONTROL | MOD_ALT, VK_RIGHT);
    RegisterHotKey(hwnd, HOTKEY_ID_DOWN, MOD_CONTROL | MOD_ALT, VK_DOWN);
    RegisterHotKey(hwnd, HOTKEY_ID_LEFT, MOD_CONTROL | MOD_ALT, VK_LEFT);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Unregister the hotkeys
    UnregisterHotKey(hwnd, HOTKEY_ID_UP);
    UnregisterHotKey(hwnd, HOTKEY_ID_RIGHT);
    UnregisterHotKey(hwnd, HOTKEY_ID_DOWN);
    UnregisterHotKey(hwnd, HOTKEY_ID_LEFT);

    return 0;
}