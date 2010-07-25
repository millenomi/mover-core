/*
 *  ILStreamMonitor.h
 *  MoverCore
 *
 *  Created by ∞ on 24/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef ILStreamMonitor_H
#define ILStreamMonitor_H 1

#include "PlatformCore.h"
#include "ILStream.h"
#include "ILRunLoop.h"
#include "ILSource.h"

struct ILStreamMonitorImpl;



// ~ ILStreamMonitor ~

extern void* kILStreamReadyForWritingMessage;
extern void* kILStreamReadyForReadingMessage;
extern void* kILStreamErrorOccurredMessage;

class ILStreamMonitor : public ILSource {
public:
	ILStreamMonitor(ILStream* s);
	ILStreamMonitor(ILStream* s, ILTarget* readTarget, ILTarget* writeTarget, ILTarget* errorTarget);
	virtual ~ILStreamMonitor();
	
	ILStream* stream();

	void beginObserving();
	void endObserving();
	
	virtual void spin();
	virtual ILRunLoop* runLoop();
	virtual void setRunLoop(ILRunLoop* r);
	
private:
	friend void ILStreamMonitoringThread(ILObject* o);
	friend class ILStreamMonitorImpl;
	ILStreamMonitorImpl* _i;
};




#endif // #ifndef ILStreamMonitor_H