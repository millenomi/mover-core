/*
 *  ILMessage.cpp
 *  Argyle
 *
 *  Created by ∞ on 05/05/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ILMessage.h"

ILMessage::ILMessage(void* k, ILObject* s, ILObject* p) : ILObject() {
	_kind = k;
	_source = ILRetain(s);
	_payload = p? ILRetain(p->copy()) : NULL;
}

ILMessage::~ILMessage() {
	ILRelease(_source);
	ILRelease(_payload);
}

void* ILMessage::kind() {
	return _kind;
}

ILObject* ILMessage::source() {
	return _source;
}

ILObject* ILMessage::payload() {
	return _payload;
}

// ~~~

ILTarget::ILTarget() {}
ILTarget::~ILTarget() {}

// ~~~

ILObjectTarget::ILObjectTarget(ILObject* o) : ILTarget() {
	_target = o;
}

ILObjectTarget::~ILObjectTarget() {
	ILRelease(_target);
}

ILObject* ILObjectTarget::target() { return _target; }

// ~~~

ILFunctionTarget::ILFunctionTarget(ILTargetFunction f, void* c) {
	_function = f;
	_context = c;
}

void ILFunctionTarget::deliverMessage(ILMessage* m) {
	_function(m, _context);
}
