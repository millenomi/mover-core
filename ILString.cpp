/*
 *  ILString.cpp
 *  Argyle
 *
 *  Created by ∞ on 12/05/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ILString.h"
#include <string.h>

#include "ILData.h"

ILString::ILString(ILData* d, ILStringEncoding encoding) {
	_utf8data = ILRetain(d);
}

ILString::ILString(const char* buffer, ILStringEncoding encoding) {
	ILData* d = new ILData((uint8_t*) buffer, strlen(buffer));
	_utf8data = ILRetain(d);
}

// Constant ILStrings
ILString::ILString(const char* stringLiteral) {
	_utf8data = ILRetain(new ILData((uint8_t*) stringLiteral, strlen(stringLiteral), false));
}

ILString::~ILString() {
	ILRelease(_utf8data);
}

ILData* ILString::dataUsingEncoding(ILStringEncoding encoding) {
	return _utf8data;
}

char* ILString::nullTerminatedStringUsingEncoding(ILStringEncoding encoding) {
	ILData* d = _utf8data->copy();
	
	uint8_t nullByte = 0;
	
	d->appendBytes(&nullByte, 1);
	
	return (char*) d->bytes();
}

char* ILString::UTF8String() {
	return this->nullTerminatedStringUsingEncoding(kILStringEncodingUTF8);
}

ILString* ILString::stringWithData(ILData* data, ILStringEncoding encoding) {
	// TODO actual parsing of the data!
	return new ILString(data, encoding);
}

ILString* ILString::stringWithCString(const char* aString, ILStringEncoding encoding) {
	return new ILString(aString, encoding);
}


bool ILString::canCopy() {
	return true;
}

ILString* ILString::copy() {
	// return new ILString(_utf8data, kILStringEncodingUTF8);
	return this; // strings are (for now) immutable.
}


ILUniqueConstant(ILStringClassIdentity);

void* ILString::classIdentity() {
	return ILStringClassIdentity;
}

uint64_t ILString::hash() {
	return _utf8data->hash() ^ ((intptr_t)ILStringClassIdentity);
}

bool ILString::equals(ILObject* o) {
	
	if (!o || o->classIdentity() != ILStringClassIdentity)
		return false;
	
	if (o == this)
		return true;
	
	ILString* s = ILAs(ILString, o);
	
	if (_utf8data->length() != s->_utf8data->length())
		return false;
	
	return memcmp(_utf8data->bytes(), s->_utf8data->bytes(), _utf8data->length()) == 0;
	
}

ILString* ILStr(const char* stringLiteral) {
	return new ILString(stringLiteral);
}
