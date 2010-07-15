/*
 *  ILString.cpp
 *  Argyle
 *
 *  Created by ∞ on 12/05/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ILString.h"

#include "ILData.h"
#include "ILList.h"

#include "ILString_UTF8.h"
#include <cstring>

#include <cstdarg>


// Used by ILString::integerAtIndex()
static const uint8_t ILStringNoDigitFound = -1;

static uint8_t ILStringValueForDigit(ILCodePoint digit) {
	if (digit > (ILCodePoint)'9' || digit < (ILCodePoint)'0')
		return ILStringNoDigitFound;
	else
		return digit - (ILCodePoint)'0';
}

int64_t ILString::integerValue() {
	return this->integerValueAtIndex(0);
}

int64_t ILString::integerValueAtIndex(size_t index) {
	int8_t sign = 1;
	long long value = 0;
	
	ILCodePoint* cps = this->codePoints();
	size_t len = this->length();
	
	bool isFirst = true;
	
	size_t i; for (i = index; i < len; i++) {
		if (isFirst) {
			isFirst = false;
			if (cps[i] == '-') {
				sign = -1;
				continue;
			}
		}
				
		uint8_t digit = ILStringValueForDigit(cps[i]);
		if (digit == ILStringNoDigitFound)
			break;
		else
			value = value * 10 + digit;
	}
	
	if (isFirst)
		return INT64_MIN;
	
	return value * sign;
}

ILString* ILStr(const char* stringLiteral) {
	return new ILString(stringLiteral);
}

ILString::ILString(const char* stringLiteral) : ILObject() {
	_utf8data = ILRetain(new ILData((uint8_t*) stringLiteral, strlen(stringLiteral)));
	_codePoints = NULL;
}

ILString::~ILString() {
	ILRelease(_utf8data);
	if (_codePoints)
		free(_codePoints);
}

ILString* ILString::stringWithData(ILData* data, ILStringEncoding encoding) {
	switch (encoding) {
		case kILStringEncodingUTF8:
		{
			ILCodePoint* codePoints;
			size_t codePointsLength;
			bool done = ILStringCodePointsFromUTF8(data->bytes(), data->length(), &codePoints, &codePointsLength);
			if (done)
				return new ILString(codePoints, codePointsLength, true);
			else
				return NULL;
		}
			
		default:
			return NULL;
	}
}

bool ILString::canCopy() {
	return true;
}

ILString* ILString::copy() {
	return new ILString(this->codePoints(), this->length());
}

void* ILString::classIdentity() {
	return ILStringClassIdentity;
}

bool ILString::equals(ILObject* o) {
	if (!o || o->classIdentity() != ILStringClassIdentity)
		return false;
	
	ILString* s = ILAs(ILString, o);
	if (this->length() != s->length())
		return false;
	
	return (memcmp(s->codePoints(), this->codePoints(), this->length()) == 0);
}

uint64_t ILString::hash() {
	// djb2 hash function, via TheMachineCharmer at http://stackoverflow.com/questions/2624192/good-hash-function-for-strings
	
	ILCodePoint* points = this->codePoints();
	size_t len = this->length();
	
	uint64_t hash = 5381;
	
	size_t i; for (i = 0; i < len; i++)
		hash = ((hash << 5) + hash) + points[i]; /* hash * 33 + c */
	
	return hash;
}

ILUniqueConstant(ILStringClassIdentity);

ILString::ILString(ILCodePoint* codePoints, size_t length) : ILObject() {
	this->initializeByUsingCodePointsArray(codePoints, length, false);
}

ILString::ILString(ILCodePoint* codePoints, size_t length, bool weOwnThisBuffer) : ILObject() {
	this->initializeByUsingCodePointsArray(codePoints, length, weOwnThisBuffer);
}

void ILString::initializeByUsingCodePointsArray(ILCodePoint* codePoints, size_t length, bool weOwnThisBuffer) {
	if (!weOwnThisBuffer) {
		_codePoints = (ILCodePoint*) malloc(length * sizeof(ILCodePoint));
		memcpy(_codePoints, codePoints, length * sizeof(ILCodePoint));
	} else
		_codePoints = codePoints;
	
	_length = length;
	
	_utf8data = NULL;
}

ILString* ILString::stringWithCString(uint8_t* aString, ILStringEncoding encoding) {
	return ILString::stringWithData(new ILData(aString, strlen((const char*) aString), kILDataNoCopy), encoding);
}

