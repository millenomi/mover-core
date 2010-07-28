/*
 *  ILMessageHub.h
 *  Argyle
 *
 *  Created by ∞ on 05/05/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef ILMessageHub_H
#define ILMessageHub_H 1

#include "ILObject.h"
#include "ILMessage.h"

/**
 A message hub is a target for messages that works to distribute those messages to a number of other targets. You register a number of targets with an instance of this class, then have messages delivered to that instance; the message will be delivered to all registered targets that want to receive that particular kind of message (optionally only from a given source).
 
 */
class ILMessageHub : public ILTarget {
public:
	ILMessageHub();
	~ILMessageHub();
	
	/** Adds a target that will be called whenever messages of the given kind are produced. If a 'source' object is given, only messages from that source will be delivered to that target; otherwise, if NULL, all messages of that kind will be delivered.
	 
		Adding the same target object twice for the same kind is a programmer error.
	 */
	void addTarget(ILTarget* target, void* kind, ILObject* source);
	
	/** Removes the given target from this message hub. If 'kind' is given and not NULL, the target will be removed only for messages of the given kind; otherwise, it will be removed for all possible kinds. */
	void removeTarget(ILTarget* t, void* kind = NULL);
	
	/** Removes all targets that refer to the given peer object. For example, removes all ILObjectTarget instances that would call a method on that object. (For more information on targets' peer objects, see the ILTarget::peer() method and its overrides in subclasses.)
	 
		If 'kind' is given and not NULL, only applicable targets that receive messages of the given kind will be removed; otherwise, all targets with the given peer object will be removed.
	 
	 */
	void removeTargetsForObject(ILObject* p, void* kind = NULL);
	
	virtual void deliverMessage(ILMessage* m);
	
	/** Returns the message hub associated with the current thread's run loop. */
	static ILMessageHub* currentHub();
	
private:
	void removeTargetForKindKey(ILTarget* t, ILObject* peer, ILNumber* key);
	ILMap* _targets;
};

#endif // #ifndef ILMessageHub_H
