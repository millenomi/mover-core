/*
 *  ILThreading.cpp
 *  MoverCore
 *
 *  Created by ∞ on 15/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ILThreading.h"
#include <pthread.h>

#include "ILMessage.h"

class ILThreadArguments {
public:
	ILThreadMainFunction function;
	ILObject* argument;
};

static void* ILThreadMain(ILThreadArguments* args) {
	ILReleasePool pool;
	
	ILThreadMainFunction f = args->function;
	f(args->argument);
	ILRelease(args->argument);
	
	delete args;
	return NULL;
}

void ILThreadStart(ILThreadMainFunction function, ILObject* object) {
	ILThreadArguments* args = new ILThreadArguments;
	args->function = function;
	args->argument = object;
	
	ILRetain(object);
	pthread_t thread;
	pthread_create(&thread, NULL, (void* (*)(void*)) &ILThreadMain, args);
}
