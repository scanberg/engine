/* Copyright (c) <2009> <Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely
*/


#ifndef __dMathDefined__
#define __dMathDefined__

#include <math.h>
#include <float.h>


#ifndef dFloat
	#ifdef __USE_DOUBLE_PRECISION__
		typedef double dFloat;
	#else 
		typedef float dFloat;
	#endif
#endif 

#ifndef dFloat64
	typedef double dFloat64;
#endif 


// transcendental functions
#define	dAbs(x)		dFloat (fabs (dFloat(x))) 
#define	dSqrt(x)	dFloat (sqrt (dFloat(x))) 
#define	dFloor(x)	dFloat (floor (dFloat(x))) 
#define	dMod(x,y)	dFloat (fmod (dFloat(x), dFloat(y))) 

#define dSin(x)		dFloat (sin (dFloat(x)))
#define dCos(x)		dFloat (cos (dFloat(x)))
#define dAsin(x)	dFloat (asin (dFloat(x)))
#define dAcos(x)	dFloat (acos (dFloat(x)))
#define	dAtan2(x,y) dFloat (atan2 (dFloat(x), dFloat(y)))

#if ( !defined (_MSC_VER) && !defined (_MINGW_32_VER) && !defined (_MINGW_64_VER) )
	#define _ASSERTE(x)
#endif

#ifndef min
    #define min(a,b) ((a < b) ? (a) : (b))
    #define max(a,b) ((a > b) ? (a) : (b))
#endif

#ifdef _MSC_VER
	#ifdef _DEBUG
		void dExpandTraceMessage (const char *fmt, ...);
		#define dTrace(x)										\
		{														\
			dExpandTraceMessage x;								\
		}																	
	#else
		#define dTrace(x)
	#endif
#endif


#endif

