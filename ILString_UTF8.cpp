/*
 *  ILString_UTF8.cpp
 *  MoverCore
 *
 *  Created by ∞ on 08/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ILString_UTF8.h"

extern bool ILStringUTF8FromCharacters(ILCodePoint* codePoints, size_t length,
									   uint8_t** UTF8Bytes, size_t* UTF8Length)
{
	// we do a naive estimate of required space thusly: given that most strings will have 1- or 2-byte-encoded code points, we simply do length * 2. If we're wrong (ick), we realloc() the whole thing * 2.5, * 3, * 3.5 and so on.
	// this does no more than four steps of reallocing (since the maximum size of a UTF-8 buffer is code points count * 4 long), and often less than that. It can waste quite a bit of space, though (more than 2x actual in the worst case).
	
	float estimateMultiplier = 2.0;
	size_t bufferLength = length * estimateMultiplier;
	
	uint8_t* buffer = (uint8_t*) malloc(bufferLength * sizeof(uint8_t));
	
	// The code that increases buffer size if we hit the ceiling.
#define ILStringUTF8FromCharacters_IncreaseBufferIfNeeded() \
	if (bufferLength <= usedBufferLength) { \
		estimateMultiplier += 0.5; \
		bufferLength = length * estimateMultiplier; \
		void* newBuf = realloc(buffer, bufferLength); \
		\
		if (!newBuf) { \
			free(buffer); \
			return false; \
		} else \
			buffer = (uint8_t*) newBuf; \
	}
	
	size_t usedBufferLength = 0;
	size_t i; for (i = 0; i < length; i++) {
		
		ILCodePoint p = codePoints[i];
		
		// we increase the current offset to the desired end, then use the new offset to realloc if needed, then write into it going backwards.
		
		if (p <= 0x7F) { // one byte, starting with 0
			
			usedBufferLength++;
			ILStringUTF8FromCharacters_IncreaseBufferIfNeeded();
			
			buffer[usedBufferLength - 1] = (uint8_t) p;
			
		} else if (p >= 0x80 && p <= 0x7FF) {
			// CP: 00000aaa bbbbbbbb
			// UTF-8 -> 110aaabb 10bbbbbb

			usedBufferLength += 2;
			ILStringUTF8FromCharacters_IncreaseBufferIfNeeded();

			// First byte.
			buffer[usedBufferLength - 2] =
				0xC0 /* 11000000 */ |
				(p >> 6 /* ******00 000aaabb */ & 0x1F /* 000_____ */)
			/* == 11000000 | 000aaabb == 110aaabb */;
			
			// Second byte.
			buffer[usedBufferLength - 1] =
				0x80 /* 10000000 */ |
				(p & 0x3F /* 00______ */)
			/* == 10000000 | 00bbbbbb == 10bbbbbb */;
			
		} else if (p > 0x800 && p <= 0xFFFF) {
			// CP: aaaabbbb bbcccccc
			// UTF-8 -> 1110aaaa 10bbbbbb 10cccccc
			
			usedBufferLength += 3;
			ILStringUTF8FromCharacters_IncreaseBufferIfNeeded();

			// First byte.
			buffer[usedBufferLength - 3] = 
				0xE0 /* 11100000 */ |
				(p >> 12 /* ******** ****aaaa */ & 0xF /* 0000____ */);
			
			// Second byte.
			buffer[usedBufferLength - 2] = 
				0x80 |
				(p >> 6 & 0x3F);
			
			// Third byte.
			buffer[usedBufferLength - 1] = 
				0x80 |
				p & 0x3F;
			
		}
#if ILPlatformCoreSupportEntireUnicodeRange
		else if (p > 0x010000 && p <= 0x10FFFF) {
			// CP: 000aaabb bbbbcccc ccdddddd
			// UTF-8 -> 11110aaa 10bbbbbb 10cccccc 10dddddd
			
			usedBufferLength += 4;
			ILStringUTF8FromCharacters_IncreaseBufferIfNeeded();

			// First byte.
			buffer[usedBufferLength - 4] =
				0xF0 /* 11110000 */ |
				(p >> 18 & 0x7 /* 00000___ */);
			
			// Second byte.
			buffer[usedBufferLength - 3] =
				0x80 |
				(p >> 12 & 0x3F);
			
			// Third byte.
			buffer[usedBufferLength - 2] =
				0x80 |
				(p >> 6 & 0x3F);
			
			// Fourth byte.
			buffer[usedBufferLength - 1] =
				0x80 |
				(p & 0x3F);
			
		}
#endif
		else {
			// not a valid code point (or wide cp with wide cps turned off).
			free(buffer);
			return false;
		}
		
	}
	
	*UTF8Bytes = buffer;
	*UTF8Length = bufferLength;
	
	return true;
}

extern bool ILStringCodePointsFromUTF8(uint8_t* bytes, size_t length,
									   ILCodePoint** codePoints, size_t* codePointsLength) {
	
	// reverse estimate: since the number of UTF-8 bytes is NEVER less than the number of code points it encodes, we can simply allocate the same number of bytes and never need to realloc().
	uint8_t* buffer = (uint8_t*) malloc(length);
	
	size_t i = 0;
	size_t offset = 0;
	while (offset < length) {
		
		// is the first byte of the form 0...?
		if (bytes[offset] & 0x80 == 0) {
			buffer[i] = (ILCodePoint) bytes[offset];
			offset++;
		} else if (bytes[offset] & 0xC0 == 0xC0) { // is it 110...?
			// check bounds
			if (offset + 1 >= length) {
				free(buffer);
				return false;
			}
			
			// 110aaabb 10cccccc --> ...00aaa bbcccccc
			// 000___00 == 0x1C
			// 000000__ == 0x3
			// 00______ == 0x3F
			buffer[i] = 
				(0x1C & bytes[offset] << 6) | /* the 'a's */
				(0x3  & bytes[offset] << 6) | /* the 'b's */ 
				(0x3F & bytes[offset + 1]);   /* the 'c's */
			#warning TODO check for an overlong sequence

		} else if (false /* TODO bytes[offset] starts with 1110... */) {
			#warning TODO decode three-byte sequences
			#warning TODO check for an overlong sequence
		}
#if ILPlatformCoreSupportEntireUnicodeRange
		else if (false /* TODO bytes[offset] starts with 11110... */) {
			#warning TODO decode four-byte sequences
			#warning TODO check for an overlong sequence
		}
#endif
		else {
			free(buffer);
			return false;
		}
	}
	
#error TO BE CONTINUED
	return false;
}
