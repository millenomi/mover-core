/*
 *  StreamEncoder.h
 *  MoverCore
 *
 *  Created by ∞ on 28/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef StreamEncoder_H
#define StreamEncoder_H 1

#include "PlatformCore.h"
#include "ILStream.h"

namespace Mover {
	struct StreamEncoderImpl;
	class StreamEncoderDelegate;
	
	// ~ StreamEncoder ~

	class StreamEncoder : public ILObject {
	public:
		StreamEncoder(); virtual ~StreamEncoder();
		
// ---------- Editing the stream's contents before sending
		void setMetadataValueForKey(ILString* key, ILString* value);
		
		void addPayloadWithData(ILString* key, ILData* content);
		void addPayloadWithStreamSource(ILString* key, ILStreamSource* source, ILSize size);
		
		ILProperty(StreamEncoderDelegate*, delegate, setDelegate);
		
// ---------- Producing the stream
		void requestStreamPart();
		
		virtual void* classIdentity();
		
		// private past this point
		void performProducingStreamPart(ILMessage* m);
	private:
		friend class StreamEncoderImpl;
		StreamEncoderImpl* _i;
	};

	extern void* kStreamEncoderClassIdentity;

	
	class StreamEncoderDelegate {
	public:
		virtual void streamEncoderWillBegin(StreamEncoder* e) = 0;
		virtual void streamEncoderDidProducePart(StreamEncoder* e, ILData* d) = 0;
		virtual void streamEncoderDidEnd(StreamEncoder* e, uint32_t endCause) = 0;
	};
}

#endif // #ifndef StreamEncoder_H