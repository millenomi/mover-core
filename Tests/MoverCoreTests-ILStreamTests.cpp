/*
 *  MoverCoreTests-ILStreamTests.cpp
 *  MoverCore
 *
 *  Created by ∞ on 16/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

// Disabled, pending a better ILStream impl.


#include "MoverCoreTests-ILStreamTests.h"
#include "ILStream.h"

#include "ILMessage.h"
#include "ILRunLoop.h"

#include "ILPipeSource.h"

#include <cstring>

namespace Mover {
	void ILStreamTests::testReadWriteWithPipeAndShortcuts() {
		ILPipeSource* p = new ILPipeSource();
		
		ILStream* wr = p->openWritingStream(),
			* rd = p->openReadingStream();
		
		const char* testString = "This is just a pipe read/write test string!";
		ILData* toBeWritten = new ILData((uint8_t*) testString, strlen(testString), kILDataNoCopy);
		
		bool didWriteTestString = wr->writeAllOf(toBeWritten);
		wr->close();
		
		ILTestTrue(didWriteTestString);
		
		ILData* received = rd->readUntilEnd();
		
		ILTestNotNULL(received);
		ILTestEqualObjects(received, toBeWritten);
		
		rd->close();
	}
}