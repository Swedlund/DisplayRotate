#include "rotation.h"

void RotateScreen(int orientation) {
    DEVMODE dm;
    memset(&dm, 0, sizeof(dm));
    dm.dmSize = sizeof(dm);

    // Get the current display settings to use the current resolution
    if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm) == 0) {
        DWORD error = GetLastError();
        TCHAR errorMsg[256];
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0, errorMsg, 256, NULL);
        MessageBox(NULL, errorMsg, TEXT("Error"), MB_OK | MB_ICONERROR);
        return;
    }

    int originalWidth = dm.dmPelsWidth;
    int originalHeight = dm.dmPelsHeight;

    dm.dmFields = DM_DISPLAYORIENTATION | DM_PELSWIDTH | DM_PELSHEIGHT;

    if (orientation == 0 || orientation == 2) {
        dm.dmPelsWidth = (dm.dmDisplayOrientation == DMDO_90 || dm.dmDisplayOrientation == DMDO_270) ? originalHeight : originalWidth;
        dm.dmPelsHeight = (dm.dmDisplayOrientation == DMDO_90 || dm.dmDisplayOrientation == DMDO_270) ? originalWidth : originalHeight;
    } else if (orientation == 1 || orientation == 3) {
        dm.dmPelsWidth = (dm.dmDisplayOrientation == DMDO_90 || dm.dmDisplayOrientation == DMDO_270) ? originalWidth : originalHeight;
        dm.dmPelsHeight = (dm.dmDisplayOrientation == DMDO_90 || dm.dmDisplayOrientation == DMDO_270) ? originalHeight : originalWidth;
    }

    if (orientation == 0) {
        dm.dmDisplayOrientation = DMDO_DEFAULT;
    } else if (orientation == 1) {
        dm.dmDisplayOrientation = DMDO_90;
    } else if (orientation == 2) {
        dm.dmDisplayOrientation = DMDO_180;
    } else if (orientation == 3) {
        dm.dmDisplayOrientation = DMDO_270;
    }

    LONG result = ChangeDisplaySettingsEx(NULL, &dm, NULL, CDS_TEST, NULL);
    if (result == DISP_CHANGE_SUCCESSFUL) {
        result = ChangeDisplaySettingsEx(NULL, &dm, NULL, CDS_UPDATEREGISTRY, NULL);
    }

    if (result != DISP_CHANGE_SUCCESSFUL) {
        DWORD error = GetLastError();
        TCHAR errorMsg[256];
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0, errorMsg, 256, NULL);
        MessageBox(NULL, errorMsg, TEXT("Error"), MB_OK | MB_ICONERROR);
    }
}