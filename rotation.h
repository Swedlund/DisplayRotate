#ifndef ROTATION_H
#define ROTATION_H

#include <windows.h>
#include <wingdi.h>

#ifndef DM_DISPLAYORIENTATION
#define DM_DISPLAYORIENTATION 0x00000080
#endif

#ifndef DMDO_DEFAULT
#define DMDO_DEFAULT 0
#endif

#ifndef DMDO_90
#define DMDO_90 1
#endif

#ifndef DMDO_180
#define DMDO_180 2
#endif

#ifndef DMDO_270
#define DMDO_270 3
#endif

void RotateScreen(int orientation);

#endif