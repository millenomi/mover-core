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

#include "ILStreamMonitor.h"

#include <cstring>

namespace Mover {
	ILTargetForMethod(ILStreamTests_isReadyForReading, ILStreamTests, isReadyForReading);
	
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
	
	void ILStreamTests::setUp() {
		_didRead = false;
		_buffer = NULL;
	}
	
	void ILStreamTests::testAsyncRead() {
		ILPipeSource* p = new ILPipeSource();
		
		ILStream* wr = p->openWritingStream(),
			* rd = p->openReadingStream();
		
		const char* testString = "This is just a pipe read/write test string!";
		ILData* toBeWritten = new ILData((uint8_t*) testString, strlen(testString), kILDataNoCopy);
		bool didWriteTestString = wr->writeAllOf(toBeWritten);
		wr->close();
		
		ILTestTrue(didWriteTestString);
		
		ILTarget* t = new ILStreamTests_isReadyForReading(this);
		ILStreamMonitor* monitor = new ILStreamMonitor(rd, t, NULL, NULL);
		_buffer = ILRetain(new ILData());
		
		while (!_didRead)
			ILRunLoop::current()->spinForAboutUpTo(0.10);
		
		monitor->endObserving();
		
		ILTestEqualObjects(_buffer, toBeWritten);
		
		ILRelease(_buffer);
	}
	
	void ILStreamTests::isReadyForReading(ILMessage* m) {
		ILStreamMonitor* monitor = m->sourceAs<ILStreamMonitor>();
		ILStream* s = monitor->stream();
		
		ILStreamError e;
		ILData* d = s->read(2048, &e);
		
		if (d)
			_buffer->appendData(d);
		else {
			ILTestTrue(e == kILStreamErrorEndOfFile || e == kILStreamErrorWouldHaveBlocked);
			
			if (e != kILStreamErrorWouldHaveBlocked)
				_didRead = true;
		}

	}

}