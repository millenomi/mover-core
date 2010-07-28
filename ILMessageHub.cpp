/*
 *  ILMessageHub.cpp
 *  Argyle
 *
 *  Created by ∞ on 05/05/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ILMessageHub.h"
#include "ILRunLoop.h"

class ILMessageHubTarget : public ILObject {
public:
	ILMessageHubTarget(ILTarget* t, ILObject* d) : ILObject() {
		_target = (ILTarget*) ILRetain(t);
		_desiredSource = ILRetain(d); // should retain? TODO
	}
	
	~ILMessageHubTarget() {
		ILRelease(_desiredSource);
		ILRetain(_target);
	}
	
	ILObject* _desiredSource;
	ILTarget* _target;
};

ILMessageHub::ILMessageHub() : ILTarget() {
	_targets = (ILMap*) ILRetain(new ILMap());
}

ILMessageHub::~ILMessageHub() {
	ILRelease(_targets);
}

void ILMessageHub::addTarget(ILTarget* target, void* type, ILObject* source) {
	ILMessageHubTarget* t = new ILMessageHubTarget(target, source);
	ILNumber* key = new ILNumber(type);
	
	ILList* l = (ILList*) _targets->valueForKey(key);
	if (!l) {
		l = new ILList();
		_targets->setValueForKey(key, l);
	}
	
	l->addObject(t);
}

void ILMessageHub::removeTarget(ILTarget* t, void* kind) {
	if (kind != NULL)
		removeTargetForKindKey(t, NULL, new ILNumber(kind));
	else {
		ILMapIterator* m = _targets->copy()->iterate();
		ILNumber* n;
		while (m->getNextAs<ILNumber, ILObject>(&n, NULL))
			removeTargetForKindKey(t, NULL, n);
	}
}

void ILMessageHub::removeTargetsForObject(ILObject* p, void* kind) {
	if (kind != NULL)
		removeTargetForKindKey(NULL, p, new ILNumber(kind));
	else {
		ILMapIterator* m = _targets->copy()->iterate();
		ILNumber* n;
		while (m->getNextAs<ILNumber, ILObject>(&n, NULL))
			removeTargetForKindKey(NULL, p, n);
	}
}

void ILMessageHub::removeTargetForKindKey(ILTarget* t, ILObject* peer, ILNumber* key) {
	ILList* l = _targets->at<ILList>(key);
	if (l) {
		ILListIterator* i = l->copy()->iterate();
		ILMessageHubTarget* ht;
		
		size_t index = 0;
		while ((ht = i->nextAs<ILMessageHubTarget>())) {
			if (t != NULL && ht->_target == t)
				l->removeObjectAtIndex(index);
			else if (peer != NULL && peer->equals(ht->_target->peer()))
				l->removeObjectAtIndex(index);
			else
				index++;
		}
		
		if (l->count() == 0)
			_targets->removeValueForKey(key);
	}
}


void ILMessageHub::deliverMessage(ILMessage* m) {
	ILReleasePool pool;
	
	ILMapIterator* bucketsIterator = _targets->iterate();
	ILList* list;
	while (bucketsIterator->getNextAs<ILObject, ILList>(NULL, &list)) {
		
		ILListIterator* targetsIterator = list->iterate();
		ILMessageHubTarget* t;
		
		while ((t = targetsIterator->nextAs<ILMessageHubTarget>())) {
			if (!t->_desiredSource || (m->source() && t->_desiredSource->equals(m->source())))
				t->_target->deliverMessage(m);
		}
		
	}
}

ILMessageHub* ILMessageHub::currentHub() {
	return ILRunLoop::current()->currentMessageHub();
}
