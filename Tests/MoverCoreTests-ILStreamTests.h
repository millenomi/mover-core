/*
 *  MoverCoreTests-ILStreamTests.h
 *  MoverCore
 *
 *  Created by ∞ on 16/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MOVER_CORE_TESTS_ILSTREAM
#define MOVER_CORE_TESTS_ILSTREAM 1

#include "PlatformCore.h"
#include "ILTesting.h"

class ILMessage;

namespace Mover {
	class ILStreamTests : public ILTesting::TestCase {
		ILTestCase(ILStreamTests) {
			ILTestWith(testReadWriteWithPipeAndShortcuts);
			ILTestWith(testAsyncRead);
            ILTestWith(testAsyncWrite);
		}
		
		void testReadWriteWithPipeAndShortcuts();
		void testAsyncRead();
        void testAsyncWrite();
		
		virtual void setUp();
		

		void isReadyForReadingForTestAsyncRead(ILMessage* m);
		void isReadyForWritingForTestAsyncWrite(ILMessage* m);
	private:
		bool _isTestingAsyncRead;
		bool _didRead;
		ILData* _buffer;
        
        bool _isTestingAsyncWrite;
        bool _didWrite;
        ILIndex _writtenIndex;
	};
}

#endif // #ifndef MOVER_CORE_TESTS_ILSTREAM