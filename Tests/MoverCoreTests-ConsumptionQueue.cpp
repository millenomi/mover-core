/*
 *  MoverCoreTests-ConsumptionQueue.cpp
 *  MoverCore
 *
 *  Created by ∞ on 11/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "MoverCoreTests-ConsumptionQueue.h"

namespace Mover {
	void ConsumptionQueueTests::setUp() {
		_queue = ILRetain(new ConsumptionQueue());
	}
	
	void ConsumptionQueueTests::tearDown() {
		ILRelease(_queue);
		_queue = NULL;
	}
	
	void ConsumptionQueueTests::testOpportunisticReading() {
		uint8_t a = 'A', b = 'B', c = 'C';
		ILData* dA = new ILData(&a, 1), * dB = new ILData(&b, 1), * dC = new ILData(&c, 1);
		
		_queue->appendData(dA);
		_queue->appendData(dB);
		_queue->appendData(dC);
		
		ILTestEqualValues(_queue->length(), (size_t) 3);
		
		ILData* x = _queue->availableDataWithMaximumLength(10000);
		ILTestEqualObjects(x, dA);
		
		ILTestEqualValues(_queue->length(), (size_t) 3);
		
		x = _queue->availableDataWithMaximumLength(10000);
		ILTestEqualObjects(x, dA);
		
		_queue->dequeueDataOfLength(1);
		ILTestEqualValues(_queue->length(), (size_t) 2);
		
		x = _queue->availableDataWithMaximumLength(10000);
		ILTestEqualObjects(x, dB);
		
		_queue->dequeueDataOfLength(1);
		ILTestEqualValues(_queue->length(), (size_t) 1);
		
		x = _queue->availableDataWithMaximumLength(10000);
		ILTestEqualObjects(x, dC);
		
		_queue->dequeueDataOfLength(1);
		ILTestEqualValues(_queue->length(), (size_t) 0);
		ILTestNULL(_queue->availableDataWithMaximumLength(10000));
	}
	
	void ConsumptionQueueTests::testPortionReading() {
		uint8_t a = 'A', b = 'B', c = 'C';
		ILData* dA = new ILData(&a, 1), * dB = new ILData(&b, 1), * dC = new ILData(&c, 1);
		
		uint8_t abc[] = { a, b, c };
		ILData* expected = new ILData(abc, 3);
		
		_queue->appendData(dA);
		_queue->appendData(dB);
		_queue->appendData(dC);
		
		ILData* newData = _queue->dataWithLength(3);
		ILTestEqualObjects(newData, expected);
		
		uint8_t bc[] = { b, c };
		expected = new ILData(bc, 2);
		
		_queue->dequeueDataOfLength(1);
		newData = _queue->dataWithLength(2);
		ILTestEqualObjects(newData, expected);
	}
	
	void ConsumptionQueueTests::testPortionReadingOnMultibyteDataParts() {
		uint8_t largishData[] = { 'A', 'b', 'C', 'd', 'E', 'f', 'G', 'h' };
		ILData* largishDataObject = new ILData(largishData, 8);
		
		uint8_t a = 'A';
		ILData* dA = new ILData(&a, 1);
		
		_queue->appendData(largishDataObject);
		_queue->appendData(dA);
		_queue->appendData(largishDataObject);
		
		ILData* newData;
		
		newData = _queue->dataWithLength(5);
		ILTestEqualObjects(newData, new ILData(largishData, 5));
		
		ILData* comparison;
		comparison = new ILData();
		comparison->appendBytes(largishData, 8);
		comparison->appendBytes(&a, 1);
		comparison->appendBytes(largishData, 8);
		newData = _queue->dataWithLength(8 + 1 + 8);
		ILTestEqualObjects(newData, comparison);
		
		comparison = new ILData();
		comparison->appendBytes(largishData, 8);
		comparison->appendBytes(&a, 1);
		comparison->appendBytes(largishData, 5);
		newData = _queue->dataWithLength(8 + 1 + 5);
		ILTestEqualObjects(newData, comparison);
		
		_queue->dequeueDataOfLength(5);
		
		comparison = new ILData();
		comparison->appendBytes(largishData + 5, 3);
		comparison->appendBytes(&a, 1);
		comparison->appendBytes(largishData, 5);
		newData = _queue->dataWithLength(3 + 1 + 5);
		ILTestEqualObjects(newData, comparison);			
	}
	
	void ConsumptionQueueTests::testOffsetFinding() {
		uint8_t largishData[] = { 'A', 'b', 'C', 'd', 0, 'f', 'G', 'h' };
		ILData* largishDataObject = new ILData(largishData, 8);
		
		uint8_t zero = 0;
		ILData* zeroObject = new ILData(&zero, 1);
		
		uint8_t a = 'A';
		ILData* dA = new ILData(&a, 1);
		
		_queue->appendData(largishDataObject); // inside a fragment
		_queue->appendData(zeroObject); // as an object of its own
		_queue->appendData(dA);
		_queue->appendData(zeroObject); // again, but after another fragment
		_queue->appendData(zeroObject); // at end
		
		// inside a fragment
		ILTestEqualValues((size_t) 4,
						  _queue->offsetForFirstByteWithValue(0));
		_queue->dequeueDataOfLength(5); // including the 0
		
		// as an object of its own
		ILTestEqualValues((size_t) 3,
						  _queue->offsetForFirstByteWithValue(0));
		_queue->dequeueDataOfLength(4); // including the 0
		
		// again, but after another fragment
		ILTestEqualValues((size_t) 1,
						  _queue->offsetForFirstByteWithValue(0));
		_queue->dequeueDataOfLength(2); // including the 0
		
		// at end
		ILTestEqualValues((size_t) 0,
						  _queue->offsetForFirstByteWithValue(0));
		_queue->dequeueDataOfLength(1); // including the 0
		
		// not anymore
		ILTestEqualValues((size_t) 0,
						  _queue->length());
		ILTestEqualValues(ILNotFound,
						  _queue->offsetForFirstByteWithValue(0));
	}
}