
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
		virtual void passed(TestCase* c, const char* description);
		virtual void failed(TestCase* c, const char* description);
	};
	
	class ResultsStdoutDisplay : public Results {
	public:
		virtual void didBeginTestCase(TestCase* c, const char* what);
		virtual void passed(TestCase* c, const char* description);
		virtual void failed(TestCase* c, const char* description);		
	};	
	
	class TestCase : public ILObject {		
		Results* _r;
		const char* _name;
		
	protected:
		void began(const char* what);
		
	public:
		TestCase(Results* r = NULL, const char* name = NULL);
		virtual ~TestCase();
		virtual void run() = 0;
		
		virtual void setUp();
		virtual void tearDown();
		
		const char* name();
		
		static Results* defaultResults();
		static void setDefaultResults(Results* r);
		
		static void runAllOf(TestCase* t, ...);
		
	protected:
		void assertTrue(bool c, const char* description, ...);
	};
		
#define ILTestWith(methodName) ILTestInvoke(methodName, #methodName)
	
#define ILTestInvoke(methodName, description) \
	{ \
		ILReleasePool pool; \
		this->setUp(); \
		this->began(description); \
		this->methodName(); \
		this->tearDown(); \
	}
	
#define ILTestTrue(condition) this->assertTrue(condition, "%s at %s:%d", #condition, __FILE__, __LINE__)
#define ILTestFalse(condition) this->assertTrue(!(condition), "!(%s) at %s:%d", #condition, __FILE__, __LINE__)

#define ILTestNULL(x) this->assertTrue(x == NULL, "is NULL: %s at %s:%d", #x, __FILE__, __LINE__)
#define ILTestNotNULL(x) this->assertTrue(x != NULL, "is not NULL: %s at %s:%d", #x, __FILE__, __LINE__)
	
#define ILTestIdenticalValues(a, b) this->assertTrue(sizeof(a) == sizeof(b) && a == b, "%s == %s at %s:%d", #a, #b, __FILE__, __LINE__)
#define ILTestNotIdenticalValues(a, b) this->assertTrue(sizeof(a) != sizeof(b) || a != b, "%s != %s at %s:%d", #a, #b, __FILE__, __LINE__)

#define ILTestEqualValues(a, b) this->assertTrue(a == b, "%s == %s at %s:%d", #a, #b, __FILE__, __LINE__)
#define ILTestNotEqualValues(a, b) this->assertTrue(a != b, "%s != %s at %s:%d", #a, #b, __FILE__, __LINE__)

#define ILTestEqualObjects(a, b) this->assertTrue((a)->equals(b), "%s equal()s %s at %s:%d", #a, #b, __FILE__, __LINE__)
#define ILTestNotEqualObjects(a, b) this->assertTrue(!((a)->equals(b)), "%s not equal()s %s at %s:%d", #a, #b, __FILE__, __LINE__)
}

#define ILTestNull(a) ILTestEqualValues(a, NULL)
#define ILTestNotNull(a) ILTestNotEqualValues(a, NULL)

#define ILTestCase(className) \
	public: \
	className(::ILTesting::Results* r = NULL) : ::ILTesting::TestCase(r, #className) {} \
	virtual void run()
// Use ILTestCase(MyClass) { ILTestWith(testXyz); }

#endif // ILTesting_H
