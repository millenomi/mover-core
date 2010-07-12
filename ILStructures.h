
#ifndef ILStructures_H
#define ILStructures_H 1

#include <sys/types.h>
#include <stdint.h>

typedef size_t ILIndex;
typedef size_t ILSize;

static const ILIndex ILNotFound = SIZE_MAX;


typedef struct _ILRange {
	ILIndex start;
	ILSize length;
} ILRange;

static inline ILRange ILMakeRange(ILIndex start, ILSize length) {
	ILRange r;
	r.start = start;
	r.length = length;
	return r;
}

static inline ILRange ILMakeRangeBetweenIndices(ILIndex a, ILIndex b) {
	return ILMakeRange( (a < b? a : b), (a > b? a - b : b - a) + 1 );
}


#endif // #ifndef ILStructures_H