ILCodePoint* ILString::codePoints() {
	this->fillCodePointsIfNeeded();
	return _codePoints;
}

size_t ILString::length() {
	this->fillCodePointsIfNeeded();
	return _length;
}

void ILString::fillCodePointsIfNeeded() {
	if (!_codePoints) {
		if (!_utf8data)
			ILAbort("Malformed string: no UTF-8 or code points data");
		
		if (!ILStringCodePointsFromUTF8(_utf8data->bytes(), _utf8data->length(), &_codePoints, &_length))
			ILAbort("String literal not valid UTF-8! Aborting.");
		
		ILRelease(_utf8data);
		_utf8data = NULL;
	}
}

ILData* ILString::dataUsingEncoding(ILStringEncoding encoding) {
	switch (encoding) {
		case kILStringEncodingUTF8:
		{
			if (_utf8data) {
				ILReleaseLater(ILRetain(_utf8data));
				return _utf8data;
			} else if (_codePoints) {
				uint8_t* bytes;
				size_t len;
				
				if (ILStringUTF8FromCodePoints(_codePoints, _length, &bytes, &len)) {
					ILData* d = new ILData(bytes, len);
					free(bytes);
					return d;
				} else
					return NULL;
			} else
				ILAbort("Malformed string: no UTF-8 data or code points array.");
		}
		default:
			return NULL;
	}
}

const char* ILString::nullTerminatedStringUsingEncoding(ILStringEncoding encoding) {
	ILData* d = this->dataUsingEncoding(encoding);
	if (!d)
		return NULL;
	
	d = d->copy();
	uint8_t zero = 0;
	d->appendBytes(&zero, 1);
	return (const char*) d->bytes();
}

const char* ILString::UTF8String() {
	return this->nullTerminatedStringUsingEncoding(kILStringEncodingUTF8);
}

ILString* ILString::substringWithRange(ILRange r) {
	if (r.start >= this->length() || r.start + r.length > this->length())
		return NULL;
	
	ILCodePoint* cps = this->codePoints();
	cps += r.start;
	
	return new ILString(cps, r.length);
}

ILString* ILString::substringBeforeIndex(ILIndex i) {
	return this->substringWithRange(ILMakeRange(0, i));
}

ILString* ILString::substringFromIndex(ILIndex i) {
	return this->substringWithRange(ILMakeRange(i, this->length() - i));
}

ILIndex ILString::indexOfCharacter(ILCodePoint c, ILIndex startingIndex) {
	if (startingIndex >= this->length())
		return ILNotFound;
	
	ILCodePoint* cps = this->codePoints();
	ILSize s = this->length();
	
	ILIndex i; for (i = startingIndex; i < s; i++) {
		if (cps[i] == c)
			return i;
	}
	
	return ILNotFound;
}

ILList* ILString::componentsSeparatedByCharacter(ILCodePoint c) {
	ILList* l = new ILList();
	
	ILIndex lastStartingIndex = 0;
	ILIndex i = this->indexOfCharacter(c);
	while (i != ILNotFound) {
		l->addObject(this->substringWithRange(ILMakeRange(lastStartingIndex, i - lastStartingIndex)));
		lastStartingIndex = i + 1;
		if (lastStartingIndex >= this->length())
			break;
		
		i = this->indexOfCharacter(c, lastStartingIndex);
	}
	
	if (lastStartingIndex < this->length())
		l->addObject(this->substringFromIndex(lastStartingIndex));
	
	ILCodePoint* cp = this->codePoints();
	if (cp[this->length() - 1] == c)
		l->addObject(ILStr(""));
	
	return l;
}

ILString* ILString::stringWithFormat(ILString* format, ...) {
	va_list l;
	va_start(l, format);
	
	//		char* newString;
	//		vasprintf(&newString, description, l);
	
	const char* utf8Format = format->UTF8String();
	
	char dummy[2];
	int size = vsnprintf(dummy, sizeof(dummy), utf8Format, l);
	
	va_end(l);
	
	va_start(l, format);
	
	char newString[ size + 1 ];
	vsnprintf(newString, size + 1, utf8Format, l);
	
	va_end(l);
	
	return ILString::stringWithCString((uint8_t*) newString, kILStringEncodingUTF8);
}
