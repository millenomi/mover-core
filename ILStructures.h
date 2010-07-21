
#ifndef ILStructures_H
#define ILStructures_H 1

class ILString;

#define __STDC_LIMIT_MACROS 1
#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

typedef size_t ILIndex;
typedef size_t ILSize;

#define ILIndexFormat "zu"
#define ILSizeFormat  "zu"

//#ifdef SIZE_MAX
//	
//#else
//
//	// TODO see if other 64-bit systems/desirable compilers use other preprocessor macros.
//	#if __LP64__ || __SIZEOF_POINTER__ == 8
//		#define ILSizeMax 18446744073709551615ULL
//	#else
//		#define ILSizeMax 4294967295UL
//	#endif
//
//#endif

#define ILSizeMax SIZE_MAX
#define ILIndexMax ILSizeMax

static const ILIndex ILNotFound = ILIndexMax;

typedef struct _ILRange {
	ILIndex start;
	ILSize length;
} ILRange;

extern ILString* ILDescribeRange(ILRange r);
extern void ILLog(ILString* x);


static inline ILRange ILMakeRange(ILIndex start, ILSize length) {
	ILRange r;
	r.start = start;
	r.length = length;
	return r;
}

#if __GNUC__ >= 4
#define ILAttributeRequiresNULLSentinel __attribute__((sentinel))
#else
#define ILAttributeRequiresNULLSentinel
#endif

#endif // #ifndef ILStructures_H
