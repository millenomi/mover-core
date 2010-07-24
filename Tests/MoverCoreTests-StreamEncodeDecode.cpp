/*
 *  MoverCoreTests-StreamEncodeDecode.cpp
 *  MoverCore
 *
 *  Created by ∞ on 21/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

// Disabled, pending a better ILStream impl.
#if 0

#include "MoverCoreTests-StreamEncodeDecode.h"

namespace Mover {
	
	void StreamEncodeDecodeTests::streamEncoderWillBeginProducingStream(StreamEncoder* e) {
        _didEndEncoding = false;
        _didStartEncoding = true;
    }
    
	void StreamEncodeDecodeTests::streamEncoderDidProduceStreamPart(StreamEncoder* e, ILData* part) {
        _encodedStream->appendData(part);
        e->requestStreamPart();
    }
    
	void StreamEncodeDecodeTests::streamEncoderDidEndProducingStream(StreamEncoder* e) {
        _didEndEncoding = true;
    }
	
	// ~~~
	
	void StreamEncodeDecodeTests::streamDecoderDidStartReceiving(StreamDecoder* decoder) {
	
	}
	
	void StreamEncodeDecodeTests::streamDecoderDidReceiveMetadataPair(StreamDecoder* decoder, ILString* key, ILString* value) {
		_decodedMetadata->setValueForKey(key, value);
	}
	
	void StreamEncodeDecodeTests::streamDecoderWillBeginReceivingPayloads(ILList* orderedKeys, ILList* orderedStops) {
		ILClear(_decodedPayloadKeys);
		ILClear(_decodedPayloadStops);
		
		_decodedPayloadKeys = ILRetain(orderedKeys->copy());
		_decodedPayloadStops = ILRetain(orderedStops->copy());
	}
	
	void StreamEncodeDecodeTests::streamDecoderWillBeginReceivingPayload(StreamDecoder* decoder, ILString* payloadKey, unsigned long long payloadSize) {
		
		_decodedPayloadSizes->setValueForKey(payloadKey, new ILNumber(payloadSize));
		
	}
	
	void StreamEncodeDecodeTests::streamDecoderDidReceivePayloadPart(StreamDecoder* decoder, ILString* payloadKey, ILData* part) {
		
		ILData* d = _decodedPayloads->at<ILData>(payloadKey);
		if (!d) {
			d = new ILData();
			_decodedPayloads->setValueForKey(payloadKey, d);
		}
		
		d->appendData(part);
	}
	
	void StreamEncodeDecodeTests::streamDecoderDidReset(StreamDecoder* decoder, StreamDecoderResetCause cause, StreamDecoderError error) {
		_didEndDecoding = true;
		_decoderError = error;
		_decoderResetCause = cause;
	}
	
	
	void StreamEncodeDecodeTests::setUpClass() {
		_decoderError = kMvrStreamDecoderNoError;
		
		_decodedMetadata = NULL;
		_decodedPayloads = NULL;
		_decodedPayloadKeys = NULL;
		_decodedPayloadStops = NULL;
		_decodedPayloadSizes = NULL;
        _encodedStream = NULL;
	}
	
	void StreamEncodeDecodeTests::setUp() {
		_didEndDecoding = false;
		_decodedMetadata = ILRetain(new ILMap());
		_decodedPayloads = ILRetain(new ILMap());
		_decodedPayloadSizes = ILRetain(new ILMap());
		_decodedPayloadStops = NULL;
		_decodedPayloadSizes = ILRetain(new ILMap());
        
        _didEndEncoding = false;
        _didStartEncoding = false;
        _encodedStream = ILRetain(new ILData());
	}
	
	void StreamEncodeDecodeTests::tearDown() {
		ILClear(_decodedMetadata);
		ILClear(_decodedPayloads);
		ILClear(_decodedPayloadKeys);
		ILClear(_decodedPayloadStops);
		ILClear(_decodedPayloadSizes);
        ILClear(_encodedStream);
	}
	
	void StreamEncodeDecodeTests::testSimpleDecoding() {
		const char* stream = "MOVR2a\0b\0c\0d\0Payload-Keys\0test rest\0Payload-Stops\0" "2 4\0\0HiHo";
		ILSize streamLength = 59;
		
		StreamDecoder* d = new StreamDecoder();
		d->setDelegate(this);
		
		d->appendData(new ILData((uint8_t*) stream, streamLength, kILDataNoCopy));
		d->processAppendedData();
		
		ILTestTrue(_didEndDecoding);
		if (_didEndDecoding) {
			ILTestEqualValues(_decoderResetCause, kMvrStreamDecoderDidFinishDecoding);
			ILTestEqualValues(_decoderError, kMvrStreamDecoderNoError);
			
			ILTestEqualObjects(_decodedMetadata, new ILMap(ILStr("a"), ILStr("b"),
														   ILStr("c"), ILStr("d"),
														   ILStr("Payload-Keys"), ILStr("test rest"),
														   ILStr("Payload-Stops"), ILStr("2 4"),
														   NULL));
			
			ILTestEqualObjects(_decodedPayloadKeys, new ILList(ILStr("test"), ILStr("rest"), NULL));
			
			ILTestEqualObjects(_decodedPayloadStops, new ILList(new ILNumber(2), new ILNumber(4), NULL));
            
            ILTestEqualObjects(_decodedPayloads->at<ILData>(ILStr("test")), ILStr("Hi")->dataUsingEncoding(kILStringEncodingUTF8));
            ILTestEqualObjects(_decodedPayloads->at<ILData>(ILStr("rest")), ILStr("Ho")->dataUsingEncoding(kILStringEncodingUTF8));
			
			ILTestEqualObjects(_decodedPayloadSizes, new ILMap(ILStr("test"), new ILNumber(2),
															   ILStr("rest"), new ILNumber(2),
															   NULL));
		}
	}
    
    void StreamEncodeDecodeTests::testSimpleEncoding() {
        StreamEncoder* e = new StreamEncoder();
        e->setDelegate(this);
        
        e->setValueForMetadataKey(ILStr("a"), ILStr("b"));
        e->setValueForMetadataKey(ILStr("c"), ILStr("d"));
        e->addPayloadWithData(ILStr("test"), ILStr("Hi")->dataUsingEncoding(kILStringEncodingUTF8));
        e->addPayloadWithData(ILStr("rest"), ILStr("Ho")->dataUsingEncoding(kILStringEncodingUTF8));
        
        e->requestStreamPart();
        while (!_didEndEncoding)
            ILRunLoop::current()->spinForAboutUpTo(0.05);
        
        ILTestEqualValues(e->state(), kMvrStreamEncoderDidEndCorrectly);
		
		StreamDecoder* d = new StreamDecoder();
		d->setDelegate(this);
		
		d->appendData(_encodedStream);
		d->processAppendedData();
		
		ILTestTrue(_didEndDecoding);
		if (_didEndDecoding) {
			ILTestEqualValues(_decoderResetCause, kMvrStreamDecoderDidFinishDecoding);
			ILTestEqualValues(_decoderError, kMvrStreamDecoderNoError);
			
			ILTestEqualObjects(_decodedMetadata, new ILMap(ILStr("a"), ILStr("b"),
														   ILStr("c"), ILStr("d"),
														   ILStr("Payload-Keys"), ILStr("test rest"),
														   ILStr("Payload-Stops"), ILStr("2 4"),
														   NULL));
			
			ILTestEqualObjects(_decodedPayloadKeys, new ILList(ILStr("test"), ILStr("rest"), NULL));
			
			ILTestEqualObjects(_decodedPayloadStops, new ILList(new ILNumber(2), new ILNumber(4), NULL));
            
            ILTestEqualObjects(_decodedPayloads->at<ILData>(ILStr("test")), ILStr("Hi")->dataUsingEncoding(kILStringEncodingUTF8));
            ILTestEqualObjects(_decodedPayloads->at<ILData>(ILStr("rest")), ILStr("Ho")->dataUsingEncoding(kILStringEncodingUTF8));            
			
			ILTestEqualObjects(_decodedPayloadSizes, new ILMap(ILStr("test"), new ILNumber(2),
															   ILStr("rest"), new ILNumber(2),
															   NULL));
        }
    }
    
    class StreamEncodeDecodeTests_PipeSource : public ILStreamSource {
    public:
        int fd;
        virtual ILStream* open() {
            return new ILStream(fd);
        }
    };
    
    void StreamEncodeDecodeTests::testEncodingWithStreams() {
        StreamEncoder* e = new StreamEncoder();
        e->setDelegate(this);
        
        e->setValueForMetadataKey(ILStr("a"), ILStr("b"));
        e->setValueForMetadataKey(ILStr("c"), ILStr("d"));
        
        int pipeFDs[2];
        bool pipeCreated = (pipe(pipeFDs) == 0);
        ILTestTrue(pipeCreated); if (!pipeCreated) return;
        
        StreamEncodeDecodeTests_PipeSource* x = new StreamEncodeDecodeTests_PipeSource();
        x->fd = pipeFDs[0];

        e->addPayloadWithContentsOfStream(ILStr("test"), x, 100 * 2);
        
        e->addPayloadWithData(ILStr("rest"), ILStr("Ho")->dataUsingEncoding(kILStringEncodingUTF8));

        e->requestStreamPart();
        
        int countOfHis = 0;
        
        while (!_didEndEncoding) {
            ILRunLoop::current()->spinForAboutUpTo(0.05);
            if (countOfHis < 100) {
                write(pipeFDs[1], (void*) "Hi", 2);
                countOfHis++;
            }
        }
        
        ILTestEqualValues(e->state(), kMvrStreamEncoderDidEndCorrectly);
		
		StreamDecoder* d = new StreamDecoder();
		d->setDelegate(this);
		
		d->appendData(_encodedStream);
		d->processAppendedData();
        
        ILData* aHundredHis = new ILData();
        for (countOfHis = 0; countOfHis < 100; countOfHis++) {
            aHundredHis->appendBytes((uint8_t*) "Hi", 2);
        }
		
		ILTestTrue(_didEndDecoding);
		if (_didEndDecoding) {
			ILTestEqualValues(_decoderResetCause, kMvrStreamDecoderDidFinishDecoding);
			ILTestEqualValues(_decoderError, kMvrStreamDecoderNoError);
			
			ILTestEqualObjects(_decodedMetadata, new ILMap(ILStr("a"), ILStr("b"),
														   ILStr("c"), ILStr("d"),
														   ILStr("Payload-Keys"), ILStr("test rest"),
														   ILStr("Payload-Stops"), ILStr("200 204"),
														   NULL));
			
			ILTestEqualObjects(_decodedPayloadKeys, new ILList(ILStr("test"), ILStr("rest"), NULL));
			
			ILTestEqualObjects(_decodedPayloadStops, new ILList(new ILNumber(200), new ILNumber(204), NULL));
            
            ILTestEqualObjects(_decodedPayloads->at<ILData>(ILStr("test")), aHundredHis);
            ILTestEqualObjects(_decodedPayloads->at<ILData>(ILStr("rest")), ILStr("Ho")->dataUsingEncoding(kILStringEncodingUTF8));            
			
			ILTestEqualObjects(_decodedPayloadSizes, new ILMap(ILStr("test"), new ILNumber(200),
															   ILStr("rest"), new ILNumber(2),
															   NULL));
        }
    }
}

#endif