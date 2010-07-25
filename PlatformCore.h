
#ifndef PlatformCore_H
#define PlatformCore_H 1

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <errno.h>

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

#include "ILThreading.h"

#define ILConcatenateAfterExpanding(a, b) \
	a ## b
#define ILConcatenate(a, b) ILConcatenateAfterExpanding(a, b)

#define ILUniqueConstant(name) \
	ILUniqueConstantWithTargetName(name, ILConcatenate(ILUniqueConstantTargetUnused__, __LINE__))

#define ILUniqueStaticConstant(name) \
	ILUniqueStaticConstantWithTargetName(name, ILConcatenate(ILUniqueConstantTargetUnused__, __LINE__))

#define ILUniqueConstantWithTargetName(name, targetName) \
	static char targetName = 0; \
	void* name = &targetName

#define ILUniqueStaticConstantWithTargetName(name, targetName) \
	static char targetName = 0; \
	static void* name = &targetName


#define ILAs(class, value) \
    static_cast<class*>(value)

static inline void ILAbortWithLocationInformation(const char* whereInfo, const char* reason) {
    fprintf(stderr, "<Aborting in %s>: %s\n", whereInfo, reason);
    abort();
}

#define ILAbort(reason) ILAbortWithLocationInformation(__func__, (reason));


#define ILAssertAtCompileTime(x) \
	switch (0) { case 0: case (x): ; }

#if DEBUG
#define ILInline
#else
#define ILInline inline
#endif

template <typename T>
static ILInline T ILMin(T a, T b) {
	return a < b? a : b;
}

template <typename T>
static ILInline T ILMax(T a, T b) {
	return a > b? a : b;
}

template <typename T>
static ILInline void ILClear(T*& x) {
	ILRelease(static_cast<ILObject*>(x)); x = NULL;
}

#define ILRetainGetterSetterImpl(className, type, var, getter, setter) \
	type className::getter() { \
		return (var); \
	} \
	void className::setter(type newValue) { \
		if (newValue != (var)) { \
			ILRelease(var); \
			var = ILRetain(newValue); \
		} \
	}

#define ILCopyGetterSetterImpl(className, type, var, getter, setter) \
	type className::getter() { \
		return (var); \
	} \
	void className::setter(type newValue) { \
		if (newValue != (var)) { \
			ILRelease(var); \
			var = ILRetain(newValue->copy()); \
		} \
	}

#define ILAssignGetterSetterImpl(className, type, var, getter, setter) \
	type className::getter() { \
		return (var); \
	} \
	void className::setter(type newValue) { \
		var = newValue; \
	}

#define ILProperty(propertyType, getter, setter) \
	propertyType getter(); \
	void setter(propertyType newValue)

#define ILPropertyWithModifiers(mods, propertyType, getter, setter) \
	mods propertyType getter(); \
	mods void setter(propertyType newValue)

#include "ILStructures.h"

#endif // #ifdef __cplusplus

#endif // #ifndef PlatformCore_H
