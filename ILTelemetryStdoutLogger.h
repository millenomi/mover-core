/*
 *  ILTelemetryStdoutLogger.h
 *  MoverCore
 *
 *  Created by ∞ on 25/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef ILTelemetryStdoutLogger_H
#define ILTelemetryStdoutLogger_H 1

#include "PlatformCore.h"
#include "ILTelemetry.h"
struct ILTelemetryStdoutLoggerImpl;



// ~ ILTelemetryStdoutLogger ~

class ILTelemetryStdoutLogger : public ILTelemetryDelegate {
public:
	ILTelemetryStdoutLogger(); virtual ~ILTelemetryStdoutLogger();
	
	
	virtual void telemetryDidLogEvent(ILString* content, ILString* eventSourceKind, ILObject* source);	
	
	virtual void* classIdentity();
	
	void addAllowedSource(ILString* source);
	void removeAllowedSource(ILString* source);
	
private:
	friend class ILTelemetryStdoutLoggerImpl;
	ILTelemetryStdoutLoggerImpl* _i;
};

extern void* kILTelemetryStdoutLoggerClassIdentity;


#endif // #ifndef ILTelemetryStdoutLogger_H