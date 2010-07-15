/*
 *  ILData.h
 *  Argyle
 *
 *  Created by ∞ on 25/04/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef ILData_H
#define ILData_H 1

#include "PlatformCore.h"
#include "ILStructures.h"
#include <sys/types.h>

typedef enum {
	// The default. Copies the given buffer in a new one. The new buffer is mutable.
	kILDataMakeCopyOfBuffer = 0,
	
	// If unset, the buffer's content will be copied.
	// Otherwise, not. If so, it's the creator's responsibility to make sure the buffer remains alive as long as this object exists.
	// ILData objects produced with this option are immutable. (Copies are mutable.)
	kILDataNoCopy = 1,
	
	// If set, the buffer has been allocated by malloc() and the ILData object must take ownership of it. The object will call free() on it when it's destroyed.
	// ILData objects produced with this option are mutable.
	kILDataTakeOwnershipOfMallocBuffer = 2,
} ILDataCreationOptions;

// A wrapper for a byte buffer.
class ILData : public ILObject, public ILCopiable {
public:
	// Makes an empty ILData object.
	ILData();
	
	// Makes an object from the given buffer. By default, data will be copie
	ILData(uint8_t* bytes, ILSize length, ILDataCreationOptions options = kILDataMakeCopyOfBuffer);
	
	virtual ~ILData();

	uint8_t* bytes();
	size_t length();
	
	void appendBytes(uint8_t* bytes, size_t length);
	
	// ILCopiable overrides.
	virtual bool canCopy();
	virtual ILData* copy();
	
	// ILObject overrides.
	virtual void* classIdentity();
	virtual bool equals(ILObject* o);
	virtual uint64_t hash();
	
private:
	void initializeFromMallocBuffer(uint8_t* bytes, size_t length, bool makeCopy);
	void initializeFromUnownedBuffer(uint8_t* bytes, size_t length);
	
	bool _owns;
	uint8_t* _bytes;
	size_t _length;
};

extern void* const ILDataClassIdentity;

#endif
