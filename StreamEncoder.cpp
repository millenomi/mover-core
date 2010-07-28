/*
 *  StreamEncoder.cpp
 *  MoverCore
 *
 *  Created by ∞ on 28/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "StreamEncoder.h"
#include "ILMessage.h"
#include "ILRunLoop.h"
#include "ILMessageHub.h"

namespace Mover {
	
	// -------------------------------------

	ILTargetClassForMethod(StreamEncoder, performProducingStreamPart);
	ILUniqueConstant(kMvrStreamEncoderPerformProducingStreamPartMessage);

	struct StreamEncoderImpl {
		StreamEncoder* self;
		
		ILMap* metadata;
		ILList* payloadContents;
		ILList* payloadKeys;
		ILList* payloadLengths;
		
		bool sealed;
		
		StreamEncoderDelegate* delegate;
		
		void performProducingStreamPart(ILMessage* m);
	};
	
	StreamEncoder::StreamEncoder() : ILObject() {
		_i = new StreamEncoderImpl;
		_i->self = this;
		
		_i->metadata = ILRetain(new ILMap());
		_i->payloadContents = ILRetain(new ILList());
		_i->payloadKeys = ILRetain(new ILList());
		_i->payloadLengths = ILRetain(new ILList());
		
		_i->sealed = false;
		_i->delegate = NULL;
		
		ILMessageHub::currentHub()->addTarget(new StreamEncoder_performProducingStreamPart(this), kMvrStreamEncoderPerformProducingStreamPartMessage, this);
	}
	
	StreamEncoder::~StreamEncoder() {
		// ~~ Destructor implementation here. ~~
		
		ILMessageHub::currentHub()->removeTargetsForObject(this);

		ILRelease(_i->metadata);
		ILRelease(_i->payloadContents);
		ILRelease(_i->payloadKeys);
		ILRelease(_i->payloadLengths);
				
		delete _i;
	}
	
	ILUniqueConstant(kStreamEncoderClassIdentity);
	
	void* StreamEncoder::classIdentity() {
		return kStreamEncoderClassIdentity;
	}
	
	ILAssignGetterSetterImpl(StreamEncoder,
							 StreamEncoderDelegate*, _i->delegate,
							 delegate, setDelegate)

	// -------------------------------------
	
	void StreamEncoder::setMetadataValueForKey(ILString* key, ILString* value) {
		if (_i->sealed)
			ILAbort("A stream encoder object cannot be modified while after it starts producing the stream.");
		
		_i->metadata->setValueForKey(key, value);
	}
	
	void StreamEncoder::addPayloadWithData(ILString* key, ILData* content) {
		if (_i->sealed)
			ILAbort("A stream encoder object cannot be modified while after it starts producing the stream.");

		if (_i->payloadKeys->containsObject(key))
			ILAbort("Attempt to insert duplicate payload key");
		
		_i->payloadContents->addObject(content);
		_i->payloadKeys->addObject(key);
		_i->payloadLengths->addObject(new ILNumber((uint64_t) content->length()));
	}
	
	void StreamEncoder::addPayloadWithStreamSource(ILString* key, ILStreamSource* source, ILSize size) {
		if (_i->sealed)
			ILAbort("A stream encoder object cannot be modified while after it starts producing the stream.");

		if (_i->payloadKeys->containsObject(key))
			ILAbort("Attempt to insert duplicate payload key");

		_i->payloadContents->addObject(source);
		_i->payloadKeys->addObject(key);
		_i->payloadLengths->addObject(new ILNumber((uint64_t) size));
	}
	
	// -------------------------------------
		
	void StreamEncoder::requestStreamPart() {
		ILRunLoop::current()->deliverMessage(new ILMessage(kMvrStreamEncoderPerformProducingStreamPartMessage, this));
	}
	
	void StreamEncoder::performProducingStreamPart(ILMessage* m) {
		_i->performProducingStreamPart(m); // avoids having to use _i's all over the place.
	}
	
	void StreamEncoderImpl::performProducingStreamPart(ILMessage* m) {
		if (!sealed) {
			sealed = true;
			if (delegate)
				delegate->streamEncoderWillBegin(self);
		}
		
#error TODO rest of it
	}
}
