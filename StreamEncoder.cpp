/*
 *  StreamEncoder.cpp
 *  MoverCore
 *
 *  Created by ∞ on 18/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "StreamEncoder.h"
#include "Protocol.h"

namespace Mover {
	
	ILUniqueConstant(kILStreamEncoderCanProceedMessage);
	
	StreamEncoder::StreamEncoder() {
		_sealed = false;
		
		_delegate = NULL;
		_metadata = ILRetain(new ILMap());
		
		_payloadKeys = ILRetain(new ILList());
		_payloadContents = ILRetain(new ILList());
		_payloadLenghts = ILRetain(new ILList());
	}
	
	StreamEncoder::~StreamEncoder() {
		ILRelease(_metadata);

		ILRelease(_payloadKeys);
		ILRelease(_payloadContents);
		ILRelease(_payloadLenghts);
	}
	
	
	StreamEncoderDelegate* StreamEncoder::delegate() {
		return _delegate;
	}
	
	void StreamEncoder::setDelegate(StreamEncoderDelegate* d) {
		_delegate = d;
	}
	
	bool StreamEncoder::hasStartedProducingStream() {
		return _sealed;
	}
	
	
	void StreamEncoder::setValueForMetadataKey(ILString* key, ILString* value) {
		if (_sealed)
			ILAbort("Cannot modify a stream encoder after it has started producing a stream.");
		
		_metadata->setValueForKey(key, value);
	}
	
	void StreamEncoder::addPayloadWithData(ILString* key, ILData* data) {
		if (_sealed)
			ILAbort("Cannot modify a stream encoder after it has started producing a stream.");
		
		_payloadKeys->addObject(key);
		_payloadContents->addObject(data);
		_payloadLenghts->addObject(new ILNumber((uint64_t) data->length()));
	}
	
	void StreamEncoder::addPayloadWithContentsOfStream(ILString* key, ILStreamSource* source, uint64_t fileSize) {
		if (_sealed)
			ILAbort("Cannot modify a stream encoder after it has started producing a stream.");
		
		_payloadKeys->addObject(key);
		_payloadContents->addObject(source);
		_payloadLenghts->addObject(new ILNumber(fileSize));
	}

	
	void StreamEncoder::produceStreamPart() {
		_sealed = true;
		
		this->setupPayloadMetadata();
		
		if (this->delegate()) {
			this->delegate()->streamEncoderWillProduceStream(this);
			this->delegate()->streamEncoderDidProduceData(this, new ILData((uint8_t*) kMvrStreamStartingBytes, kMvrStreamStartingBytesLength));
		}
				
		ILData* metadataKeys = new ILData();
		uint8_t zero = 0;
		
		ILMapIterator* i = _metadata->iterate();
		ILObject* keyObject, * valueObject;
		while (i->getNext(&keyObject, &valueObject)) {
			ILString* key = ILAs(ILString, keyObject),
				* value = ILAs(ILString, valueObject);
						
			metadataKeys->appendData(key->dataUsingEncoding(kILStringEncodingUTF8));
			metadataKeys->appendBytes(&zero, 1);
			metadataKeys->appendData(value->dataUsingEncoding(kILStringEncodingUTF8));
			metadataKeys->appendBytes(&zero, 1);
		}
		
		metadataKeys->appendBytes(&zero, 1);
		
		if (this->delegate())
			this->delegate()->streamEncoderDidProduceData(this, metadataKeys);
		
		_currentPayloadIndex = 0;
		_currentStream = NULL;
		this->producePayloadPart();
	}
	
	void StreamEncoder::producePayloadPart() {
		while (_currentPayloadIndex < _payloadKeys->count()) {
			ILObject* content = _payloadContents->objectAtIndex(_currentPayloadIndex);
			if (content->classIdentity() == ILDataClassIdentity) {
				
				if (this->delegate())
					this->delegate()->streamEncoderDidProduceData(this, ILAs(ILData, content));
				
			} else {
#error Produce parts from stream.
			}
		}
		
		if (this->delegate())
			this->delegate()->streamEncoderDidEndProducingStream(kMvrStreamEncoderDidEndCorrectly);
	}
	
	void StreamEncoder::setupPayloadMetadata() {
		ILString* keys = new ILString(), * stops = new ILString();
		
		ILSize count = _payloadKeys->count();
		uint8_t i; for (i = 0; i < count; i++) {
			ILString* key = ILAs(ILString, (_payloadKeys->objectAtIndex(i)));
			ILNumber* length = ILAs(ILNumber, (_payloadLenghts->objectAtIndex(i)));
			
			if (i == 0) {
				keys->appendString(key);
				stops->appendString(ILString::stringWithFormat(ILStr("%llu"), (unsigned long long) length->unsignedIntegerValue()));
			} else {
				keys->appendString(ILString::stringWithFormat(ILStr(" %s"), key->UTF8String()));
				stops->appendString(ILString::stringWithFormat(ILStr(" %llu"), (unsigned long long) length->unsignedIntegerValue()));
			}
		}
		
		_metadata->setValueForKey(ILStr("Payload-Keys"), keys);
		_metadata->setValueForKey(ILStr("Payload-Stops"), stops);
	}
	
}
