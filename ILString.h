/*
 *  ILString.h
 *  Argyle
 *
 *  Created by ∞ on 12/05/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef ILString_H
#define ILString_H 1

#include "ILObject.h"

class ILData;

typedef enum {
	kILStringEncodingUTF8 = 0,
} ILStringEncoding;

class ILString : public ILObject, public ILCopiable {
public:
	~ILString();
	
	// WARNING: used ONLY for string constants (literals). Assumes UTF-8 encoding.
	ILString(const char* stringLiteral);
	
	// USE THESE TWO INSTEAD for arbitrary data.
	static ILString* stringWithData(ILData* data, ILStringEncoding encoding);
	static ILString* stringWithCString(const char* aString, ILStringEncoding encoding);	
	
	ILData* dataUsingEncoding(ILStringEncoding encoding);
	char* nullTerminatedStringUsingEncoding(ILStringEncoding encoding);
    char* UTF8String();
		
	virtual bool canCopy();
	virtual ILString* copy();
	
	virtual void* classIdentity();
	virtual bool equals(ILObject* o);
	virtual uint64_t hash();
	
private:
	ILString(ILData* d, ILStringEncoding encoding);
	ILString(const char* buffer, ILStringEncoding encoding);
	ILData* _utf8data;
};

// For constant strings (literals).
extern ILString* ILStr(const char* stringLiteral);

extern void* ILStringClassIdentity;

#endif // #ifndef ILString_H
