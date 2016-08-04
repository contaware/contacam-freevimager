/*
Necessary VS2010 hack to correctly link to ffmpeg which needs hypot:
the problem is that hypot and hypotf are declared inline in math.h,
but fortunately we can turn the declaration off by defining RC_INVOKED.
Note: MATH_H_PATH is defined in uImagervs2010 project settings
*/

#ifndef _INC_MATH_WRAPPER
#define _INC_MATH_WRAPPER

#ifndef RC_INVOKED

#define RC_INVOKED
#include MATH_H_PATH
extern "C" double __cdecl hypot(double, double); // defined in uImager.cpp
extern "C" float __cdecl hypotf(float, float);   // defined in uImager.cpp
#undef RC_INVOKED

#else

#include MATH_H_PATH
extern "C" double __cdecl hypot(double, double); // defined in uImager.cpp
extern "C" float __cdecl hypotf(float, float);   // defined in uImager.cpp

#endif

#endif  /* _INC_MATH_WRAPPER */