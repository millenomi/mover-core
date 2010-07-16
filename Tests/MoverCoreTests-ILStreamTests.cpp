/*
 *  MoverCoreTests-ILStreamTests.cpp
 *  MoverCore
 *
 *  Created by ∞ on 16/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "MoverCoreTests-ILStreamTests.h"
#include "ILStream.h"
#include "ILMessage.h"
#include "ILRunLoop.h"

#include <cstring>

namespace Mover {
	ILTargetForMethod(ILStreamTests_readyForReading, ILStreamTests, readyForReading);
	
	void ILStreamTests::testReadingUsingPipe() {
		int fd[2];
		bool didCreatePipes = (pipe(fd) == 0);
		ILTestTrue(didCreatePipes);
		
		if (didCreatePipes) {
			ILStream* s = new ILStream(fd[0]);
			
			const char* string = "testtesttest!";
			ILSize size = strlen(string);
			
			// TODO loop lie in testWritingUsingPipe.
			write(fd[1], (const void*) string, strlen(string));
			
			ILData* received = new ILData();
			while (received->length() < size) {
				ILData* dataReadFromPipe = s->readDataOfMaximumLength(size);
				ILTestNotNULL(dataReadFromPipe);
				if (!dataReadFromPipe)
					break;
				
				received->appendData(dataReadFromPipe);
			}
			
			ILTestEqualValues(received->length(), size);
			if (received->length() == size)
				ILTestTrue(memcmp(received->bytes(), string, size) == 0);
			
			close(fd[0]);
			s->close();
		}		
	}
	
	void ILStreamTests::testWritingUsingPipe() {
		int fd[2];
		bool didCreatePipes = (pipe(fd) == 0);
		ILTestTrue(didCreatePipes);
		
		if (didCreatePipes) {
			ILStream* s = new ILStream(fd[1]);
			
			const char* string = "testtesttest!";
			ILSize size = strlen(string);
			
			ILSize written = 0, writtenInLastPass;
			
			while (written < size) {
				bool writingDidSucceed = s->write(new ILData((uint8_t*) (string + written), size - written, kILDataNoCopy), &writtenInLastPass);
								
				ILTestTrue(writingDidSucceed);
				if (!writingDidSucceed)
					return;
				
				written += writtenInLastPass;
			}
			
			
			ILData* received = new ILData();
			
			while (received->length() < size) {
				uint8_t bytes[2048];
				
				int readInLastPass = read(fd[0], bytes, sizeof(bytes));
				ILTestTrue(readInLastPass > 0);
				if (readInLastPass <= 0)
					return;
				
				received->appendBytes(bytes, readInLastPass);
			}
			
			ILTestEqualValues(received->length(), size);
			if (received->length() == size)
				ILTestTrue(memcmp(received->bytes(), string, size) == 0);
			
			close(fd[1]);
			s->close();
		}		
	}
	
	void ILStreamTests::testReadMessages() {
		int fd[2];
		bool didCreatePipes = (pipe(fd) == 0);
		ILTestTrue(didCreatePipes);
		
		if (didCreatePipes) {
			_didReceiveReadyForReading = false;

			ILStream* s = new ILStream(fd[0]);
			// s->observeSignificantChanges();
			
			ILTarget* toMe = new ILStreamTests_readyForReading(this);
			
			ILRunLoop::current()->currentMessageHub()
			  ->addTargetForMessagesOfKind(toMe,
										   kILStreamNowReadyForReadingMessage,
										   s);
			
			ILRunLoop::current()->addSource(s);
			
			const char* string = "testtesttest!";
			
			// TODO loop lie in testWritingUsingPipe.
			write(fd[1], (const void*) string, strlen(string));
						
			while (!_didReceiveReadyForReading)
				ILRunLoop::current()->spinForAboutUpTo(0.5);
			
			close(fd[0]);
			s->close();
			
			ILRunLoop::current()->removeSource(s);
			
			ILRunLoop::current()->currentMessageHub()
 			  ->removeTargetForMessagesOfKind(toMe,
											  kILStreamNowReadyForReadingMessage);
		}		
	}
	
	void ILStreamTests::readyForReading(ILMessage* m) {
		_didReceiveReadyForReading = true;
		ILStream* s = ILAs(ILStream, m->source());
		s->close();
	}
}
