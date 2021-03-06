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

#include <unistd.h>
#include "ILStructures.h"
#include "ILObject.h"

class ILData;
class ILList;

typedef enum {
	kILStringEncodingUTF8 = 0,
} ILStringEncoding;


// Can represent any Unicode code point.
typedef uint32_t ILWideRangeCodePoint;
#define ILWideRangeCodePoint_Max UINT32_MAX

// To save storage space, we use 16-bit (BMP) code points only. You can ask for wide-range CPs if needed, in which case all CP-using functions will automatically work with wide CPs.
#if !ILPlatformCoreSupportEntireUnicodeRange
typedef uint16_t ILCodePoint;
#define ILCodePoint_Max UINT16_MAX
#else
typedef uint32_t ILCodePoint;
#define ILCodePoint_Max ILWideRangeCodePoint_Max
#endif


class ILString : public ILObject, public ILCopiable {
public:
	~ILString();
	
	// WARNING: used ONLY for string constants (literals). Assumes UTF-8 encoding.
	// Use the ILStr() function instead if possible.
	ILString(const char* stringLiteral);
	
	// USE THESE TWO INSTEAD for arbitrary data.
	static ILString* stringWithData(ILData* data, ILStringEncoding encoding);
	static ILString* stringWithCString(const char* aString, ILStringEncoding encoding);
	
	static ILString* stringWithFormat(ILString* format, ...);
	
	// Use this if you have Unicode code points and you want to make a string out of 'em.
	ILString(ILCodePoint* codePoints, size_t length);
	
	// Makes an empty string.
	ILString();
	
	ILData* dataUsingEncoding(ILStringEncoding encoding);
	
	const char* nullTerminatedStringUsingEncoding(ILStringEncoding encoding); // guaranteed valid until release pool is drained.
    const char* UTF8String(); // shortcut for nullTerminatedStringUsingEncoding(kILStringEncodingUTF8).
		
	virtual bool canCopy();
	virtual ILString* copy();
	
	virtual void* classIdentity();
	virtual bool equals(ILObject* o);
	virtual uint64_t hash();
	
	// substrings
	ILString* substringWithRange(ILRange r);
	ILString* substringBeforeIndex(ILIndex i);
	ILString* substringFromIndex(ILIndex i);
	
	// search
	ILIndex indexOfCharacter(ILCodePoint c, ILIndex startingIndex = 0);
	ILList* componentsSeparatedByCharacter(ILCodePoint c);
	
	// integer
	int64_t integerValueAtIndex(size_t index);
	int64_t integerValue();
	
	// appending stuff
	void appendString(ILString* s);
	
private:
	ILString(ILCodePoint* codePoints, size_t length, bool weOwnThisBuffer);
	void initializeByUsingCodePointsArray(ILCodePoint* codePoints, size_t length, bool weOwnThisBuffer);
	
	ILCodePoint* codePoints();
	size_t length();
	void fillCodePointsIfNeeded();
	
	ILData* _utf8data;
	
	ILCodePoint* _codePoints;
	size_t _length;
};

// For constant strings (literals).
extern ILString* ILStr(const char* stringLiteral);

extern void* ILStringClassIdentity;

#endif // #ifndef ILString_H
