/*
 *  StreamDecoder.cpp
 *  MoverCore
 *
 *  Created by ∞ on 02/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "StreamDecoder.h"
#include "Protocol.h"
#include "ConsumptionQueue.h"

#include <cstring>

namespace Mover {

	// These are the states of the 'state machine' that in theory drives the decoder. The current state is in this->_state.
	enum {	
		kMvrStreamDecoderExpectingStart,
		kMvrStreamDecoderExpectingMetadataItemKey,
		kMvrStreamDecoderExpectingMetadataItemValue,
		kMvrStreamDecoderExpectingBody,
		
		kMvrStreamDecoderStartingState = kMvrStreamDecoderExpectingStart,
	};	
	
// ----------- ADMINISTRIVIA
	StreamDecoder::StreamDecoder(StreamDecoderDelegate* delegate) {
		_state = kMvrStreamDecoderStartingState;
		_queue = ILRetain(new ConsumptionQueue());
		_lastItemKey = NULL;
		_receivedKeys = ILRetain(new ILSet());
		_receivedPayloadKeys = NULL;
		_receivedPayloadStops = NULL;
		
		this->setDelegate(delegate);		
	}
	
	StreamDecoder::~StreamDecoder() {
		ILRelease(_queue);
		ILRelease(_receivedKeys);
		ILRelease(_lastItemKey);
		ILRelease(_receivedPayloadKeys);
		ILRelease(_receivedPayloadStops);
	}
	
	
	void StreamDecoder::setDelegate(StreamDecoderDelegate* delegate) {
		_delegate = delegate;
	}
	StreamDecoderDelegate* StreamDecoder::delegate() {
		return _delegate;
	}
	
// ----------- DATA MANAGEMENT & PROGRESS
	void StreamDecoder::reset() {
		this->resetWithCause(kMvrStreamDecoderProgrammaticReset);
	}
	
	void StreamDecoder::resetWithCause(StreamDecoderResetCause cause, StreamDecoderError error) {
		if (this->delegate())
			this->delegate()->streamDecoderDidReset(this, cause, error);
		
		_queue->clear();
		_receivedKeys->removeAllObjects();
		_state = kMvrStreamDecoderExpectingStart;
		ILRelease(_receivedPayloadKeys); _receivedPayloadKeys = NULL;
		ILRelease(_receivedPayloadStops); _receivedPayloadStops = NULL;
	}
	
	bool StreamDecoder::isExpectingNewStream() {
		return _state == kMvrStreamDecoderExpectingStart;
	}
	
	void StreamDecoder::appendData(ILData* data) {
		_queue->appendData(data);
	}
	
	
// ----------- THE CORE OF IT
	void StreamDecoder::processAppendedData() {
		bool canContinueDecoding = true;
		
		while (canContinueDecoding && _queue->length() > 0) {
			switch (_state) {
				case kMvrStreamDecoderExpectingStart:
					canContinueDecoding = this->consumeStreamStart();
					break;
					
				case kMvrStreamDecoderExpectingMetadataItemKey:
					canContinueDecoding = this->consumeMetadataItemKey();
					break;
					
				case kMvrStreamDecoderExpectingMetadataItemValue:
					canContinueDecoding = this->consumeMetadataItemValue();
					break;
					
				case kMvrStreamDecoderExpectingBody:
					canContinueDecoding = this->consumeBody();
					break;
                    
				default:
					ILAbort("Stream decoder entered an unknown state");
					return;
			}
		}
	}
	
// --- Handler for magic number parsing. (The magic number constants are in Protocol.h)
	bool StreamDecoder::consumeStreamStart() {
		if (_queue->length() < kMvrStreamStartingBytesLength)
			return false;
		
		ILData* top = _queue->dataWithLength(kMvrStreamStartingBytesLength);
		if (memcmp(top->bytes(), kMvrStreamStartingBytes, kMvrStreamStartingBytesLength) == 0) {
			_queue->dequeueDataOfLength(kMvrStreamStartingBytesLength);
			_state = kMvrStreamDecoderExpectingMetadataItemKey;
			
			if (this->delegate())
				this->delegate()->streamDecoderDidStartReceiving(this);
			
			return true;
			
		} else {
			this->resetWithCause(kMvrStreamDecoderDidFindError);
			return false;
		}
	}

// --- Handler for metadata item title parsing.
	bool StreamDecoder::consumeMetadataItemKey() {
		size_t index = _queue->offsetForFirstByteWithValue(0);
		
		if (index == ILNotFound)
			return false;
		
		if (index == 0) {
			if (this->canProceedToBodyFromMetadata()) {
				_queue->dequeueDataOfLength(1); // eat the \0
				this->moveToExpectingBodyState();
				return true;
			} else {
				this->resetWithCause(kMvrStreamDecoderDidFindError, kMvrStreamDecoderErrorPayloadMetadataIncompleteOrMissing);
				return false;
			}
		}
			
		// 0..index - 1 ('index' bytes long) contains the title.
		ILData* itemKeyData = _queue->dataWithLength(index);
		ILString* itemKey = ILString::stringWithData(itemKeyData, kILStringEncodingUTF8);
		
		if (!itemKey) {
			this->resetWithCause(kMvrStreamDecoderDidFindError, kMvrStreamDecoderErrorKeyOrValueNotValidUTF8);
			return false;
		}
		
		if (_receivedKeys->containsObject(itemKey)) {
			this->resetWithCause(kMvrStreamDecoderDidFindError, kMvrStreamDecoderErrorDidFindDuplicateKey);
			return false;
		} 
		
		_receivedKeys->addObject(itemKey);
		
		ILRelease(_lastItemKey);
		_lastItemKey = ILRetain(itemKey);
		
		_queue->dequeueDataOfLength(index + 1); // consume title and \0

		_state = kMvrStreamDecoderExpectingMetadataItemValue;
		
		return true;
	}
	
// --- Handler for metadata item value parsing.
	bool StreamDecoder::consumeMetadataItemValue() {
		size_t index = _queue->offsetForFirstByteWithValue(0);
		if (index == ILNotFound)
			return false;
		
		ILData* itemValueData = _queue->dataWithLength(index);
		_queue->dequeueDataOfLength(index + 1);
		
		ILString* itemValue = ILString::stringWithData(itemValueData, kILStringEncodingUTF8);
		
		if (!itemValue) {
			this->resetWithCause(kMvrStreamDecoderDidFindError, kMvrStreamDecoderErrorKeyOrValueNotValidUTF8);
			return false;
		}
		
		if (_lastItemKey->equals(ILStr("Payload-Keys"))) {
			
			ILSet* s = new ILSet();
			ILList* keys = itemValue->componentsSeparatedByCharacter(' ');
			
			ILListIterator* i = keys->iterate();
			ILObject* o;
			while ((o = i->next())) {
				ILString* payloadKey = ILAs(ILString, o);
				
				if (s->containsObject(payloadKey)) {
					this->resetWithCause(kMvrStreamDecoderDidFindError, kMvrStreamDecoderErrorDidFindDuplicatePayloadKey);
					return false;
				}
			}
			
			_receivedPayloadKeys = ILRetain(keys->copy());
			
		} else if (_lastItemKey->equals(ILStr("Payload-Stops"))) {
			
			ILList* stops = itemValue->componentsSeparatedByCharacter(' ');
			ILList* numericStops = new ILList();
			
			int64_t lastStop = INT64_MIN;
			
			ILListIterator* i = stops->iterate();
			ILObject* o;
			while ((o = i->next())) {
				ILString* payloadStopStr = ILAs(ILString, o);
				int64_t payloadStop = payloadStopStr->integerValue();
				
				if (payloadStop < 0 || payloadStop < lastStop) {
					this->resetWithCause(kMvrStreamDecoderDidFindError, kMvrStreamDecoderErrorPayloadStopsInvalid);
					return false;
				}
				
				numericStops->addObject(new ILNumber(payloadStop));
				
				lastStop = payloadStop;
			}
			
			_receivedPayloadStops = ILRetain(numericStops);
			
		}
		
		if (this->delegate())
			this->delegate()->streamDecoderDidReceiveMetadataPair(this, _lastItemKey, itemValue);
		
		_state = kMvrStreamDecoderExpectingMetadataItemKey;
		return true;
	}
	
	bool StreamDecoder::canProceedToBodyFromMetadata() {
		
		return _receivedPayloadKeys && _receivedPayloadStops &&
			(_receivedPayloadKeys->count() == _receivedPayloadStops->count());
		
	}

	void StreamDecoder::moveToExpectingBodyState() {
		_currentPayloadIndex = 0;
		ILNumber* n = ILAs(ILNumber, _receivedPayloadStops->objectAtIndex(0));
		_remainingPayloadLength = n->integerValue();
		_hasAnnouncedCurrentPayload = false;
		
		_state = kMvrStreamDecoderExpectingBody;
		
		if (this->delegate())
			this->delegate()->streamDecoderWillBeginReceivingPayloads(_receivedPayloadKeys, _receivedPayloadStops);
	}
	
// --- Handler for body.
	bool StreamDecoder::consumeBody() {
		ILString* payloadKey = ILAs(ILString, _receivedPayloadKeys->objectAtIndex(_currentPayloadIndex));
		
		if (this->delegate() && !_hasAnnouncedCurrentPayload)
			this->delegate()->streamDecoderWillBeginReceivingPayload(this, payloadKey, _remainingPayloadLength);
		
		_hasAnnouncedCurrentPayload = true;

		ILData* d = _queue->availableDataWithMaximumLength(_remainingPayloadLength);
		
		if (!d)
			return false;
		
		_remainingPayloadLength -= d->length();
		_queue->dequeueDataOfLength(d->length());
		
		if (this->delegate())
			this->delegate()->streamDecoderDidReceivePayloadPart(this, payloadKey, d);
		
		if (_remainingPayloadLength == 0) {
			
			_currentPayloadIndex++;
			if (_currentPayloadIndex < _receivedPayloadKeys->count()) {
				
				ILNumber* prevStop = 
					ILAs(ILNumber, _receivedPayloadStops->objectAtIndex(_currentPayloadIndex - 1)),
					* currentStop = ILAs(ILNumber, _receivedPayloadStops->objectAtIndex(_currentPayloadIndex));

				
				_remainingPayloadLength = currentStop->integerValue() - prevStop->integerValue();
				_hasAnnouncedCurrentPayload = false;
				
				return true;
				
			} else {
				
				// whoa! we've finished!
				this->resetWithCause(kMvrStreamDecoderDidFinishDecoding);
				return false;
				
			}
		}
		
		return true;
	}

}


