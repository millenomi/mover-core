/*
 *  main-MoverCore-Tests.cpp
 *  MoverCore
 *
 *  Created by ∞ on 05/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */


#include "PlatformCore.h"
#include "ILTesting.h"

#include "MoverCoreTests.h"
using namespace Mover;

class MoverTestingResults : public ILTesting::XcodeResultsStdoutDisplay {
public:
	bool anyTestFailed;
	
	MoverTestingResults() : XcodeResultsStdoutDisplay() {
		anyTestFailed = false;
	}
	
	virtual void failed(ILTesting::TestCase* c, const char* description, const char* file, unsigned long line) {
		anyTestFailed = true;
		XcodeResultsStdoutDisplay::failed(c, description, file, line);
	}
};

int main(int argc, const char* argv[]) {

	ILReleasePool p;
	
	MoverTestingResults* r = new MoverTestingResults();
	RunCoreTests(r);
	
	return r->anyTestFailed? 1 : 0;
	
}