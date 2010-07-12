/*
 *  MoverCoreTests-ILStringParsing.h
 *  MoverCore
 *
 *  Created by ∞ on 12/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MOVER_CORE_TESTS_ILSTRINGPARSING
#define MOVER_CORE_TESTS_ILSTRINGPARSING 1

#include "PlatformCore.h"
#include "ILTesting.h"

namespace Mover {
	class ILStringParsingTests : ILTesting::TestCase {
		ILTestCase(ILStringParsingTests) {
			ILTestWith(testByParsingInteger);
			ILTestWith(testByParsingIntegerAtIndex);
		}
		
		void testByParsingInteger();
		void testByParsingIntegerAtIndex();
	};
}

#endif // #ifndef MOVER_CORE_TESTS_ILSTRINGPARSING
