/*
 *  MoverCoreTests.cpp
 *  MoverCore
 *
 *  Created by ∞ on 05/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "MoverCoreTests.h"
#include "ConsumptionQueue.h"

#include "MoverCoreTests-ConsumptionQueue.h"
#include "MoverCoreTests-ILString_UTF8.h"
#include "MoverCoreTests-ILStringParsing.h"
#include "MoverCoreTests-ILStreamTests.h"

using namespace ILTesting;

namespace Mover {
	void RunCoreTests(Results* results) {
		TestCase::runAllOf(new ConsumptionQueueTests(results),
						   new ILString_UTF8Tests(results),
						   new ILStringParsingTests(results),
						   new ILStreamTests(results),
						   NULL);
	}
}
