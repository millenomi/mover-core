/*
 *  MoverCoreTests-ILString_UTF8.cpp
 *  MoverCore
 *
 *  Created by ∞ on 11/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

// This file is encoded in UTF-8 (for the comments).

#include "MoverCoreTests-ILString_UTF8.h"
#include "ILString_UTF8.h"

#include <cstring>

namespace Mover {

	void ILString_UTF8Tests::testFromASCIIAndBack() {
		uint8_t ascii[] = {
			'H', 
			'e',
			'l',
			'l',
			'o' };
		
		ILCodePoint* codePoints;
		size_t codePointsLength;
		
		bool done;
		ILTestTrue((done = ILStringCodePointsFromUTF8(ascii, 5, &codePoints, &codePointsLength)));
		
		if (done) {
			ILTestEqualValues(codePointsLength, 5);
			ILTestEqualValues(codePoints[0], 'H');
			ILTestEqualValues(codePoints[1], 'e');
			ILTestEqualValues(codePoints[2], 'l');
			ILTestEqualValues(codePoints[3], 'l');
			ILTestEqualValues(codePoints[4], 'o');
			
			uint8_t* bytes;
			size_t length;
			ILTestTrue((done = ILStringUTF8FromCodePoints(codePoints, 5, &bytes, &length)));
			if (done) {
				ILTestEqualValues(length, 5);
				ILTestEqualValues(bytes[0], 'H');
				ILTestEqualValues(bytes[1], 'e');
				ILTestEqualValues(bytes[2], 'l');
				ILTestEqualValues(bytes[3], 'l');
				ILTestEqualValues(bytes[4], 'o');
				
				free(bytes);
			}
			
			free(codePoints);
		}
	}

	void ILString_UTF8Tests::testFromMixedCharsAndBack() {
		uint8_t encodedString[] = {
			'H', 
			'e',
			'l',
			'l',
			'o',
			' ',
			0xE2, 0x88, 0x9E /* ∞ U+221E INFINITY */,
			0xE3, 0x81, 0x8C /* が U+304B HIRAGANA LETTER GA */,
			0xC9, 0xB3 /* ɳ U+0273 LATIN SMALL LETTER N WITH RETROFLEX HOOK */ };
		size_t lengthOfEncodedString = sizeof(encodedString) / sizeof(uint8_t);
		
		ILCodePoint* codePoints;
		size_t codePointsLength;
		
		bool done;
		ILTestTrue((done = ILStringCodePointsFromUTF8(encodedString, lengthOfEncodedString, &codePoints, &codePointsLength)));
		
		if (done) {
			ILTestEqualValues(codePointsLength, 9);
			ILTestEqualValues(codePoints[0], 'H');
			ILTestEqualValues(codePoints[1], 'e');
			ILTestEqualValues(codePoints[2], 'l');
			ILTestEqualValues(codePoints[3], 'l');
			ILTestEqualValues(codePoints[4], 'o');
			ILTestEqualValues(codePoints[5], ' ');
			ILTestEqualValues(codePoints[6], 0x221E);
			ILTestEqualValues(codePoints[7], 0x304C);
			ILTestEqualValues(codePoints[8], 0x273);
			
			uint8_t* bytes;
			size_t length;
			ILTestTrue((done = ILStringUTF8FromCodePoints(codePoints, 9, &bytes, &length)));
			if (done) {
				ILTestEqualValues(length, lengthOfEncodedString);
				if (length == lengthOfEncodedString)
					ILTestTrue(memcmp(encodedString, bytes, lengthOfEncodedString) == 0);
				
				free(bytes);
			}
			
			free(codePoints);
		}
	}
	
	void ILString_UTF8Tests::testTruncatedString() {
		uint8_t encodedString[] = {
			'H', 
			'e',
			'l',
			'l',
			'o',
			' ',
			0xE2, 0x88, 0x9E /* ∞ U+221E INFINITY */,
			0xE3, 0x81, 0x8C /* が U+304B HIRAGANA LETTER GA */,
			0xC9 //, 0xB3 (ɳ U+0273 LATIN SMALL LETTER N WITH RETROFLEX HOOK missing its final byte)
		};
		
		size_t lengthOfEncodedString = sizeof(encodedString) / sizeof(uint8_t);
		
		ILCodePoint* codePoints;
		size_t codePointsLength;
		
		ILTestFalse(ILStringCodePointsFromUTF8(encodedString, lengthOfEncodedString, &codePoints, &codePointsLength));
	}
}
