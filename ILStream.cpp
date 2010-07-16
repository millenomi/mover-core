/*
 *  ILStream.cpp
 *  MoverCore
 *
 *  Created by ∞ on 14/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */


#include "ILStream.h"

#include <cstdlib>
#include <cerrno>
#include <cstdio>
#include <sys/select.h>
#include <ctime>

ILUniqueConstant(kILStreamDidCloseWithErrorMessage);
ILUniqueConstant(kILStreamNowReadyForReadingMessage);
ILUniqueConstant(kILStreamNowReadyForWritingMessage);

class ILMutexAcquisition {
public:
	pthread_mutex_t* _mutex;
	
	ILMutexAcquisition(pthread_mutex_t* mutex) {
		pthread_mutex_lock(mutex);
		_mutex = mutex;
	}
	
	~ILMutexAcquisition() {
		pthread_mutex_unlock(_mutex);
	}
};

void ILStreamMonitor(ILObject* o) {
	ILStream* s = ILAs(ILStream, o);
	
	int fd;
	while ((fd = s->fileDescriptor()) >= 0) {
		struct timeval tv;
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		
		fd_set readSet, writeSet, errorSet;
		FD_ZERO(&readSet); FD_ZERO(&writeSet); FD_ZERO(&errorSet);
		FD_SET(fd, &readSet); FD_SET(fd, &writeSet);  FD_SET(fd, &errorSet);

		int result = select(fd + 1, &readSet, &writeSet, &errorSet, &tv);
		
		if (result >= 0) {
			if (FD_ISSET(fd, &errorSet)) {
				s->close();
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

ILStream::ILStream(int fileDescriptor) : ILSource() {
	_fileDescriptor = fileDescriptor;
	
	_canRead = _canWrite = false;
	
	pthread_mutexattr_t attrs;
	pthread_mutexattr_init(&attrs);
	
	pthread_mutexattr_settype(&attrs, PTHREAD_MUTEX_RECURSIVE);
	int result = pthread_mutex_init(&this->_mutex, &attrs);
	if (result != 0)
		ILAbort("Couldn't make the mutex for the ILStream monitoring thread.");
	
	ILThreadStart(&ILStreamMonitor, this);
}

ILStream::~ILStream() {
	this->close();
}

bool ILStream::isReadyForWriting() { 
	ILMutexAcquisition(&this->_mutex);
	return _canWrite && _fileDescriptor >= 0;
}

bool ILStream::isReadyForReading() { 
	ILMutexAcquisition(&this->_mutex);
	return _canRead && _fileDescriptor >= 0;
}

bool ILStream::write(ILData* data, ILSize* writtenSize) {
	int fd = this->fileDescriptor();
	if (fd >= 0)
		return false;
	
	ssize_t s = ::write(fd, data->bytes(), data->length());
	if (s >= 0 && writtenSize)
		*writtenSize = s;
	
	return (s >= 0);
}

ILData* ILStream::readDataOfMaximumLength(ILSize s) {
	int fd = this->fileDescriptor();
	if (fd >= 0)
		return false;
	
	uint8_t* buffer = (uint8_t*) malloc(s);
	ssize_t read = ::read(fd, buffer, s);
	
	if (read < 0) {
		free(buffer);
		return NULL;
	} else {
		realloc(buffer, s);
		return new ILData(buffer, read, kILDataTakeOwnershipOfMallocBuffer);
	}
}

int ILStream::fileDescriptor() {
	ILMutexAcquisition(&this->_mutex);
	return _fileDescriptor;
}

bool ILStream::isValid() {
	ILMutexAcquisition(&this->_mutex);
	return _fileDescriptor >= 0;
}

void ILStream::close() {
	ILMutexAcquisition(&this->_mutex);

	if (_fileDescriptor >= 0) {
		::close(_fileDescriptor);
		_fileDescriptor = -1;
	}
	
	if (this->runLoop())
		this->runLoop()->signalReady();	
}

void ILStream::signalReadyForReadingAndWriting(bool reading, bool writing) {
	ILMutexAcquisition(&this->_mutex);
	
	_canRead = reading;
	_canWrite = writing;
	
	if (this->runLoop())
		this->runLoop()->signalReady();	
}

void ILStream::spin() {
	ILMutexAcquisition(&this->_mutex);

	if (!this->runLoop())
		return;
		
	if (!_fileDescriptor >= 0)
		this->runLoop()->currentMessageHub()->deliverMessage(new ILMessage(kILStreamDidCloseWithErrorMessage, this, NULL));
	else {
		if (_canRead)
			this->runLoop()->currentMessageHub()->deliverMessage(new ILMessage(kILStreamNowReadyForReadingMessage, this, NULL));

		if (_canWrite)
			this->runLoop()->currentMessageHub()->deliverMessage(new ILMessage(kILStreamNowReadyForWritingMessage, this, NULL));
	}
}
