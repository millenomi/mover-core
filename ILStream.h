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
	virtual void beginMonitoring();
	virtual void endMonitoring();
	virtual bool isReadyForWriting();
	virtual bool isReadyForReading();
	
	virtual bool isValid();
	
	virtual void setRunLoop(ILRunLoop* loop);
	
// implementation details. do not call.
	virtual void spin();
	friend void ILStreamMonitor(ILObject* o);
	
	
// debug only
	void observeSignificantChanges();
	
private:
	int _fileDescriptor;
	
	bool _canWrite, _canRead, _didAnnounceClose;
	bool _hadError;
	
	bool _isMonitoring;
	
	pthread_mutex_t _mutex;
	
	void signalReadyForReadingAndWriting(bool reading, bool writing);
	
	bool _debug_showLogs;
	
protected:
	int fileDescriptor();
	void log(ILString* s);
};

extern void* kILStreamDidCloseWithErrorMessage;
extern void* kILStreamNowReadyForReadingMessage;
extern void* kILStreamNowReadyForWritingMessage;

class ILStreamSource : public ILObject {
public:
	virtual ILStream* open() = 0;
};

#endif // #ifndef ILStream_H
