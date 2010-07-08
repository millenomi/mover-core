/*
 *  ILString_UTF8.h
 *  MoverCore
 *
 *  Created by ∞ on 08/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef ILString_UTF8_H
#define ILString_UTF8_H 1

#include "PlatformCore.h"

// For internal use only.
// Returned storage is malloc()'d. Caller has responsibility to free() it after use.
// Returns false on encoding error.
extern bool ILStringUTF8FromCharacters(ILCodePoint* codePoints, size_t length,
									   uint8_t** UTF8Bytes, size_t* UTF8Length);

// For internal use only.
// Returned storage is malloc()'d. Caller has responsibility to free() it after use.
// Returns false on decoding error.
extern bool ILStringCodePointsFromUTF8(uint8_t* bytes, size_t length,
									   ILCodePoint** codePoints, size_t* codePointsLength);


#endif // #ifndef ILString_UTF8_H
