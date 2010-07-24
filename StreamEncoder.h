/*
 *  StreamEncoder.h
 *  MoverCore
 *
 *  Created by ∞ on 18/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

// Disabled, pending a better ILStream impl.
#if 0

#include "PlatformCore.h"

#ifndef MOVER_STREAM_ENCODER_H
#define MOVER_STREAM_ENCODER_H 1

#include "ILStream.h"
#include "ILRunLoop.h"
#include "ILMessage.h"
#include "ILMessageHub.h"

namespace Mover {
	class StreamEncoder_ReadyTarget;
    class StreamEncoder_ActuallyProvideStreamPartTarget;
	class StreamEncoderDelegate;
	
	typedef enum {
		kMvrStreamEncoderEditable,
		kMvrStreamEncoderReadyToProduceStreamPart,
		kMvrStreamEncoderProducingStreamPart,
		kMvrStreamEncoderDidEndCorrectly,
		kMvrStreamEncoderStreamIsOfIncorrectSizeError,
		kMvrStreamEncoderStreamDidCloseWithError,
	} StreamEncoderState;
	
	class StreamEncoder : public ILObject {
	public:
		StreamEncoder();
		~StreamEncoder();
		
		StreamEncoderDelegate* delegate();
		void setDelegate(StreamEncoderDelegate* d);
		
		void setValueForMetadataKey(ILString* key, ILString* value);
		void addPayloadWithData(ILString* key, ILData* data);
		void addPayloadWithContentsOfStream(ILString* key, ILStreamSource* source, uint64_t fileSize);
		
		void requestStreamPart();
		StreamEncoderState state();
		
	friend class StreamEncoder_ReadyTarget;
    friend class StreamEncoder_ActuallyProvideStreamPartTarget;
	private:
        void actuallyProvideStreamPart(ILMessage* m);
		void streamReady(ILMessage* m);
		
		StreamEncoderDelegate* _delegate;
		
		ILMap* _metadata;
		ILList* _payloadKeys, * _payloadContents, * _payloadLenghts;
		
		bool _sealed;
		StreamEncoderState _state;
		
		void _setupPayloadMetadata();
		void _readDataFromStreamAndInformDelegate();
		void _endWithState(StreamEncoderState e);
		void _closeStream();
		
		bool _didProvidePrologue;
		bool _didProvideMetadata;

		ILIndex _currentPayloadIndex;
		ILStream* _currentStream;
		uint64_t _readFromCurrentStream;
				
		StreamEncoder_ReadyTarget* _readyTarget;
        StreamEncoder_ActuallyProvideStreamPartTarget* _provideStreamTarget;
		
		bool _didAnnounceEnd;
	};
	
	class StreamEncoderDelegate {
	public:
		virtual void streamEncoderWillBeginProducingStream(StreamEncoder* e) = 0;
		virtual void streamEncoderDidProduceStreamPart(StreamEncoder* e, ILData* part) = 0;
		virtual void streamEncoderDidEndProducingStream(StreamEncoder* e) = 0;
	};
}

#endif // #ifndef MOVER_STREAM_ENCODER_H

#endif