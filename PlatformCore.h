
#ifndef PlatformCore_H
#define PlatformCore_H 1

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

#ifdef __cplusplus

#include "ILObject.h"
#include "ILReleasePool.h"

#include "ILNumber.h"
#include "ILData.h"
#include "ILLinkedList.h"
#include "ILList.h"
#include "ILMap.h"
#include "ILSet.h"
#include "ILString.h"


#define ILConcatenateAfterExpanding(a, b) \
	a ## b
#define ILConcatenate(a, b) ILConcatenateAfterExpanding(a, b)

#define ILUniqueConstant(name) \
	ILUniqueConstantWithTargetName(name, ILConcatenate(ILUniqueConstantTargetUnused__, __LINE__))

#define ILUniqueConstantWithTargetName(name, targetName) \
	static char targetName = 0; \
	void* name = &targetName


#define ILAs(class, value) \
    ( value? static_cast<class*>(value) : NULL )


#define ILNotFound ((size_t) SIZE_MAX)

static inline void ILAbortWithLocationInformation(const char* whereInfo, const char* reason) {
    fprintf(stderr, "<Aborting in %s>: %s", whereInfo, reason);
    abort();
}

#define ILAbort(reason) ILAbortWithLocationInformation(__func__, (reason));


#define ILAssertAtCompileTime(x) \
	switch (0) { case 0: case (x): ; }

#include "ILStructures.h"

#endif // #ifdef __cplusplus

#endif // #ifndef PlatformCore_H
