/*
 *  MoverCoreTests-ConsumptionQueue.h
 *  MoverCore
 *
 *  Created by ∞ on 11/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MOVER_CORE_TESTS_CONSUMPTION_QUEUE_H
#define MOVER_CORE_TESTS_CONSUMPTION_QUEUE_H 1

#include "PlatformCore.h"
#include "ILTesting.h"
#include "ConsumptionQueue.h"

namespace Mover {

	class ConsumptionQueueTests : public ILTesting::TestCase {
		ILTestCase(ConsumptionQueueTests) {
			ILTestWith(testOpportunisticReading);
			ILTestWith(testPortionReading);
			ILTestWith(testPortionReadingOnMultibyteDataParts);
			ILTestWith(testOffsetFinding);
		}

		ConsumptionQueue* _queue;
		
		virtual void setUp();
		virtual void tearDown();
				
		void testOpportunisticReading();
		void testPortionReading();
		void testPortionReadingOnMultibyteDataParts();
		void testOffsetFinding();
	};
	
}
#endif // #ifndef MOVER_CORE_TESTS_CONSUMPTION_QUEUE_H
