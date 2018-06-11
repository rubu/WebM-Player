#pragma once

#if defined(_WIN32)
#include <Winsock2.h>
#else
#include <CoreFoundation/CoreFoundation.h>
#define ntohf(x) CFConvertFloat32SwappedToHost({(x)})
#define ntohd(x) CFConvertDoubleSwappedToHost({(x)})
#define ntohll CFSwapInt64BigToHost
#endif
