/*
 *  ILStream.h
 *  MoverCore
 *
 *  Created by ∞ on 14/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef ILStream_H
#define ILStream_H 1

#include "PlatformCore.h"
#include "ILSource.h"
#include "ILMessage.h"
#include "ILRunLoop.h"

#include <pthread.h>

class ILStream : public ILSource {
public:
	ILStream(int fileDescriptor);
	virtual ~ILStream();
	
	virtual bool write(ILData* data, ILSize* writtenSize);
	virtual ILData* readDataOfMaximumLength(ILSize s);
	
	virtual void close();
		
	// ASYNCHRONOUS STUFF.
	
	// Only meaningful if the target is non-NULL.
	virtual bool isReadyForWriting();
	virtual bool isReadyForReading();
	
	virtual bool isValid();
	
// implementation details. do not call.
	virtual void spin();
	friend void ILStreamMonitor(ILObject* o);
	
private:
	int _fileDescriptor;
	
	bool _canWrite, _canRead;
	bool _hadError;
	
	pthread_mutex_t _mutex;
	
	void signalReadyForReadingAndWriting(bool reading, bool writing);
	
protected:
	int fileDescriptor();
};

extern void* kILStreamDidCloseWithErrorMessage;
extern void* kILStreamNowReadyForReadingMessage;
extern void* kILStreamNowReadyForWritingMessage;

#endif // #ifndef ILStream_H
