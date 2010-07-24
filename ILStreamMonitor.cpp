/*
 *  ILStreamMonitor.cpp
 *  MoverCore
 *
 *  Created by ∞ on 24/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ILStreamMonitor.h"

#include "ILMessageHub.h"

ILUniqueConstant(kILStreamReadyForReadingMessage);
ILUniqueConstant(kILStreamReadyForWritingMessage);
ILUniqueConstant(kILStreamErrorOccurredMessage);


struct ILStreamMonitorImpl {
	ILStream* stream;	
	ILTarget* readTarget, * writeTarget, * errorTarget;
	ILMutex* mutex;
	
	// Protected by the mutex beyond this point.
	
	bool shouldTerminate;
	
	// each thread we spawn attached to this stream monitor is given a thread ID. If the thread determines the monitor is being served by another thread (because the monitor's ID is not the same as its own), the thread terminates without issuing messages.
	// thread IDs roll over, which is only a problem in an implementation that can run 2^64-1 threads at the same time (aka: no current one).
	uint64_t currentThreadID;
	
	void init(ILStream* s) {
		stream = ILRetain(s);
		
		readTarget = NULL;
		writeTarget = NULL;
		errorTarget = NULL;
		
		mutex = ILRetain(new ILMutex());
		
		shouldTerminate = false;
		
		currentThreadID = 0;
	}
	
	// called by the thread to issue messages.
	// if true, events have been delivered. if false, the events were not delivered and the thread should terminate.
	bool signalEvents(uint64_t threadID, bool read, bool write, bool error);
};

ILStreamMonitor::ILStreamMonitor(ILStream* s) : ILSource() {
	_i = new ILStreamMonitorImpl;
	_i->init(s);
}

ILStreamMonitor::ILStreamMonitor(ILStream* s, ILTarget* readTarget, ILTarget* writeTarget, ILTarget* errorTarget) : ILSource() {
	_i = new ILStreamMonitorImpl;
	_i->init(s);

	_i->readTarget = ILRetain(readTarget);
	_i->writeTarget = ILRetain(writeTarget);
	_i->errorTarget = ILRetain(errorTarget);
	
	if (readTarget)
		ILMessageHub::currentHub()->addTarget(readTarget, kILStreamReadyForReadingMessage, this);
	if (writeTarget)
		ILMessageHub::currentHub()->addTarget(writeTarget, kILStreamReadyForWritingMessage, this);
	if (errorTarget)
		ILMessageHub::currentHub()->addTarget(errorTarget, kILStreamErrorOccurredMessage, this);
		
	beginObserving();
}

ILStreamMonitor::~ILStreamMonitor() {
	ILRelease(_i->stream);
	
	if (_i->readTarget)
		ILMessageHub::currentHub()->removeTarget(_i->readTarget, kILStreamReadyForReadingMessage);
	if (_i->writeTarget)
		ILMessageHub::currentHub()->removeTarget(_i->writeTarget, kILStreamReadyForWritingMessage);
	if (_i->errorTarget)
		ILMessageHub::currentHub()->removeTarget(_i->errorTarget, kILStreamErrorOccurredMessage);

	ILRelease(_i->readTarget);
	ILRelease(_i->writeTarget);
	ILRelease(_i->errorTarget);
	
	delete _i;
}

#define kILStreamMonitorObjectKey ILStr("ILStreamMonitor")
#define kILStreamMonitorThreadIDKey ILStr("ILThreadID")

static void ILStreamMonitoringThread(ILObject* o);

void ILStreamMonitor::beginObserving() {
	ILAcquiredMutex mutex(_i->mutex);
	
	if (_i->currentThreadID == UINT64_MAX)
		_i->currentThreadID = 0;
	else
		_i->currentThreadID++;
	
	ILMap* m = new ILMap(kILStreamMonitorObjectKey, this,
						 kILStreamMonitorThreadIDKey, new ILNumber(_i->currentThreadID),
						 NULL);
	ILThreadStart(&ILStreamMonitoringThread, m);
}

void ILStreamMonitor::endObserving() {
	ILAcquiredMutex m(_i->mutex);

	// We set the current thread ID to a ID that corresponds to NO EXISTING THREAD (again, unless we can get 2^64-1 threads running at once).
	// This causes all currently running threads to fail on the next check-in, thus causing them to quit.
	
	if (_i->currentThreadID == UINT64_MAX)
		_i->currentThreadID = 0;
	else
		_i->currentThreadID++;
}

static void ILStreamMonitoringThread(ILObject* o) {
#warning TODO actually perform select() loop.
}
