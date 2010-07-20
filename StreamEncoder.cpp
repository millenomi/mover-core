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
	
	ILTargetForMethod(StreamEncoder_ReadyTarget, StreamEncoder, streamReady);
	
	StreamEncoder::StreamEncoder() {
		_sealed = false;
		_state = kMvrStreamEncoderEditable;
		
		_delegate = NULL;
		
		_metadata = ILRetain(new ILMap());
		
		_payloadKeys = ILRetain(new ILList());
		_payloadContents = ILRetain(new ILList());
		_payloadLenghts = ILRetain(new ILList());
		
		_currentStream = NULL;
		_currentPayloadIndex = 0;
		_didProvideMetadata = false;
		_didProvidePrologue = false;
		_didAnnounceEnd = false;
		
		_readyTarget = ILRetain(new StreamEncoder_ReadyTarget(this));
	}
	
	StreamEncoder::~StreamEncoder() {
		ILRelease(_metadata);

		ILRelease(_payloadKeys);
		ILRelease(_payloadContents);
		ILRelease(_payloadLenghts);
		
		ILRelease(_currentStream);
		ILRelease(_readyTarget);
	}
	
	StreamEncoderState StreamEncoder::state() {
		if (!_sealed)
			return kMvrStreamEncoderEditable;
		else
			return _state;
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

	// OK, the methods below are a bit of a mess, so here's how they work:
	// CONTRACT: The user calls requestStreamPart(). SOMETIME IN THE FUTURE (possibly during the rSP() call itself!), the delegate receives appropriate messages but NO MORE THAN ONE streamEncoderDidProduceStreamPart(). The delegate can request more streamEncoderDidProduceStreamPart() calls by calling again requestStreamPart().
	// So here's how the implementation works:
	
#error TODO advance to next payload on stream read end.
	
	// rSP() ->
	//	if the user wasn't given the prologue or the metadata, provide the appropriate block of data and return. State on return is kMvrStreamEncoderReadyToProduceStreamPart.
	//	otherwise:
	//		are we waiting for a stream payload?
	//		yes: check if there's data to get, or, if none, reenable monitoring on the stream.
	//		no: get a new payload
	//			is it a ILData payload? provide it all
	//			is it a ILStreamSource payload?
	//				open it
	//				monitor it
	//				go to state kMvrStreamEncoderProducingStreamPart
	//					SOMETIME IN THE FUTURE: the stream is ready to be read or has an error.
	//					if error: close, end.
	//					if readable, try to read.
	//						if can read (and not one of the cases below), provide data, return to kMvrStreamEncoderReadyToProduceStreamPart, wait for next call to rSP().
	//						if EOF and we're fine with the size as provided by the user, then we're STILL waiting for a piece of data, so we advance to the next payload and call rSP ourselves.
	//						if EOF and not enough data, or if too much data, end with error.
	
	void StreamEncoder::requestStreamPart() {
		if (_state == kMvrStreamEncoderProducingStreamPart)
			return; // already producing one
		
		if (_didAnnounceEnd)
			return; // we finished already
		
		if (!_didProvidePrologue && !_didProvideMetadata) {
			_sealed = true;
			_state = kMvrStreamEncoderReadyToProduceStreamPart;
			if (this->delegate())
				this->delegate()->streamEncoderWillBeginProducingStream(this);
			_setupPayloadMetadata();
		}
		
		if (!_didProvidePrologue) {
			_didProvidePrologue = true;
			if (this->delegate())
				this->delegate()->streamEncoderDidProduceStreamPart(this, new ILData((uint8_t*) kMvrStreamStartingBytes, kMvrStreamStartingBytesLength));
			return;
		}
		
		if (!_didProvideMetadata) {
			_didProvideMetadata = true;
			ILData* encodedMetadata = new ILData();
			uint8_t zero = 0;
			
			ILMapIterator* i = _metadata->iterate();
			ILObject* keyObject, * valueObject;
			while (i->getNext(&keyObject, &valueObject)) {
				ILString* key = ILAs(ILString, keyObject),
					* value = ILAs(ILString, valueObject);
							
				encodedMetadata->appendData(key->dataUsingEncoding(kILStringEncodingUTF8));
				encodedMetadata->appendBytes(&zero, 1);
				encodedMetadata->appendData(value->dataUsingEncoding(kILStringEncodingUTF8));
				encodedMetadata->appendBytes(&zero, 1);
			}
			
			encodedMetadata->appendBytes(&zero, 1);
			
			if (this->delegate())
				this->delegate()->streamEncoderDidProduceStreamPart(this, encodedMetadata);
			return;
		}
		
		if (_currentPayloadIndex >= _payloadContents->count()) {
			_didAnnounceEnd = true;
			if (this->delegate())
				this->delegate()->streamEncoderDidEndProducingStream(this);
			return;
		}
		
		if (_currentStream) {
			if (_currentStream->isReadyForReading())
				_readDataFromStreamAndInformDelegate();
			else
				_currentStream->beginMonitoring();
			return;
		}
		
		ILObject* o = _payloadContents->objectAtIndex(_currentPayloadIndex);
		
		if (o->classIdentity() == ILDataClassIdentity) {
			ILData* d = ILAs(ILData, o);
			
			if (this->delegate())
				this->delegate()->streamEncoderDidProduceStreamPart(this, d);
			return;
		}
		
		// If we're here, then we just encountered an unopened stream source! let's get going!
		_state = kMvrStreamEncoderProducingStreamPart;
		ILStreamSource* source = ILAs(ILStreamSource, o);
		
		_currentStream = ILRetain(source->open());
		_readFromCurrentStream = 0;
		
		ILMessageHub::current()->addTargetForMessagesOfKind(_readyTarget, kILStreamNowReadyForReadingMessage, _currentStream);
		ILMessageHub::current()->addTargetForMessagesOfKind(_readyTarget, kILStreamDidCloseWithErrorMessage, _currentStream);
		
		_currentStream->beginMonitoring();
	}
	
	void StreamEncoder::streamReady(ILMessage* m) {
		if (m->kind() == kILStreamDidCloseWithErrorMessage) {
			_endWithState(kMvrStreamEncoderStreamDidCloseWithError);
			return;
		}
		
		_readDataFromStreamAndInformDelegate();
	}
						  
	void StreamEncoder::_endWithState(StreamEncoderState s) {
		
		_closeStream();
		
		if (!_didAnnounceEnd) {
			_state = s;
			_didAnnounceEnd = true;
			if (this->delegate())
				this->delegate()->streamEncoderDidEndProducingStream(this);
		}
		
	}
	
	void StreamEncoder::_closeStream() {
		ILMessageHub::current()->removeTargetForMessagesOfKind(_readyTarget, kILStreamNowReadyForReadingMessage);
		ILMessageHub::current()->removeTargetForMessagesOfKind(_readyTarget, kILStreamDidCloseWithErrorMessage);

		_currentStream->close();
		ILRelease(_currentStream); _currentStream = NULL;
	}
	
	void StreamEncoder::_readDataFromStreamAndInformDelegate() {
		ILData* d = _currentStream->readDataOfMaximumLength(524288 /* 0.5 MB */);
		uint64_t len = _payloadLenghts->at<ILNumber>(_currentPayloadIndex)->unsignedIntegerValue();
		
		if (!d) {
			if (len == _readFromCurrentStream) {
				_closeStream();
				this->requestStreamPart();
			} else {
				_endWithState(kMvrStreamEncoderStreamIsOfIncorrectSizeError);
			}
			
			return;
		}
		
		if (_readFromCurrentStream + d->length() > len) {
			_endWithState(kMvrStreamEncoderStreamIsOfIncorrectSizeError);
			return;
		}
		
		_readFromCurrentStream += d->length();
		
		if (this->delegate())
			this->delegate()->streamEncoderDidProduceStreamPart(this, d);
		
		_currentStream->endMonitoring();
	}
	
	
	void StreamEncoder::_setupPayloadMetadata() {
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
	
	
	StreamEncoderDelegate* StreamEncoder::delegate() {
		return _delegate;
	}
	
	void StreamEncoder::setDelegate(StreamEncoderDelegate* d) {
		_delegate = d;
	}
}
