//
//  ILStream.cpp
//  MoverCore
//
//  Created by ∞ on 23/07/10.
//  Copyright (c) 2010 __MyCompanyName__. All rights reserved.
//

#include "ILStream.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>

#include <fcntl.h>

#include <sys/select.h>

#include "ILRunLoop.h"

ILStream::ILStream(int fd) : ILObject() {
	_fd = fd;
	_mutex = ILRetain(new ILMutex());
    _countOfObservingClients = 0;
}

ILStream::~ILStream() {
	close();
	ILRelease(_mutex);
}

ILUniqueConstant(kILStreamDidReadMessage);
ILUniqueConstant(kILStreamDidWriteMessage);

ILData* ILStream::read(ILSize maximumBytes, ILStreamError* e) {
	int fd = fileDescriptor();
	if (fd < 0) {
		if (e) *e = kILStreamErrorIsClosed;
		return NULL;
	}
	
    if (_countOfObservingClients > 0)
        ILRunLoop::current()->currentThreadTarget()->deliverMessage(new ILMessage(kILStreamDidReadMessage, this, NULL));
    
	uint8_t* bytes = (uint8_t*) malloc(maximumBytes);
	int result = ::read(fd, bytes, maximumBytes);
	
	if (result > 0) {
		realloc(bytes, result);
		return new ILData(bytes, (ILSize) result, kILDataTakeOwnershipOfMallocBuffer);
	} else if (result == 0) {
		if (e) *e = kILStreamErrorEndOfFile;
	} else {
		if ((errno == EAGAIN || errno == EWOULDBLOCK) && e)
			*e = kILStreamErrorWouldHaveBlocked;
		else if (e)
			*e = kILStreamErrorPOSIX;
	}
    
	return NULL;
}

bool ILStream::write(ILData* data, ILSize* writtenBytes, ILStreamError* e) {
	int fd = fileDescriptor();
	if (fd < 0) {
		if (e) *e = kILStreamErrorIsClosed;
		return false;
	}	
	
    if (_countOfObservingClients > 0)
        ILRunLoop::current()->currentThreadTarget()->deliverMessage(new ILMessage(kILStreamDidWriteMessage, this, NULL));
    
	int result = ::write(_fd, data->bytes(), data->length());
	
	if (result >= 0) {
		if (writtenBytes) *writtenBytes = result;
		return true;
	} else {
		if ((errno == EAGAIN || errno == EWOULDBLOCK) && e)
			*e = kILStreamErrorWouldHaveBlocked;
		else if (e)
			*e = kILStreamErrorPOSIX;
	}
	
	return false;
}

bool ILStream::writeAllOf(ILData* data, ILStreamError* e) {
	ILSize written = 0;
	ILStreamError innerError;
	
	do {
		ILReleasePool p;
		
		ILSize newlyWritten;
		bool result = write(new ILData(data->bytes() + written, data->length() - written), &newlyWritten, &innerError);
		
		if (!result && innerError != kILStreamErrorWouldHaveBlocked) {
			if (e) *e = innerError;
			return false;
		}
		
		if (result)
			written += newlyWritten;
	} while (written < data->length());
	
	return true;
}

ILData* ILStream::readBytesUpTo(ILSize toRead, bool endOfFileIsError, ILStreamError* e) {
	ILData* d = new ILData();
	
	do {
		ILReleasePool p;
		
		ILStreamError innerError;
		ILData* part = read(toRead, &innerError);
		
		if (part) {
			d->appendData(part);
			toRead -= part->length();
		} else {
			if (innerError == kILStreamErrorEndOfFile && !endOfFileIsError)
				break;
			
			if (e) *e = innerError;
			return NULL;
		}
	} while (toRead > 0);
	
	return d;
}

ILData* ILStream::readUntilEnd(ILStreamError* e) {
	ILData* d = new ILData();
	
	ILStreamError innerError;
	do {
		ILReleasePool p;
		
		ILData* part = read(1024 * 512, &innerError);
		
		if (part) {
			innerError = kILStreamErrorNone;
			d->appendData(part);
		} else if (innerError != kILStreamErrorEndOfFile) {
			if (e) *e = innerError;
			return NULL;
		}
	} while (innerError == kILStreamErrorNone);
	
	return d;
}

void ILStream::close() {
	ILAcquiredMutex m(_mutex);
	::close(_fd);
	_fd = -1;
}


// Non-blocking I/O
// NOTE: For now, we REQUIRE the platform to support it. This may be relaxed in a future version of this code.

bool ILStream::allowsBlocking() {
	int fd = fileDescriptor();
	if (fd < 0)
		return false;
	
	int flags = fcntl(fd, F_GETFD);
	if (flags < 0)
		return false;
	
	return (flags & O_NONBLOCK) != 0;
}

bool ILStream::setAllowsBlocking(bool x) {
	int fd = fileDescriptor();
	if (fd < 0)
		return false;
	
	int flags = fcntl(fd, F_GETFD);
	if (flags < 0)
		return false;
	
	if (x)
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;
	
	return fcntl(fd, F_SETFD, flags) >= 0;
}

bool ILStream::canDisableBlocking() {
	return true;
}


bool ILStream::canDetermineReadyState() {
	return true;
}

bool ILStream::isReadyForWriting() {
	bool canWrite;
	_determineReadyState(NULL, &canWrite);
	return canWrite;
}

bool ILStream::isReadyForReading() {
	bool canRead;
	_determineReadyState(&canRead, NULL);	
	return canRead;
}

void ILStream::_determineReadyState(bool* reading, bool* writing) {
	int fd = fileDescriptor();
	if (fd < 0) {
		if (reading) *reading = false;
		if (writing) *writing = false;
		return;
	}
	
	fd_set* readingSetPointer = NULL;
	fd_set readingSet;
	if (reading) {
		FD_ZERO(&readingSet);
		FD_SET(fd, &readingSet);
		readingSetPointer = &readingSet;
	}
	
	fd_set* writingSetPointer = NULL;
	fd_set writingSet;
	if (writing) {
		FD_ZERO(&writingSet);
		FD_SET(fd, &writingSet);
		writingSetPointer = &writingSet;
	}
	
	// TODO error conds?
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	int result = select(fd + 1, readingSetPointer, writingSetPointer, NULL, &tv);
	
	if (result < 0) {
		if (reading) *reading = false;
		if (writing) *writing = false;
		return;
	}
	
	if (reading)
		*reading = FD_ISSET(fd, readingSetPointer);
	
	if (writing)
		*writing = FD_ISSET(fd, writingSetPointer);
}

int ILStream::fileDescriptor() {
	ILAcquiredMutex m(_mutex);
	return _fd;
}

void ILStream::beginObservingStream() {
    _countOfObservingClients++;
}

void ILStream::endObservingStream() {
    if (_countOfObservingClients == 0)
        ILAbort("Mismatched call (end... called without a matching begin...).");
    
    _countOfObservingClients--;
}

// ~ ILStreamSource ~

ILStreamSource::ILStreamSource() : ILObject() {}
ILStreamSource::~ILStreamSource() {}

bool ILStreamSource::canOpen() { return true; }
