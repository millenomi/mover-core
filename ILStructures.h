
#ifndef ILStructures_H
#define ILStructures_H 1

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>

class ILString;

typedef size_t ILIndex;
typedef size_t ILSize;

#define ILIndexFormat "zu"
#define ILSizeFormat  "zu"

static const ILIndex ILNotFound = SIZE_MAX;

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

#endif // #ifndef ILStructures_H
