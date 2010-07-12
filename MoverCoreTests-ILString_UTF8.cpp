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
	
	void ILString_UTF8Tests::testOverlongSequence() {
		// This is an overlong sequence encoding ASCII 13 (\r).
		uint8_t overlongSequence[] = { 0xC0, 0x8D };
		size_t lengthOfEncodedString = sizeof(overlongSequence) / sizeof(uint8_t);
		
		ILCodePoint* codePoints;
		size_t codePointsLength;

		ILTestFalse(ILStringCodePointsFromUTF8(overlongSequence, lengthOfEncodedString, &codePoints, &codePointsLength));
	}
	
	void ILString_UTF8Tests::testErroneouslyContinuedSequence() {
		// This is a sequence that starts with a two-byte seq marker, but has three continuation bytes.
		uint8_t erroneouslyContinuedSequence[] = { 0xE2, 0x88, 0x9E, 0x9E };
		size_t lengthOfEncodedString = sizeof(erroneouslyContinuedSequence) / sizeof(uint8_t);
		
		ILCodePoint* codePoints;
		size_t codePointsLength;
		
		ILTestFalse(ILStringCodePointsFromUTF8(erroneouslyContinuedSequence, lengthOfEncodedString, &codePoints, &codePointsLength));		
	}

	void ILString_UTF8Tests::testIncreasingBufferSizeDuringEncoding() {
		ILCodePoint codePoints[] = {
			0x221E, 0x221E, 0x221E
		}; // These code points take 9 bytes, more than twice their count (the starting size for the UTF-8 encoding buffer in ILStringUTF8FromCodePoints.
		
		uint8_t* bytes;
		size_t length;
		
		bool done;
		ILTestTrue((done = ILStringUTF8FromCodePoints(codePoints, 3, &bytes, &length)));
		
		if (done) {
			ILTestEqualValues(length, 9);
			
			if (length == 9) {
				ILTestEqualValues(bytes[0], 0xE2);
				ILTestEqualValues(bytes[1], 0x88);
				ILTestEqualValues(bytes[2], 0x9E);
				ILTestEqualValues(bytes[3], 0xE2);
				ILTestEqualValues(bytes[4], 0x88);
				ILTestEqualValues(bytes[5], 0x9E);
				ILTestEqualValues(bytes[6], 0xE2);
				ILTestEqualValues(bytes[7], 0x88);
				ILTestEqualValues(bytes[8], 0x9E);
			}
			
			free(bytes);
		}
	}
	
#if ILPlatformCoreSupportEntireUnicodeRange
	void ILString_UTF8Tests::testInvalidCodePointEncoding() {
		ILCodePoint invalidCodePoint = 0x20FFFF;
		
		uint8_t* bytes;
		size_t bytesLength;
		
		ILTestFalse(ILStringUTF8FromCodePoints(&invalidCodePoint, 1, &bytes, &bytesLength));
	}
#endif
	
}
