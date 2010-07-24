/*
 *  MoverCoreTests-StreamEncodeDecode.h
 *  MoverCore
 *
 *  Created by ∞ on 21/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

// Disabled, pending a better ILStream impl.
#if 0

#ifndef MOVER_CORE_TESTS_STREAM_ENCODE_DECODE
#define MOVER_CORE_TESTS_STREAM_ENCODE_DECODE 1

#include "PlatformCore.h"
#include "ILTesting.h"

#include "StreamEncoder.h"
#include "StreamDecoder.h"

namespace Mover {
	class StreamEncodeDecodeTests : public ILTesting::TestCase, public StreamEncoderDelegate, public StreamDecoderDelegate {
		ILTestCase(StreamEncodeDecodeTests) {
			ILTestWith(testSimpleDecoding);
            ILTestWith(testSimpleEncoding);
            ILTestWith(testEncodingWithStreams);
		}
		
		void testSimpleDecoding();
        void testSimpleEncoding();
        void testEncodingWithStreams();
        
		// delegate methods
		virtual void streamEncoderWillBeginProducingStream(StreamEncoder* e);
		virtual void streamEncoderDidProduceStreamPart(StreamEncoder* e, ILData* part);
		virtual void streamEncoderDidEndProducingStream(StreamEncoder* e);
		
		virtual void streamDecoderDidStartReceiving(StreamDecoder* decoder);
		
		virtual void streamDecoderDidReceiveMetadataPair(StreamDecoder* decoder, ILString* key, ILString* value);
		
		virtual void streamDecoderWillBeginReceivingPayloads(ILList* orderedKeys, ILList* orderedStops);
		
		virtual void streamDecoderWillBeginReceivingPayload(StreamDecoder* decoder, ILString* payloadKey, unsigned long long payloadSize);
		virtual void streamDecoderDidReceivePayloadPart(StreamDecoder* decoder, ILString* payloadKey, ILData* part);
		
		virtual void streamDecoderDidReset(StreamDecoder* decoder, StreamDecoderResetCause cause, StreamDecoderError error);
		
		virtual void setUp();
		virtual void tearDown();
		virtual void setUpClass();
		
	private:
        // Decoding stuff
        
		StreamDecoderResetCause _decoderResetCause;
		StreamDecoderError _decoderError;
		
		bool _didEndDecoding;
		
		ILMap* _decodedMetadata;
		ILMap* _decodedPayloads;
		ILList* _decodedPayloadKeys;
		ILList* _decodedPayloadStops;
		ILMap* _decodedPayloadSizes;
        
        // Encoding stuff
        
        ILData* _encodedStream;
        bool _didEndEncoding, _didStartEncoding;
	};
}

#endif // #ifndef MOVER_CORE_TESTS_ILSTREAM

#endif
