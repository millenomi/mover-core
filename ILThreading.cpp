/*
 *  ILThreading.cpp
 *  MoverCore
 *
 *  Created by ∞ on 15/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ILThreading.h"
#include <pthread.h>

#include "ILMessage.h"

class ILThreadArguments {
public:
	ILThreadMainFunction function;
	ILObject* argument;
};

static void* ILThreadMain(ILThreadArguments* args) {
	ILReleasePool pool;
	
	ILThreadMainFunction f = args->function;
	f(args->argument);
	ILRelease(args->argument);
	
	delete args;
	return NULL;
}

void ILThreadStart(ILThreadMainFunction function, ILObject* object) {
	ILThreadArguments* args = new ILThreadArguments;
	args->function = function;
	args->argument = object;
	
	ILRetain(object);
	pthread_t thread;
	pthread_create(&thread, NULL, (void* (*)(void*)) &ILThreadMain, args);
}

// ~ ILMutex ~

struct ILMutexImpl {
	pthread_mutex_t _mutex;
};

ILMutex::ILMutex() : ILObject() {
	_me = new ILMutexImpl;
	
	pthread_mutexattr_t attrs;
	pthread_mutexattr_init(&attrs);
	
	pthread_mutexattr_settype(&attrs, PTHREAD_MUTEX_RECURSIVE);
	
	pthread_mutex_init(&(_me->_mutex), &attrs);
	
	pthread_mutexattr_destroy(&attrs);
}

ILMutex::~ILMutex() {
	pthread_mutex_destroy(&(_me->_mutex));
	delete _me;
}

void ILMutex::lock() {
	pthread_mutex_lock(&(_me->_mutex));
}

void ILMutex::unlock() {
	pthread_mutex_unlock(&(_me->_mutex));
}

bool ILMutex::tryLock() {
	return pthread_mutex_trylock(&(_me->_mutex)) == 0;
}


ILAcquiredMutex::ILAcquiredMutex(ILMutex* m) {
	_mutex = ILRetain(m);
	_mutex->lock();
}

ILAcquiredMutex::~ILAcquiredMutex() {
	_mutex->unlock();
	ILRelease(_mutex);
}
