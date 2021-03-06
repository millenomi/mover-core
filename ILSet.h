/*
 *  ILSet.h
 *  Argyle
 *
 *  Created by ∞ on 12/05/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef ILSet_H
#define ILSet_H 1

#include "ILObject.h"
#include "ILHash.h"

class ILSetIterator : public ILObject {
public:
	virtual ILObject* next() = 0;
    
    template <class T>
    T* nextAs() {
        ILObject* o = next();
        return static_cast<T*>(o);
    }
};

class ILSet : public ILObject, public ILCopiable {
public:
	ILSet();
	~ILSet();
	
	void addObject(ILObject* o);
	void removeObject(ILObject* o);
	bool containsObject(ILObject* o);
	
	void removeAllObjects();
	
	ILIndex count();
	
	ILSetIterator* iterate();
	
	virtual void* classIdentity();
	virtual bool equals(ILObject* other);
	virtual uint64_t hash();
	
	virtual bool canCopy();
	virtual ILSet* copy();
	
private:
	ILHash _content;
};

extern void* ILSetClassIdentity;

#endif // #ifndef ILSet_H
