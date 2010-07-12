/*
 *  MoverCoreTests-ILStringParsing.cpp
 *  MoverCore
 *
 *  Created by ∞ on 12/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "MoverCoreTests-ILStringParsing.h"

namespace Mover {
	void ILStringParsingTests::testByParsingInteger() {
		ILString* str = ILStr("1542398076");
		long long l = str->integerValueAtIndex(0);
		ILTestEqualValues(l, 1542398076);
		
		str = ILStr("-1542398076");
		l = str->integerValueAtIndex(0);
		ILTestEqualValues(l, -1542398076);
	}
	
	void ILStringParsingTests::testByParsingIntegerAtIndex() {
		ILString* str = ILStr("ciao 1542398076 miao -1542398076");
		long long l = str->integerValueAtIndex(5);
		ILTestEqualValues(l, 1542398076);
		
		l = str->integerValueAtIndex(21);
		ILTestEqualValues(l, -1542398076);
	}
}
