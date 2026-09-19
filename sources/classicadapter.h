#ifndef INCLUDED_CLASSICADAPTER_H
#define INCLUDED_CLASSICADAPTER_H

#include <windows.h>
#include "callback.h"

extern BOOL ClassicAdapterApply(D2GSCALLBACKABI *callbackAbi);
extern BOOL ClassicAdapterFinalize(void);

#endif
