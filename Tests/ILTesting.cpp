
#include "ILTesting.h"
#include <stdarg.h>
#include <stdio.h>

namespace ILTesting {
	
	void TestCase::runAllOf(TestCase* t, ...) {
		va_list l;
		va_start(l, t);
		
		TestCase* theCase = t;
		while (theCase) {
			theCase->run();
			theCase = va_arg(l, TestCase*);
		}
		
		va_end(l);
	}
	
	static Results* ILTestDefaultResults = NULL;
	
	Results* TestCase::defaultResults() {
		return ILTestDefaultResults;
	}
	
	void TestCase::setDefaultResults(Results* r) {
		if (r != ILTestDefaultResults) {
			ILRelease(ILTestDefaultResults);
			ILTestDefaultResults = ILRetain(r);
		}
	}
	
	TestCase::TestCase(Results* r, const char* name) {
		if (!r)
			r = TestCase::defaultResults();
		
		_r = ILRetain(r);
		_name = name;
	}
	
	const char* TestCase::name() {
		return _name;
	}
	
	TestCase::~TestCase() {
		ILRelease(_r);
	}
	
	void TestCase::assertTrue(bool c, const char* file, unsigned long line, const char* description, ...) {
		va_list l;
		va_start(l, description);
		
		//		char* newString;
		//		vasprintf(&newString, description, l);
		
		char dummy;
		int size = vsnprintf(&dummy, 1, description, l);
		
		va_end(l);
		
		va_start(l, description);
		
		char newString[ size + 1 ];
		vsnprintf(newString, size + 1, description, l);
		
		va_end(l);
		
		if (c)
			_r->passed(this, newString, file, line);
		else
			_r->failed(this, newString, file, line);
	}

	void TestCase::began(const char* what) {
		_r->didBeginTestCase(this, what);
	}
	
	void TestCase::setUp() {}
	void TestCase::tearDown() {}
	
// ------------ RESULTS
	
	void Results::didBeginTestCase(TestCase* c, const char* what) {}
	void Results::passed(TestCase* c, const char* description, const char* file, unsigned long line) {}
	void Results::failed(TestCase* c, const char* description, const char* file, unsigned long line) {}
	
	Results::Results() {}
	Results::~Results() {}
	
// ------------ ResultsStdoutDisplay
	
	void ResultsStdoutDisplay::didBeginTestCase(TestCase* c, const char* what) {
		printf(" == == = STARTING TEST: %s (from %s)\n", what, c->name());
	}
	
	void ResultsStdoutDisplay::passed(TestCase* c, const char* description, const char* file, unsigned long line) {
		printf(" ok: %s (at %s:%lu)\n", description, file, line);
	}
	
	void ResultsStdoutDisplay::failed(TestCase* c, const char* description, const char* file, unsigned long line) {
		fprintf(stderr, " !! FAILED: %s (at %s:%lu)\n", description, file, line);
	}
	
// ------------ XcodeResultsStdoutDisplay
	
	XcodeResultsStdoutDisplay::XcodeResultsStdoutDisplay() {
		_mode = kILTestingXcodeShowFailuresAsErrors;
		_showPassing = false;
	}
	
	void XcodeResultsStdoutDisplay::didBeginTestCase(TestCase* c, const char* what) {
		fprintf(stderr, "note: - Starting test %s (from %s)\n", what, c->name());
	}
	
	void XcodeResultsStdoutDisplay::passed(TestCase* c, const char* description, const char* file, unsigned long line) {
		if (_showPassing)
			fprintf(stderr, "%s:%lu: note: passed: %s\n", file, line, description);
	}
	
	void XcodeResultsStdoutDisplay::failed(TestCase* c, const char* description, const char* file, unsigned long line) {
		const char* prefix;
		switch (_mode) {
			case kILTestingXcodeShowFailuresAsErrors:
				prefix = "error";
				break;
			case kILTestingXcodeShowFailuresAsWarnings:
				prefix = "warning";
				break;
			default:
				ILAbort("Unknown Xcode results mode set!");
				return;
		}
		
		fprintf(stderr, "%s:%lu: %s: Failed test %s.\n", file, line, prefix, description);
	}
	
	void XcodeResultsStdoutDisplay::setFailureDisplayMode(ILTestingXcodeResultsFailureDisplayMode m) {
		_mode = m;
	}
	
	void XcodeResultsStdoutDisplay::setShowPassingTests(bool show) {
		_showPassing = show;
	}
	
}

