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
	ILTargetClassForMethod(ILStreamTests, isReadyForReadingForTestAsyncRead);
	ILTargetClassForMethod(ILStreamTests, isReadyForWritingForTestAsyncWrite);
	
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
        _didWrite = false;
		_buffer = NULL;
		
		_isTestingAsyncRead = false;
        _isTestingAsyncWrite = false;
	}
    
    // -- testAsyncRead()
	
	void ILStreamTests::testAsyncRead() {
		ILPipeSource* p = new ILPipeSource();
		
		ILStream* wr = p->openWritingStream(),
			* rd = p->openReadingStream();
		
		const char* testString = "This is just a pipe read/write test string!";
		ILData* toBeWritten = new ILData((uint8_t*) testString, strlen(testString), kILDataNoCopy);
		bool didWriteTestString = wr->writeAllOf(toBeWritten);
		wr->close();
		
		ILTestTrue(didWriteTestString);
		
		_isTestingAsyncRead = true;
		
		ILTarget* t = new ILStreamTests_isReadyForReadingForTestAsyncRead(this);
		ILStreamMonitor* monitor = new ILStreamMonitor(rd, t, NULL, NULL);
		_buffer = ILRetain(new ILData());
		
		while (!_didRead)
			ILRunLoop::current()->spinForAboutUpTo(0.10);
		
		monitor->endObserving();
		
		_isTestingAsyncRead = false;
		
		// spin one final time to test whether spurious messages are being delivered.
        ILRunLoop::current()->spinForAboutUpTo(1.0);
		
		ILTestEqualObjects(_buffer, toBeWritten);
		
		ILRelease(_buffer);
	}
	
	void ILStreamTests::isReadyForReadingForTestAsyncRead(ILMessage* m) {
		ILTestTrue(_isTestingAsyncRead);
		
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

    // -- testAsyncRead()    
    
    void ILStreamTests::testAsyncWrite() {
		ILPipeSource* p = new ILPipeSource();
		
		ILStream* wr = p->openWritingStream(),
            * rd = p->openReadingStream();
		
        new ILStreamMonitor(wr, NULL, new ILStreamTests_isReadyForWritingForTestAsyncWrite(this), NULL);
        
        _writtenIndex = 0;
        _isTestingAsyncWrite = true;
        while (!_didWrite)
            ILRunLoop::current()->spinForAboutUpTo(0.10);
        
        _isTestingAsyncWrite = false;
        
        const char* toBeWritten = "This is just a pipe read/write test string!";
        
        ILData* received = rd->readUntilEnd();
        ILTestNotNULL(received);
        if (received)
            ILTestEqualObjects(received, new ILData((uint8_t*) toBeWritten, strlen(toBeWritten), kILDataNoCopy));
        
        // spin one final time to test whether spurious messages are being delivered.
        ILRunLoop::current()->spinForAboutUpTo(1.0);
		
    }
    
    void ILStreamTests::isReadyForWritingForTestAsyncWrite(ILMessage *m) {
        ILTestTrue(_isTestingAsyncWrite);
        ILTestFalse(_didWrite);
        
        if (!_didWrite) {            
            ILStreamMonitor* monitor = m->sourceAs<ILStreamMonitor>();
            ILStream* s = monitor->stream();
            
            const char* toBeWritten = "This is just a pipe read/write test string!";
            s->writeAllOf(new ILData((uint8_t*) &toBeWritten[_writtenIndex], 1, kILDataNoCopy));
            
            _writtenIndex++;
            if (_writtenIndex >= strlen(toBeWritten)) {
				monitor->endObserving();
				s->close();
                _didWrite = true;
			}
        }
    }
}
