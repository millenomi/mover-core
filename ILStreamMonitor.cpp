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
	ILStreamMonitor* self;
	
	ILStream* stream;	
	ILTarget* readTarget, * writeTarget, * errorTarget;
	ILMutex* mutex;
	
	// Protected by the mutex beyond this point.
	
	// each thread we spawn attached to this stream monitor is given a thread ID. If the thread determines the monitor is being served by another thread (because the monitor's ID is not the same as its own), the thread terminates without issuing messages.
	// thread IDs roll over, which is only a problem in an implementation that can run 2^64-1 threads at the same time (aka: no current one).
	uint64_t currentThreadID;
	
	bool hasPendingReadEvent, hasPendingWriteEvent, hasPendingErrorEvent;
	
	void init(ILStreamMonitor* me, ILStream* s) {
		self = me;
		stream = ILRetain(s);
		
		readTarget = NULL;
		writeTarget = NULL;
		errorTarget = NULL;
		
		mutex = ILRetain(new ILMutex());
				
		hasPendingReadEvent = hasPendingWriteEvent = hasPendingErrorEvent = false;
		
		currentThreadID = 0;
	}
	
	// called by the thread to issue messages.
	// if true, events have been delivered. if false, the events were not delivered and the thread should terminate.
	bool signalEvents(uint64_t threadID, bool read, bool write, bool error);
};

ILStreamMonitor::ILStreamMonitor(ILStream* s) : ILSource() {
	_i = new ILStreamMonitorImpl;
	_i->init(this, s);
}

ILStreamMonitor::ILStreamMonitor(ILStream* s, ILTarget* readTarget, ILTarget* writeTarget, ILTarget* errorTarget) : ILSource() {
	_i = new ILStreamMonitorImpl;
	_i->init(this, s);

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

void ILStreamMonitoringThread(ILObject* o);

void ILStreamMonitor::beginObserving() {
	ILAcquiredMutex mutex(_i->mutex);
	
	ILRunLoop::current()->addSource(this);
	
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
	
	ILRunLoop::current()->removeSource(this);
}

void ILStreamMonitoringThread(ILObject* o) {
	ILMap* arguments = ILAs(ILMap, o);
	ILStreamMonitor* self = arguments->at<ILStreamMonitor>(kILStreamMonitorObjectKey);
	uint64_t threadID = arguments->at<ILNumber>(kILStreamMonitorThreadIDKey)->unsignedIntegerValue();
	
	int fd = self->_i->stream->fileDescriptor();
	if (fd < 0)
		return;
	
	while (true) {
		fd_set readingSet;
		FD_ZERO(&readingSet);
		FD_SET(fd, &readingSet);
		
		fd_set writingSet;
		FD_ZERO(&writingSet);
		FD_SET(fd, &writingSet);
		
		fd_set errorSet;
		FD_ZERO(&errorSet);
		FD_SET(fd, &errorSet);
		
		struct timeval tv;
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		int result = select(fd + 1, &readingSet, &writingSet, &errorSet, &tv);
	
		if (result < 0)
			return;
		
		bool canRead = FD_ISSET(fd, &readingSet),
			canWrite = FD_ISSET(fd, &writingSet),
			hasError = FD_ISSET(fd, &errorSet);
		
		if (!self->_i->signalEvents(threadID, canRead, canWrite, hasError))
			return;
	}	
}

bool ILStreamMonitorImpl::signalEvents(uint64_t threadID, bool read, bool write, bool error) {
	ILAcquiredMutex acquisition(mutex);
	
	if (threadID != currentThreadID)
		return false;
	
	hasPendingReadEvent = hasPendingReadEvent || read;
	hasPendingWriteEvent = hasPendingWriteEvent || write;
	hasPendingErrorEvent = hasPendingErrorEvent ||error;
	
	ILRunLoop* r = self->runLoop();
	if (r)
		r->signalReady();
	
	return true;
}

void ILStreamMonitor::spin() {
	ILAcquiredMutex acquisition(_i->mutex);
	ILRunLoop* r = this->runLoop();
	
	if (_i->hasPendingReadEvent) {
		_i->hasPendingReadEvent = false;
		
		if (r) {
			ILMessage* m = new ILMessage(kILStreamReadyForReadingMessage, this, NULL);
			r->currentMessageHub()->deliverMessage(m);
		}
	}

	if (_i->hasPendingWriteEvent) {
		_i->hasPendingWriteEvent = false;
		
		if (r) {
			ILMessage* m = new ILMessage(kILStreamReadyForWritingMessage, this, NULL);
			r->currentMessageHub()->deliverMessage(m);
		}
	}

	if (_i->hasPendingErrorEvent) {
		_i->hasPendingErrorEvent = false;
		
		if (r) {
			ILMessage* m = new ILMessage(kILStreamErrorOccurredMessage, this, NULL);
			r->currentMessageHub()->deliverMessage(m);
		}
	}
}

ILRunLoop* ILStreamMonitor::runLoop() {
	ILAcquiredMutex m(_i->mutex);
	return ILSource::runLoop();
}

void ILStreamMonitor::setRunLoop(ILRunLoop* r) {
	ILAcquiredMutex m(_i->mutex);
	ILSource::setRunLoop(r);
}
