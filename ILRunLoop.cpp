/*
 *  ILRunLoop.cpp
 *  Argyle
 *
 *  Created by ∞ on 11/05/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ILRunLoop.h"
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>

#include "ILTelemetry.h"

static char ILRunLoopUnusedValue = 0;
void* const ILRunLoopSignalReadyMessage = &ILRunLoopUnusedValue;

ILTimeInterval ILGetAbsoluteTime() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	
	ILTimeInterval i = (double) tv.tv_sec + (double) tv.tv_usec / 1000000;
	return i;
}

void ILRunLoop::addSource(ILSource* s) {
	_sources->addObject(s);
	s->setRunLoop(this);
}

void ILRunLoop::removeSource(ILSource* s) {
	s->setRunLoop(NULL);
	_sources->removeObject(s);
}

void ILRunLoop::spinForAboutUpTo(ILTimeInterval seconds) {
	ILEvent(kILRunLoopTelemetrySource, ILStr("Spinning for about %f seconds"), (double) seconds);
	
	if (pthread_mutex_lock(&_mutex) != 0) {
		fprintf(stderr, "Error: we couldn't lock a run loop's private mutex -- this should never have happened.\n");
		abort();
	}
	
	ILTimeInterval now = ILGetAbsoluteTime();
	ILTimeInterval budget = seconds;
	
	do {
		if (_sources->count() == 0)
			return;
		
		ILTimeInterval sleepInterval = budget;
		
		ILSetIterator* i = _sources->iterate();
		ILSource* s;
		while ((s = i->nextAs<ILSource>())) {
			ILTimeInterval desiredInterval = s->nextDesiredExecutionTime() - now;
			if (desiredInterval < sleepInterval)
				sleepInterval = desiredInterval;
		}
		
		ILEvent(kILRunLoopTelemetrySource, ILStr("Beginning a spin wait for no more than %f budgeted seconds."), (double) sleepInterval);
		
		long sleepSecs = (long) floor(sleepInterval);
		long sleepNsecs = (long) ((sleepInterval - floor(sleepInterval)) * 1000000000.0);
		
		ILTimeInterval began;
		if (ILTelemetryIsOn())
			began = ILGetAbsoluteTime();
		
		if (sleepSecs > 0 || (sleepSecs == 0 && sleepNsecs > 0)) {
			struct timeval td;
			gettimeofday(&td, NULL);
			
			struct timespec ts;
			ts.tv_sec = td.tv_sec + sleepSecs;
			ts.tv_nsec = sleepNsecs + td.tv_usec * 1000.0;
			
			pthread_cond_timedwait(&_signaler, &_mutex, &ts);
			// we don't check for errors --
			// if error, then something went wrong with the cond, which it shouldn't.
			// if timeout, it's fine -- we spin once, then quit when the while sees our time is up.
			// if the pipe has something to read, well, ok.			
		}
		
		if (ILTelemetryIsOn()) {
			ILTimeInterval ended = ILGetAbsoluteTime();
			ILEvent(kILRunLoopTelemetrySource, ILStr("Wait over, spinning (after a %f seconds wait)"), (double) (ended - began));
		}

		this->spin();
		
		ILTimeInterval newNow = ILGetAbsoluteTime();
		budget -=  (newNow - now);
		now = newNow;
		
	} while (budget >= 0);
	
	ILEvent(kILRunLoopTelemetrySource, ILStr("Spinning got over budget, handing back control to caller."));
	
	if (pthread_mutex_unlock(&_mutex) != 0) {
		fprintf(stderr, "Error: we couldn't unlock a run loop's private mutex -- this should never have happened.\n");
		abort();
	}
	
}

void ILRunLoop::signalReady() {
	ILEvent(kILRunLoopTelemetrySource, ILStr("Signaled ready."));
	pthread_cond_signal(&_signaler);
}

void ILRunLoop::spin() {
	ILSetIterator* eachSource = _sources->copy()->iterate();
	ILObject* sObject;
	ILSource* s;
	
	while ((sObject = eachSource->next())) {
		ILReleasePool pool;
		
		s = ILAs(ILSource, sObject);
		
		if (_sources->containsObject(sObject))
			s->spin();
	}
}

ILRunLoop::ILRunLoop() : ILTarget() {
	_sources = ILRetain(new ILSet());
	_messageHub = NULL;
	_target = NULL;
	
	if (pthread_cond_init(&_signaler, NULL) != 0) {
		fprintf(stderr, "Error: Cannot allocate condition variable for signaling.");
		abort();
	}

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    
	if (pthread_mutex_init(&_mutex, &attr) != 0) {
		fprintf(stderr, "Error: Cannot allocate mutex for signaling.");
		abort();
	}
    
    pthread_mutexattr_destroy(&attr);
}

ILRunLoop::~ILRunLoop() {
	ILRelease(_sources);
	ILRelease(_messageHub);
	ILRelease((ILObject*) _target);
	
	pthread_cond_destroy(&_signaler);
	pthread_mutex_destroy(&_mutex);
}

ILMessageHub* ILRunLoop::currentMessageHub() {
	if (!_messageHub)
		_messageHub = ILRetain(new ILMessageHub());
	
	return _messageHub;
}

ILTarget* ILRunLoop::currentThreadTarget() {
	if (!_target) {
		ILThreadTarget* tt = new ILThreadTarget(this->currentMessageHub());
		this->addSource(tt->source());
		_target = ILRetain(tt);
	}
	
	return _target;
}


void ILRunLoop::deliverMessage(ILMessage* m) {
	currentThreadTarget()->deliverMessage(m);
}

// ~~~

static pthread_key_t ILRunLoopCurrentKey;

__attribute__((constructor)) void ILRunLoopInitializeKey() {
	pthread_key_create(&ILRunLoopCurrentKey, (void (*)(void*)) &ILRelease);
}

ILRunLoop* ILRunLoop::current() {	
	ILRunLoop* r = (ILRunLoop*) pthread_getspecific(ILRunLoopCurrentKey);
	if (!r) {
		r = ILRetain(new ILRunLoop());
		pthread_setspecific(ILRunLoopCurrentKey, (const void*) r);
	}
	
	return r;
}
