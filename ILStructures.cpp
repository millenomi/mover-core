
#include "PlatformCore.h"
#include "ILStructures.h"

ILString* ILDescribeRange(ILRange r) {
	return ILString::stringWithFormat(ILStr("{Range from "
											"%" ILIndexFormat
											" to "
											"%" ILIndexFormat
											" (ending index %d)}"),
									  r.start, r.length, (int) r.start + r.length);
}

void ILLog(ILString* str) {
	fprintf(stderr, "%s\n", str->UTF8String());
}

