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
#include "ILTelemetry.h"

ILUniqueConstant(kILStreamReadyForReadingMessage);
ILUniqueConstant(kILStreamReadyForWritingMessage);
ILUniqueConstant(kILStreamErrorOccurredMessage);

struct ILStreamMonitorImpl {
	ILStreamMonitor* self;
	
	ILStream* stream;	
	ILTarget* readTarget, * writeTarget, * errorTarget, * monitorStreamTarget;
	ILMutex* mutex;
		
	// Protected by the mutex beyond this point.
	
	// each thread we spawn attached to this stream monitor is given a thread ID. If the thread determines the monitor is being served by another thread (because the monitor's ID is not the same as its own), the thread terminates without issuing messages.
	// thread IDs roll over, which is only a problem in an implementation that can run 2^64-1 threads at the same time (aka: no current one).
	uint64_t currentThreadID;
	
	bool hasPendingReadEvent, hasPendingWriteEvent, hasPendingErrorEvent;
	bool hasActiveReadEvent, hasActiveWriteEvent, hasActiveErrorEvent;
	
	void init(ILStreamMonitor* me, ILStream* s) {
		self = me;
		stream = ILRetain(s);
		
		readTarget = NULL;
		writeTarget = NULL;
		errorTarget = NULL;
		monitorStreamTarget = NULL;
		
		mutex = ILRetain(new ILMutex());
				
		hasPendingReadEvent = hasPendingWriteEvent = hasPendingErrorEvent = false;
		hasActiveReadEvent = hasActiveWriteEvent = hasActiveErrorEvent = false;
		
		currentThreadID = 0;
	}
	
	// called by the thread to issue messages.
	// if true, events have been delivered. if false, the events were not delivered and the thread should terminate.
	bool signalEvents(uint64_t threadID, bool read, bool write, bool error);
	
	void beginMonitoringStream();
	void endMonitoringStream();
};

ILTargetClassForMethod(ILStreamMonitor, didMonitorStreamReadOrWrite);


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
    _i->stream->endObservingStream();
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

ILStream* ILStreamMonitor::stream() {
	return _i->stream;
}

#define kILStreamMonitorObjectKey ILStr("ILStreamMonitor")
#define kILStreamMonitorThreadIDKey ILStr("ILThreadID")

void ILStreamMonitoringThread(ILObject* o);

void ILStreamMonitor::beginObserving() {
	ILAcquiredMutex mutex(_i->mutex);
	
	_i->beginMonitoringStream();
	
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

void ILStreamMonitorImpl::beginMonitoringStream() {
	monitorStreamTarget = ILRetain(new ILStreamMonitor_didMonitorStreamReadOrWrite(self));
	ILMessageHub::currentHub()->addTarget(monitorStreamTarget, kILStreamDidReadMessage, stream);
	ILMessageHub::currentHub()->addTarget(monitorStreamTarget, kILStreamDidWriteMessage, stream);
	
	stream->beginObservingStream();
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
	
	_i->endMonitoringStream();
}

void ILStreamMonitorImpl::endMonitoringStream() {
	stream->endObservingStream();

	ILMessageHub::currentHub()->removeTarget(monitorStreamTarget, kILStreamDidReadMessage);
	ILMessageHub::currentHub()->removeTarget(monitorStreamTarget, kILStreamDidWriteMessage);
	
	ILClear(monitorStreamTarget);
}

void ILStreamMonitoringThread(ILObject* o) {
	ILMap* arguments = ILAs(ILMap, o);
	ILStreamMonitor* self = arguments->at<ILStreamMonitor>(kILStreamMonitorObjectKey);
	uint64_t threadID = arguments->at<ILNumber>(kILStreamMonitorThreadIDKey)->unsignedIntegerValue();
	
	ILEventWithSourceObject(self, kILStreamMonitorThreadTelemetrySource, ILStr("Will start monitoring with stream monitor %p, stream %p (thread ID %llu)"), self, self->stream(), (unsigned long long) threadID);
	
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
	
		if (result < 0) {
			ILEventWithSourceObject(self, kILStreamMonitorThreadTelemetrySource, ILStr("select() had an error, interrupting monitoring."));
			self->_i->signalEvents(threadID, false, false, true);
			return;
		}
		
		bool canRead = result > 0 && FD_ISSET(fd, &readingSet),
			canWrite = FD_ISSET(fd, &writingSet),
			hasError = FD_ISSET(fd, &errorSet);
		
		ILCEvent(ILStr("Has spun once for select() with results read = %d, write = %d, error = %d"), canRead, canWrite, hasError);
		
		if (!self->_i->signalEvents(threadID, canRead, canWrite, hasError)) {
			ILEventWithSourceObject(self, kILStreamMonitorThreadTelemetrySource, ILStr("Check-in reports we have to stop monitoring, returning"));
			return;
		} else
			ILEventWithSourceObject(self, kILStreamMonitorThreadTelemetrySource, ILStr("Checked-in successfully, spinning again."));
	}	
}

