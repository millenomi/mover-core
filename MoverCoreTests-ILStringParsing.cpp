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
	
	void ILStringParsingTests::testStringEquality() {
		ILTestEqualObjects(ILStr("ciao"), ILStr("ciao"));
		ILTestNotEqualObjects(ILStr("ciao"), ILStr("miao"));
	}
	
	void ILStringParsingTests::testSubstrings() {
		ILString* s = ILStr("Ciao ciao ciao!");
		
		ILString* rangeSubstring = s->substringWithRange(ILMakeRangeBetweenIndices(4, 9)),
			* beforeIndexSubstring = s->substringBeforeIndex(4),
			* fromIndexSubstring = s->substringFromIndex(10);
		
		ILTestNotNULL(rangeSubstring);
		ILTestNotNULL(beforeIndexSubstring);
		ILTestNotNULL(fromIndexSubstring);
		
		ILTestEqualObjects(ILStr(" ciao "), rangeSubstring);
		ILTestEqualObjects(ILStr("Ciao"), beforeIndexSubstring);
		ILTestEqualObjects(ILStr("ciao!"), fromIndexSubstring);
	}
	
	void ILStringParsingTests::testSearch() {
		ILString* s = ILStr("Ciao ciao ciao!");
		
		ILTestEqualValues(4, s->indexOfCharacter(' '));
		ILTestEqualValues(9, s->indexOfCharacter(' ', 5));
		ILTestEqualValues(ILNotFound, s->indexOfCharacter(' ', 10));
		ILTestEqualValues(ILNotFound, s->indexOfCharacter('x'));
	}
}
