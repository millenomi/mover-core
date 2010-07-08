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
		
		this->setDelegate(delegate);		
	}
	
	StreamDecoder::~StreamDecoder() {
		ILRelease(_queue);
		ILRelease(_receivedKeys);
		ILRelease(_lastItemKey);
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
	}
	
	bool StreamDecoder::isExpectingNewStream() {
		return _state == kMvrStreamDecoderExpectingStart;
	}
	
	void StreamDecoder::appendData(ILData* data) {
		_queue->appendData(data);
	}
	
	
	float StreamDecoder::progress() {
#warning TODO
		return kMvrIndeterminateProgress;
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
					
#warning TODO
                    
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
				_state = kMvrStreamDecoderExpectingBody;
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
		
		ILRelease(_lastItemKey);
		_lastItemKey = ILRetain(itemKey);
		
		_queue->dequeueDataOfLength(index + 1); // consume title and \0
		
		return true;
	}
	
// --- Handler for metadata item value parsing.
	bool StreamDecoder::consumeMetadataItemValue() {
		size_t index = _queue->offsetForFirstByteWithValue(0);
		if (index == ILNotFound)
			return false;
		
		ILData* itemValueData = _queue->dataWithLength(index);
		ILString* itemValue = ILString::stringWithData(itemValueData, kILStringEncodingUTF8);
		
		if (!itemValue) {
			this->resetWithCause(kMvrStreamDecoderDidFindError, kMvrStreamDecoderErrorKeyOrValueNotValidUTF8);
			return false;
		}
		
		if (_lastItemKey->equals(ILStr("Payload-Keys")) || _lastItemKey->equals(ILStr("Payload-Stops"))) {
#warning TODO parse Payload-Keys and Payload-Stops
		}
		
		if (this->delegate())
			this->delegate()->streamDecoderDidReceiveMetadataPair(this, _lastItemKey, itemValue);
		
		_state = kMvrStreamDecoderExpectingMetadataItemKey;
		return true;
	}
	
	bool StreamDecoder::canProceedToBodyFromMetadata() {
#warning TODO
		return false;
	}

}

