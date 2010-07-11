/*
 *  MoverCoreTests-ILString_UTF8.h
 *  MoverCore
 *
 *  Created by ∞ on 11/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MOVER_CORE_TESTS_ILSTRING_UTF8
#define MOVER_CORE_TESTS_ILSTRING_UTF8 1

#include "PlatformCore.h"
#include "ILTesting.h"

namespace Mover {
	class ILString_UTF8Tests : ILTesting::TestCase {
		ILTestCase(ILString_UTF8Tests) {
			ILTestWith(testFromASCIIAndBack);
			ILTestWith(testFromMixedCharsAndBack);
			
			ILTestWith(testTruncatedString);
			ILTestWith(testOverlongSequence);
			ILTestWith(testErroneouslyContinuedSequence);
			
#if ILPlatformCoreSupportEntireUnicodeRange
			ILTestWith(testInvalidCodePointEncoding);
#endif
		}
		
		void testFromASCIIAndBack();
		void testFromMixedCharsAndBack();
		void testTruncatedString();
		void testOverlongSequence();
		void testErroneouslyContinuedSequence();
		
#if ILPlatformCoreSupportEntireUnicodeRange
		void testInvalidCodePointEncoding();
#endif
	};
}


#endif // #ifndef MOVER_CORE_TESTS_ILSTRING_UTF8