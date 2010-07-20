/*
 *  StreamDecoder.h
 *  MoverCore
 *
 *  Created by ∞ on 02/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "PlatformCore.h"

#ifndef MOVER_STREAM_DECODER_H
#define MOVER_STREAM_DECODER_H 1

namespace Mover {
		
	typedef enum {
		// An entire stream was decoded without errors.
		kMvrStreamDecoderDidFinishDecoding,
		// An error was found, so stream decoding was reset.
		kMvrStreamDecoderDidFindError,
		// ->reset() was called on the stream decoder.
		kMvrStreamDecoderProgrammaticReset,
	} StreamDecoderResetCause;
	
    typedef enum {
        kMvrStreamDecoderNoError,
        kMvrStreamDecoderErrorDidNotFindStart,
        kMvrStreamDecoderErrorDidNotFindValidKey,
        kMvrStreamDecoderErrorDidNotFindValidValue,
        kMvrStreamDecoderErrorKeyOrValueNotValidUTF8,
        kMvrStreamDecoderErrorDidFindDuplicateKey,
        kMvrStreamDecoderErrorDidFindDuplicatePayloadKey,
        kMvrStreamDecoderErrorPayloadKeysMismatch,
        kMvrStreamDecoderErrorPayloadStopsInvalid,
		kMvrStreamDecoderErrorPayloadMetadataIncompleteOrMissing,
    } StreamDecoderError;

	class StreamDecoderDelegate;
	class ConsumptionQueue;
	
	
// ------- THE STREAM DECODER PROPER
	class StreamDecoder : public ILObject {
	public:
		StreamDecoder(StreamDecoderDelegate* delegate = NULL);
		~StreamDecoder();
		
		// does not retain.
		void setDelegate(StreamDecoderDelegate* delegate);
		StreamDecoderDelegate* delegate();
		
		void reset();
		bool isExpectingNewStream();
		
		void appendData(ILData* data);
		void processAppendedData();
		
	private:
		StreamDecoderDelegate* _delegate;
		ConsumptionQueue* _queue;
		int _state;
		bool consumeStreamStart();
		void resetWithCause(StreamDecoderResetCause cause, StreamDecoderError error = kMvrStreamDecoderNoError);
		
		bool consumeMetadataItemKey();
		bool consumeMetadataItemValue();
		bool canProceedToBodyFromMetadata();
		
		void moveToExpectingBodyState();
		bool consumeBody();
		
		ILString* _lastItemKey;
		ILSet* _receivedKeys;
		
		ILList* _receivedPayloadKeys;
		ILList* _receivedPayloadStops;
		
		uint64_t _currentPayloadIndex;
		int64_t _remainingPayloadLength;
		bool _hasAnnouncedCurrentPayload;
	};
	

// ------- THE STREAM DECODER DELEGATE INTERFACE
	class StreamDecoderDelegate {
	public:
		virtual void streamDecoderDidStartReceiving(StreamDecoder* decoder) = 0;

		virtual void streamDecoderDidReceiveMetadataPair(StreamDecoder* decoder, ILString* key, ILString* value) = 0;
		
		virtual void streamDecoderWillBeginReceivingPayloads(ILList* orderedKeys, ILList* orderedStops) = 0;
		
		virtual void streamDecoderWillBeginReceivingPayload(StreamDecoder* decoder, ILString* payloadKey, unsigned long long payloadSize) = 0;
		virtual void streamDecoderDidReceivePayloadPart(StreamDecoder* decoder, ILString* payloadKey, ILData* part) = 0;
		
		virtual void streamDecoderDidReset(StreamDecoder* decoder, StreamDecoderResetCause cause, StreamDecoderError error) = 0;
	};
	
}

#endif // #ifndef MOVER_STREAM_DECODER_H
