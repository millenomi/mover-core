/*
 *  ILMessage.h
 *  Argyle
 *
 *  Created by ∞ on 05/05/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef ILMessage_H
#define ILMessage_H 1

#include "PlatformCore.h"

/**
 A message is a single item of information about an event that occurred in an application. A message has a kind (an unique pointer value that is not dereferenced), retains its optional source object and copies an optional copiable payload.
 */
class ILMessage : public ILObject {
public:
	/** Creates a new message with specified kind, source and payload. The payload will be copied.  */
	ILMessage(void* kind, ILObject* source, ILObject* payload);
	~ILMessage();
	
	/** Returns the kind of this message. */
	void* kind();
	/** Returns the source object for this message. It may be NULL. */
	ILObject* source();
	/** Returns the payload for this message. */
	ILObject* payload();
	
	template <class T>
	T* sourceAs() {
		return static_cast<T*>(source());
	}
	
	template <class T>
	T* payloadAs() {
		return static_cast<T*>(payload());
	}
	
private:
	void* _kind;
	ILObject* _source;
	ILObject* _payload;
};


/** A target is an object that is able to receive ILMessage objects (messages). */
class ILTarget : public ILObject {
public:
	ILTarget();
	virtual ~ILTarget();
	
	/** Delivers the message to this target. The message may be processed synchronously or asynchronously. */
	virtual void deliverMessage(ILMessage* m) = 0;
};


/** The signature for a function that can be called by a ILFunctionTarget.
 @param m The message to receive.
 @param contest The context value passed to ILFunctionTarget::ILFunctionTarget() at creation time.
 */
typedef void (*ILTargetFunction)(ILMessage* m, void* context);

/** A target that delivers a message by calling a function synchronously. */
class ILFunctionTarget : public ILTarget {
public:
	/** Creates a target that delivers messages to the given function. The context parameter will not be dereferenced and will be passed with all invocations of that function. */
	ILFunctionTarget(ILTargetFunction f, void* context);
	
	/** Delivers the message by calling the associated function with this message and the context parameter passed at creation time. */
	virtual void deliverMessage(ILMessage* m);
	
private:
	ILTargetFunction _function;
	void* _context;
};

/** A target that delivers a message by calling a method on an object. This class is virtual; you create implementations for particular methods by using the @link ILTargetForMethod ILTargetForMethod macro.
 
	Note that the object passed to this class is NOT retained to avoid retain loops!
 */
class ILObjectTarget : public ILTarget {
public:
	ILObjectTarget(ILObject* o); // o is NOT RETAINED
	~ILObjectTarget();
	
	virtual ILObject* target();
	virtual void deliverMessage(ILMessage* m) = 0;
    
    // Makes the target stop working. Called by the backed object on destruction. Not thread-safe.
    virtual void disableTarget();
	
private:
	ILObject* _target;
};	

/** Creates a new subclass of ILObjectTarget that delivers messages by calling the given method of objects of the given class.
 
 @param targetClass The name of the ILObjectTarget subclass to create.
 @param receiverClass The name of the class whose instances will receive messages.
 @param method The name of the method that will receive the messages. The method must be public. Use the following signature: <code>void someMethodName(ILMessage* m)</code>.
 */
#define ILTargetForMethod(targetClass, receiverClass, method) \
	class targetClass : public ILObjectTarget { \
	public: \
		targetClass(receiverClass* me) : ILObjectTarget(me) {} \
		virtual void deliverMessage(ILMessage* m) { receiverClass* t = ((receiverClass*)this->target()); if (t) t->method(m); } \
	}

#endif // #ifndef ILMessage_H
