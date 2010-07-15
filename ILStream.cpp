/*
 *  ILStream.cpp
 *  MoverCore
 *
 *  Created by ∞ on 14/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#error Redo invalidate(), close(), make sure locking is sane.

#include "ILStream.h"

#include <cstdio>
#include <sys/select.h>
#include <ctime>

void ILStreamMonitor(ILObject* o) {
	ILStream* s = ILAs(ILStream, o);
	
	int fd = s->fileDescriptor();
	if (fd == 0)
		return;
	
	while (s->isValid()) {
		struct timeval tv;
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		
		fd_set readSet, writeSet, errorSet;
		FD_ZERO(&readSet); FD_ZERO(&writeSet); FD_ZERO(&errorSet);
		FD_SET(fd, &readSet); FD_SET(fd, &writeSet);  FD_SET(fd, &errorSet);

		int result = select(fd + 1, &readSet, &writeSet, &errorSet, &tv);
		
		if (result >= 0) {
			if (FD_ISSET(fd, errorSet)) {
				s->invalidate();
				break;
			} else {
				s->signalReadyForReadingAndWriting(FD_ISSET(fd, &readSet), FD_ISSET(fd, &writeSet));
			}
		} else if (errno == EAGAIN) {
			sleep(1);
			continue;
		} else if (errno == EBADF) {
			break;
		} else
			ILAbort("Stream monitoring got an error in select().");
	}
}

ILStream::ILStream(int fileDescriptor, bool closeFileDescriptionOnClose) : ILSource() {
	_closeFileDescriptorOnClose = closeFileDescriptionOnClose;
	_fileDescriptor = fileDescriptor;
	_valid = true;
	
	_canRead = _canWrite = false;
	
	pthread_mutexattr_t attrs;
	pthread_mutexattr_init(&attrs);
	
	pthread_mutexattr_settype(&attrs, PTHREAD_MUTEX_RECURSIVE);
	int result = pthread_mutex_init(&_mutex, &attrs);
	if (result != 0)
		ILAbort("Couldn't make the mutex for the ILStream monitoring thread.");
	
	ILThreadStart(&ILStreamMonitor, this);
}

ILStream::~ILStream() {

}

bool ILStream::isReadyForWriting() { 
	bool b;
	
	pthread_mutex_lock(&_mutex);
	b = _canWrite && _valid;
	pthread_mutex_unlock(&_mutex);
	
	return b;
}

bool ILStream::isReadyForReading() { 
	bool b;
	
	pthread_mutex_lock(&_mutex);
	b = _canRead && _valid;
	pthread_mutex_unlock(&_mutex);
	
	return b;
}

bool ILStream::write(ILData* data, ILSize* writtenSize) {
	if (!this->isValid())
		return 0;
	
	ssize_t s = ::write(_fileDescriptor, data->bytes(), data->length());
	if (s >= 0 && writtenSize)
		*writtenSize = s;
	
	return (s >= 0);
}

ILData* ILStream::readDataOfMaximumLength(ILSize s) {
	if (!this->isValid())
		return NULL;
	
	uint8_t* buffer = (uint8_t*) malloc(s);
	ssize_t read = ::read(_fileDescriptor, buffer, s);
	
	if (read < 0) {
		free(buffer);
		return NULL;
	} else {
		realloc(buffer, s);
		return new ILData(buffer, read, kILDataTakeOwnershipOfMallocBuffer);
	}
}

void ILStream::close() {
	if (!_valid)
		return;
	
	if (_closeFileDescriptorOnClose && this->isValid()) {
		int fd = this->fileDescriptor();
		this->invalidate();
		::close(fd);
	}
}

int ILStream::fileDescriptor() {
	int i;
	
	pthread_mutex_lock(&_mutex);
	i = _fileDescriptor;
	pthread_mutex_unlock(&_mutex);
	
	return i;
}

bool ILStream::isValid() {
	bool b;
	
	pthread_mutex_lock(&_mutex);
	b = _valid;
	pthread_mutex_unlock(&_mutex);
	
	return b;
}

void ILStream::invalidate() {
	pthread_mutex_lock(&_mutex);
	_valid = false;
	_fileDescriptor = 0;
	
	if (this->runLoop())
		this->runLoop()->signalReady();
	
	pthread_mutex_unlock(&_mutex);
}

void ILStream::signalReadyForReadingAndWriting(bool reading, bool writing) {
	pthread_mutex_lock(&_mutex);
	_canRead = reading;
	_canWrite = writing;
	
	if (this->runLoop())
		this->runLoop()->signalReady();
	
	pthread_mutex_unlock(&_mutex);
}

void ILStream::spin() {
	if (!this->runLoop())
		return;
	
	pthread_mutex_lock(&_mutex);
	
	if (!_valid)
		this->runLoop()->currentMessageHub()->deliverMessage(new ILMessage(kILStreamDidCloseWithErrorMessage, this, NULL));
	else {
		if (_canRead)
			this->runLoop()->currentMessageHub()->deliverMessage(new ILMessage(kILStreamNowReadyForReadingMessage, this, NULL));

		if (_canWrite)
			this->runLoop()->currentMessageHub()->deliverMessage(new ILMessage(kILStreamNowReadyForWritingMessage, this, NULL));
	}
	
	pthread_mutex_unlock(&_mutex);
}
