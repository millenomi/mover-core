/*
 *  ILThreading.h
 *  MoverCore
 *
 *  Created by ∞ on 15/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "PlatformCore.h"

class ILTarget;
class ILMessage;

#ifndef ILThreading_H
#define ILThreading_H 1

typedef void (*ILThreadMainFunction)(ILObject* object);

extern void ILThreadStart(ILThreadMainFunction function, ILObject* object);

#endif // #ifndef ILThreading_H