bool ILStreamMonitorImpl::signalEvents(uint64_t threadID, bool read, bool write, bool error) {
	ILAcquiredMutex acquisition(mutex);
	
	if (threadID != currentThreadID)
		return false;
	
	hasPendingReadEvent = read;
	hasPendingWriteEvent = write;
	hasPendingErrorEvent = error;
	
	ILRunLoop* r = self->runLoop();
	if (r)
		r->signalReady();
	
	return true;
}

void ILStreamMonitor::spin() {
	ILAcquiredMutex acquisition(_i->mutex);
	ILRunLoop* r = this->runLoop();
	
	if (_i->hasPendingReadEvent) {
		if (r && !_i->hasActiveReadEvent) {
			ILEvent(kILStreamMonitorTelemetrySource, ILStr("Will send ready-for-reading message."));
			_i->hasActiveReadEvent = true;
			ILMessage* m = new ILMessage(kILStreamReadyForReadingMessage, this, NULL);
			r->currentMessageHub()->deliverMessage(m);
		}
	} else
		_i->hasActiveReadEvent = false;

	if (_i->hasPendingWriteEvent) {
		if (r && !_i->hasActiveWriteEvent) {
			ILEvent(kILStreamMonitorTelemetrySource, ILStr("Will send ready-for-writing message."));
			_i->hasActiveWriteEvent = true;
			ILMessage* m = new ILMessage(kILStreamReadyForWritingMessage, this, NULL);
			r->currentMessageHub()->deliverMessage(m);
		}
	} else
		_i->hasActiveWriteEvent = false;
	
	if (_i->hasPendingErrorEvent) {
		if (r && !_i->hasActiveErrorEvent) {
			ILEvent(kILStreamMonitorTelemetrySource, ILStr("Will send error-occurred message."));
			_i->hasActiveErrorEvent = true;
			ILMessage* m = new ILMessage(kILStreamErrorOccurredMessage, this, NULL);
			r->currentMessageHub()->deliverMessage(m);
		}
	} else
		_i->hasActiveErrorEvent = false;
}

ILRunLoop* ILStreamMonitor::runLoop() {
	ILAcquiredMutex m(_i->mutex);
	return ILSource::runLoop();
}

void ILStreamMonitor::setRunLoop(ILRunLoop* r) {
	ILAcquiredMutex m(_i->mutex);
	ILSource::setRunLoop(r);
}

ILUniqueConstant(kILStreamMonitorClassIdentity);

void* ILStreamMonitor::classIdentity() {
	return kILStreamMonitorClassIdentity;
}

void ILStreamMonitor::didMonitorStreamReadOrWrite(ILMessage* m) {
	ILAcquiredMutex mutex(_i->mutex);
		
	if (m->kind() == kILStreamDidReadMessage) {
		ILEvent(kILStreamMonitorTelemetrySource, ILStr("Stream has been read from, will produce messages for further reading availability."));
		_i->hasActiveReadEvent = false;
	} else if (m->kind() == kILStreamDidWriteMessage) {
		ILEvent(kILStreamMonitorTelemetrySource, ILStr("Stream has been written to, will produce messages for further writing availability."));
		_i->hasActiveWriteEvent = false;
	}
}
