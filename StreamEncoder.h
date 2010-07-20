/*
 *  StreamEncoder.h
 *  MoverCore
 *
 *  Created by ∞ on 18/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "PlatformCore.h"

#ifndef MOVER_STREAM_ENCODER_H
#define MOVER_STREAM_ENCODER_H 1

#include "ILStream.h"

namespace Mover {
	class StreamEncoderDelegate;
	
	typedef enum {
		kMvrStreamEncoderDidEndCorrectly = 0,
		kMvrStreamEncoderNotEnoughDataInStreamError,
	} StreamEncoderEndCause;
	
	class StreamEncoder : public ILObject {
	public:
		StreamEncoder();
		~StreamEncoder();
		
		void setValueForMetadataKey(ILString* key, ILString* value);
		void addPayloadWithData(ILString* key, ILData* data);
		void addPayloadWithContentsOfStream(ILString* key, ILStreamSource* source, uint64_t fileSize);
		
		StreamEncoderDelegate* delegate();
		void setDelegate(StreamEncoderDelegate* delegate);
		
		void produceStreamPart();
		bool hasStartedProducingStream();
		
	private:
		StreamEncoderDelegate* _delegate;
		
		ILMap* _metadata;
		ILList* _payloadKeys, * _payloadContents, * _payloadLenghts;
		
		bool _sealed;
		
		void setupPayloadMetadata();
		
		ILIndex _currentPayloadIndex;
		ILStream* _currentStream;
		void producePayloadPart();
	};
	
	class StreamEncoderDelegate {
	public:
		virtual void streamEncoderWillProduceStream(StreamEncoder* encoder) = 0;
		virtual void streamEncoderDidProduceData(StreamEncoder* encoder, ILData* data) = 0;
		virtual void streamEncoderDidEndProducingStream(StreamEncoderEndCause cause) = 0;
	};
	
	extern void* kILStreamEncoderCanProceedMessage;
}

#endif // #ifndef MOVER_STREAM_ENCODER_H
