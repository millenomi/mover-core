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
	
	ILSet* allowed;
	
};

ILTelemetryStdoutLogger::ILTelemetryStdoutLogger() : ILTelemetryDelegate() {
	_i = new ILTelemetryStdoutLoggerImpl;
	_i->allowed = ILRetain(new ILSet());
	
	char* defaultFilter = getenv("ILTelemetryStdoutAllowedSources");
	if (defaultFilter) {
		ILString* s = ILString::stringWithCString(defaultFilter, kILStringEncodingUTF8);
		if (s) {
			ILList* l = s->componentsSeparatedByCharacter((ILCodePoint)' ');
			ILListIterator* i = l->iterate(); 
			
			ILString* source;
			while ((source = i->nextAs<ILString>()))
				_i->allowed->addObject(source);
		}
	}
}

ILTelemetryStdoutLogger::~ILTelemetryStdoutLogger() {
	ILRelease(_i->allowed);
	delete _i;
}

ILUniqueConstant(kILTelemetryStdoutLoggerClassIdentity);

void* ILTelemetryStdoutLogger::classIdentity() {
	return kILTelemetryStdoutLoggerClassIdentity;
}


void ILTelemetryStdoutLogger::telemetryDidLogEvent(ILString* content, ILString* eventSourceKind, ILObject* source) {
	
	if (_i->allowed->count() > 0 && eventSourceKind != NULL && !_i->allowed->containsObject(eventSourceKind))
		return;
	
	fprintf(stdout, "%s - %p:\n\t%s\n", eventSourceKind->UTF8String(), source, content->UTF8String());
	fflush(stdout);
	
}

void ILTelemetryStdoutLogger::addAllowedSource(ILString* source) {
	_i->allowed->addObject(source);
}

void ILTelemetryStdoutLogger::removeAllowedSource(ILString* source) {
	_i->allowed->removeObject(source);
}
