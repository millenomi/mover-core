
#include "PlatformCore.h"

#ifndef ILTesting_H
#define ILTesting_H 1

namespace ILTesting {
	
	class TestCase;
	class Results : public ILObject {
	public:
		Results();
		virtual ~Results();
		
		virtual void didBeginTestCase(TestCase* c, const char* what);
		virtual void passed(TestCase* c, const char* description, const char* file, unsigned long line);
		virtual void failed(TestCase* c, const char* description, const char* file, unsigned long line);
	};
	
	class ResultsStdoutDisplay : public Results {
	public:
		virtual void didBeginTestCase(TestCase* c, const char* what);
		virtual void passed(TestCase* c, const char* description, const char* file, unsigned long line);
		virtual void failed(TestCase* c, const char* description, const char* file, unsigned long line);
	};	
	
	typedef enum {
		kILTestingXcodeShowFailuresAsWarnings = 0,
		kILTestingXcodeShowFailuresAsErrors = 1,
	} ILTestingXcodeResultsFailureDisplayMode;
	
	class XcodeResultsStdoutDisplay : public Results {
	public:
		XcodeResultsStdoutDisplay();
		virtual void didBeginTestCase(TestCase* c, const char* what);
		virtual void passed(TestCase* c, const char* description, const char* file, unsigned long line);
		virtual void failed(TestCase* c, const char* description, const char* file, unsigned long line);
		
		void setFailureDisplayMode(ILTestingXcodeResultsFailureDisplayMode m);
		void setShowPassingTests(bool showPassing);
		
	private:
		ILTestingXcodeResultsFailureDisplayMode _mode;
		bool _showPassing;
	};
	
	class TestCase : public ILObject {		
		Results* _r;
		const char* _name;
		
	protected:
		void began(const char* what);
		
	public:
		TestCase(Results* r = NULL, const char* name = NULL);
		virtual ~TestCase();
		virtual void run();
		virtual void runTests() = 0;
		
		virtual void setUp();
		virtual void tearDown();
		
		virtual void setUpClass();
		virtual void tearDownClass();
		
		const char* name();
		
		static Results* defaultResults();
		static void setDefaultResults(Results* r);
		
		static void runAllOf(TestCase* t, ...);
		
	protected:
		void assertTrue(bool c, const char* file, unsigned long line, const char* description, ...);
	};
}
		
#define ILTestWith(methodName) ILTestInvoke(methodName, #methodName)
	
#define ILTestInvoke(methodName, description) \
	{ \
		ILReleasePool pool; \
		this->setUp(); \
		this->began(description); \
		this->methodName(); \
		this->tearDown(); \
	}
	
#define ILTestTrue(condition) this->assertTrue(\
	condition, \
	__FILE__, __LINE__, \
	"%s", #condition)

#define ILTestFalse(condition) this->assertTrue(\
	!(condition), \
	__FILE__, __LINE__, \
	"!(%s)", #condition)
	
#define ILTestNULL(x) ILTestTrue(x == NULL)
#define ILTestNotNULL(x) ILTestTrue(x != NULL)
	
// TODO add unexpected value in the description for these

#define ILTestIdenticalValues(a, b) ILTestTrue(sizeof(a) == sizeof(b) && a == b)
#define ILTestNotIdenticalValues(a, b) ILTestFalse(sizeof(a) == sizeof(b) && a == b)

#define ILTestEqualValues(a, b) ILTestTrue(a == b)
#define ILTestNotEqualValues(a, b) ILTestTrue(a != b)

#define ILTestEqualObjects(a, b) ILTestTrue((a)->equals((b)))
#define ILTestNotEqualObjects(a, b) ILTestFalse((a)->equals((b)))

#define ILTestNull(a) ILTestEqualValues(a, NULL)
#define ILTestNotNull(a) ILTestNotEqualValues(a, NULL)

#define ILTestCase(className) \
	public: \
	className(::ILTesting::Results* r = NULL) : ::ILTesting::TestCase(r, #className) {} \
	virtual void runTests()
// Use ILTestCase(MyClass) { ILTestWith(testXyz); }

#endif // ILTesting_H
