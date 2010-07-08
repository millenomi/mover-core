/*
 *  ConsumptionQueue.h
 *  MoverCore
 *
 *  Created by ∞ on 02/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "PlatformCore.h"

#ifndef MOVER_CONSUMPTION_QUEUE_H
#define MOVER_CONSUMPTION_QUEUE_H 1

namespace Mover {
	class ConsumptionQueue : public ILObject {
	public:
		ConsumptionQueue();
		~ConsumptionQueue();
		
		void appendData(ILData* data);
		void appendDataNoCopy(ILData* data);
		void clear();
		size_t length();
		
        // Opportunistic reading
        // Gets data as an ILData* object. Object may only be valid up to the point where you dequeue (copy if needed past dequeue...; same rules as removing items from ILLists).
        ILData* availableDataWithMaximumLength(size_t length);
        
        // Gets raw data. Data remains valid up to next dequeue... call.
        void getAvailableDataWithMaximumLength(size_t length, void** bytes, size_t* actualLength);
        
        // If true, data up to specified 'length' bytes can be obtained using opportunistic reading; false otherwise.
        size_t availableDataLength();
        
        // Cursor advancement. Moves the cursor 'length' bytes ahead so new calls to reading methods will start returning data from there on. (Reading methods never advance the reading cursor on their own.)
        void dequeueDataOfLength(size_t length);
        
        // Portion reading (not opportunistic, may copy if needed).
		// If 'length' is greater than the remaining length of the data in the queue, all the data in the queue will be returned.
        ILData* dataWithLength(size_t length);
		
		// TODO should it be here?
		// Returns the offset from the current top of the queue of the first byte with the given value.
		// For example, eiven { 'H', 'e', 'l', 'l', 'o', \0 }, offsetForFirstByteWithValue(0) will return 5. If you dequeue that many bytes, \0 will be at the top of the queue; and if you retrieve that many bytes (eg via dataWithLength), you will retrieve up to the \0, not included.
		// If not found, returns ILNotFound.
		size_t offsetForFirstByteWithValue(uint8_t value);
		
	private:
		ILList* _queue;
		size_t _topOffset;
        ILData* topOfQueue();
	};
}

#endif // #ifndef MOVER_CONSUMPTION_QUEUE_H