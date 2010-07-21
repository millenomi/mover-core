/*
 *  ILObject.cpp
 *  Argyle
 *
 *  Created by ∞ on 25/04/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ILObject.h"
#include <stdio.h>

#include <pthread.h>
#include <stdlib.h>

#include "ILReleasePool.h"

static pthread_mutex_t ILRetainReleaseMutex;
void ILRetainReleaseMutexInit() __attribute__((constructor));

void ILRetainReleaseMutexInit() {
	pthread_mutexattr_t attrs;
	pthread_mutexattr_init(&attrs);

	pthread_mutexattr_settype(&attrs, PTHREAD_MUTEX_RECURSIVE);
	
	int result = pthread_mutex_init(&ILRetainReleaseMutex, &attrs);
	if (result != 0)
		abort();
}

ILObject::ILObject() {
	_retainCount = 1;
	_isObservingRetainRelease = false;
	_isPotentiallyUninitialized = false;
	ILReleaseLater(this);
}
ILObject::~ILObject() {}

void ILObject::retain() {
	pthread_mutex_lock(&ILRetainReleaseMutex);
	
	if (this->_isPotentiallyUninitialized) {
		fprintf(stderr, " !! Object %p is potentially uninitialized. Make sure its constructor calls ILObject()'s.\n", this);
		abort();
	}
	
	_retainCount++;

	if (this->_isObservingRetainRelease)
		fprintf(stderr, " <> Retain for %p.\n", this);

	pthread_mutex_unlock(&ILRetainReleaseMutex);
}

void _ILObservedObjectWillDestroy(ILObject* o) {
	fprintf(stderr, " <> Observed object %p is about to be destroyed. Break on %s to debug.\n", o, __func__);
}

bool ILObject::release() {
	bool result;
	
	pthread_mutex_lock(&ILRetainReleaseMutex);

	if (this->_isPotentiallyUninitialized) {
		fprintf(stderr, " !! Object %p is potentially uninitialized. Make sure its constructor calls ILObject()'s.\n", this);
		abort();
	}
	
	if (_retainCount > 1) {
		_retainCount--;
		result = false;
	} else if (_retainCount == 1) {
		_retainCount = 0;
		result = true;
	} else {
		fprintf(stderr, " !! Invalid release count for object %p.\n", this);
		abort();
	}
	
	if (this->_isObservingRetainRelease) {
		fprintf(stderr, " <> Release for %p.\n", this);
		if (result)
			_ILObservedObjectWillDestroy(this);
	}

	pthread_mutex_unlock(&ILRetainReleaseMutex);
	return result;
}

uint64_t ILObject::retainCount() {
	uint64_t rc;
	pthread_mutex_lock(&ILRetainReleaseMutex);
	rc = _retainCount;
	pthread_mutex_unlock(&ILRetainReleaseMutex);
	
	return rc;
}

void ILObject::observeRetainRelease() {
	this->_isObservingRetainRelease = true;
}


void* const ILObjectClassIdentity = (void*) "ILObject";

void* ILObject::classIdentity() {
	return ILObjectClassIdentity;
}


bool ILObject::equals(ILObject* o) {
	return this == o;
}

uint64_t ILObject::hash() {
	return (uint64_t) this;
}

ILObject* ILObject::copy() {
	abort();
	return NULL;
}

ILObject* ILPerformRetain(ILObject* o) {
	if (!o)
		return NULL;
	
	o->retain();
	return o;
}

void ILRelease(ILObject* o) {
	if (!o)
		return;
	
	if (o->release())
		delete o;
}

bool ILObject::canCopy() {
	return false;
}

