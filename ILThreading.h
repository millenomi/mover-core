/*
 *  ILThreading.h
 *  MoverCore
 *
 *  Created by ∞ on 15/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "PlatformCore.h"

class ILTarget;
class ILMessage;

#ifndef ILThreading_H
#define ILThreading_H 1

typedef void (*ILThreadMainFunction)(ILObject* object);

extern void ILThreadStart(ILThreadMainFunction function, ILObject* object);

struct ILMutexImpl;

class ILMutex : public ILObject {
public:
	ILMutex(); ~ILMutex();
	
	void lock();
	void unlock();
	bool tryLock();
	
private:
	ILMutexImpl* _me;
};

// Not a ILObject class! Construct on the stack only.
class ILAcquiredMutex {
public:
	ILAcquiredMutex(ILMutex* o);
	~ILAcquiredMutex();
	
private:
	ILMutex* _mutex;
};

#endif // #ifndef ILThreading_H
