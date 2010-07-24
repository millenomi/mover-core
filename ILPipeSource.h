/*
 *  ILPipeSource.h
 *  MoverCore
 *
 *  Created by ∞ on 24/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef ILPipeSource_H
#define ILPipeSource_H 1

#include "PlatformCore.h"
#include "ILStream.h"

typedef enum {
	kILPipeReadingStream,
	kILPipeWritingStream,
} ILPipeStreamKind;

struct ILPipeSourceImpl;

class ILPipeSource : public ILStreamSource {
public:
	ILPipeSource(); virtual ~ILPipeSource();
	
	virtual ILStream* open();
	virtual bool canOpen();
	
	ILStream* openReadingStream();
	ILStream* openWritingStream();
	
	ILPipeStreamKind defaultStreamKind();
	void setDefaultStreamKind(ILPipeStreamKind kind);
	
private:
	friend class ILPipeSourceImpl;
	ILPipeSourceImpl* _ps;
};

#endif // #ifndef ILPipeSource_H
