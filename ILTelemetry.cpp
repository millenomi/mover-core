/*
 *  ILTelemetry.cpp
 *  MoverCore
 *
 *  Created by ∞ on 25/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ILTelemetry.h"
#include "ILTelemetryStdoutLogger.h"

#include <cstring>

struct ILTelemetryImpl {
	ILTelemetryDelegate* delegate;
	bool on;
};

ILTelemetry::ILTelemetry() : ILObject() {
	_i = new ILTelemetryImpl;
	_i->delegate = NULL;
	_i->on = false;
	
	setDelegate(new ILTelemetryStdoutLogger());
	
	char* isOn = getenv("ILTelemetryIsOn");
	if (isOn && strcmp(isOn, "YES") == 0)
		setOn(true);
}

ILTelemetry::~ILTelemetry() {
	ILRelease(_i->delegate);
	
	delete _i;
}

ILUniqueConstant(kILTelemetryClassIdentity);

void* ILTelemetry::classIdentity() {
	return kILTelemetryClassIdentity;
}

ILRetainGetterSetterImpl(ILTelemetry,
						 ILTelemetryDelegate*, _i->delegate,
						 delegate, setDelegate)
ILAssignGetterSetterImpl(ILTelemetry,
						 bool, _i->on,
						 on, setOn)

void ILTelemetry::log(ILString* content, ILString* eventSourceKind, ILObject* source) {
	if (delegate() && on())
		delegate()->telemetryDidLogEvent(content, eventSourceKind, source);
}

ILTelemetry* ILTelemetry::sharedTelemetry() {
	static ILTelemetry* me = NULL; if (!me)
		me = ILRetain(new ILTelemetry());
	
	return me;
}


ILTelemetryDelegate::ILTelemetryDelegate() {}
ILTelemetryDelegate::~ILTelemetryDelegate() {}