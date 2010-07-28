/*
 *  ILTelemetry.h
 *  MoverCore
 *
 *  Created by ∞ on 25/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef ILTelemetry_H
#define ILTelemetry_H 1

#include "PlatformCore.h"

struct ILTelemetryImpl;

#ifndef ILTelemetryCanBeOn
#define ILTelemetryCanBeOn 1
#endif

#if ILTelemetryCanBeOn

#define ILTelemetryIsOn() (ILTelemetry::sharedTelemetry()->on())

#define ILEventWithSourceObject(self, s, x, ...) do { \
		ILTelemetry::sharedTelemetry()->log(ILString::stringWithFormat(x, ## __VA_ARGS__), s, self); \
	} while (0)


#define ILEvent(s, x, ...) ILEventWithSourceObject(this, s, x, ## __VA_ARGS__)

#define ILCEvent(x, ...) ILEventWithSourceObject(NULL, ILStr(__FUNCTION__), x, ## __VA_ARGS__)

#else

#define ILTelemetryIsOn() (0)
#define ILEventWithSourceObject(self, s, x, ...)
#define ILEvent(s, x, ...) do {} while (0)
#define ILCEvent(x, ...) do {} while (0)

#endif

// ~ ILTelemetry ~

class ILTelemetryDelegate : public ILObject {
public:
	ILTelemetryDelegate(); virtual ~ILTelemetryDelegate();
	virtual void telemetryDidLogEvent(ILString* content, ILString* eventSourceKind, ILObject* source) = 0;
};
	

class ILTelemetry : public ILObject {
public:
	ILTelemetry(); virtual ~ILTelemetry();
	
	static ILTelemetry* sharedTelemetry();
	
	// --- 
	
	ILProperty(ILTelemetryDelegate*, delegate, setDelegate);
	ILProperty(bool, on, setOn);
	void log(ILString* content, ILString* eventSourceKind, ILObject* source);
	
	// ---
			   
	virtual void* classIdentity();
	
private:
	friend class ILTelemetryImpl;
	ILTelemetryImpl* _i;
};

extern void* kILTelemetryClassIdentity;


#endif // #ifndef ILTelemetry_H