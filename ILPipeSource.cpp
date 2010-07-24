/*
 *  ILPipeSource.cpp
 *  MoverCore
 *
 *  Created by ∞ on 24/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ILPipeSource.h"

struct ILPipeSourceImpl {
	int _fd[2];
	ILPipeStreamKind _kind;
	
	ILStream* _openStreamOfKind(ILPipeStreamKind k) {
		int i;
		
		if (k == kILPipeReadingStream)
			i = 0;
		else
			i = 1;
		
		if (_fd[i] == -1)
			return NULL;
		else {
			ILStream* s = new ILStream(_fd[i]);
			_fd[i] = -1; // lose ownership of it.
			return s;
		}	
	}
	
	bool _hasStreamOfKind(ILPipeStreamKind k) {
		int i;
		
		if (k == kILPipeReadingStream)
			i = 0;
		else
			i = 1;
		
		return _fd[i] == -1;
	}
};

ILPipeSource::ILPipeSource() : ILStreamSource() {
	_ps = new ILPipeSourceImpl;
	
	_ps->_kind = kILPipeReadingStream;
	if (pipe(_ps->_fd) != 0) {
		_ps->_fd[0] = -1;
		_ps->_fd[1] = -1;
	}
	
}

ILPipeSource::~ILPipeSource() {
	if (_ps->_fd[0] != -1)
		::close(_ps->_fd[0]);
	if (_ps->_fd[1] != -1)
		::close(_ps->_fd[1]);
	
	delete _ps;
}

ILStream* ILPipeSource::open() {
	return _ps->_openStreamOfKind(defaultStreamKind());
}

bool ILPipeSource::canOpen() {
	return !(_ps->_hasStreamOfKind(defaultStreamKind()));
}

ILPipeStreamKind ILPipeSource::defaultStreamKind() {
	return _ps->_kind;
}

void ILPipeSource::setDefaultStreamKind(ILPipeStreamKind kind) {
	_ps->_kind = kind;
}

ILStream* ILPipeSource::openReadingStream() {
	return _ps->_openStreamOfKind(kILPipeReadingStream);
}

ILStream* ILPipeSource::openWritingStream() {
	return _ps->_openStreamOfKind(kILPipeWritingStream);	
}
