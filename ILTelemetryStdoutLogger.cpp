/*
 *  ILTelemetryStdoutLogger.cpp
 *  MoverCore
 *
 *  Created by ∞ on 25/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ILTelemetryStdoutLogger.h"


struct ILTelemetryStdoutLoggerImpl {
};

ILTelemetryStdoutLogger::ILTelemetryStdoutLogger() : ILTelemetryDelegate() {
	_i = new ILTelemetryStdoutLoggerImpl;
}

ILTelemetryStdoutLogger::~ILTelemetryStdoutLogger() {
	delete _i;
}

ILUniqueConstant(kILTelemetryStdoutLoggerClassIdentity);

void* ILTelemetryStdoutLogger::classIdentity() {
	return kILTelemetryStdoutLoggerClassIdentity;
}


void ILTelemetryStdoutLogger::telemetryDidLogEvent(ILString* content, ILString* eventSourceKind, ILObject* source) {
	
	fprintf(stdout, "%s - %p:\n\t%s\n", eventSourceKind->UTF8String(), source, content->UTF8String());
	fflush(stdout);
	
}
