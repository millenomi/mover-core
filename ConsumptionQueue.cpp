/*
 *  ConsumptionQueue.cpp
 *  MoverCore
 *
 *  Created by ∞ on 02/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ConsumptionQueue.h"
#include <cstring>

namespace Mover {
	ConsumptionQueue::ConsumptionQueue() {
		_queue = ILRetain(new ILList());
		_topOffset = 0;
	}
	
	ConsumptionQueue::~ConsumptionQueue() {
		ILRelease(_queue);
	}
	
	
	void ConsumptionQueue::appendData(ILData* data) {
		_queue->addObject(data->copy());
	}
	void ConsumptionQueue::appendDataNoCopy(ILData* data) {
		_queue->addObject(data);
	}
	
	void ConsumptionQueue::clear() {
		_queue->removeAllObjects();
		_topOffset = 0;
	}
	
	size_t ConsumptionQueue::length() {
		ILListIterator* i = _queue->iterate();
		ILData* d;
		size_t len = 0;
		
		while ((d = static_cast<ILData*>(i->next())))
			len += d->length();
		
		return len - _topOffset;
	}
    
    ILData* ConsumptionQueue::topOfQueue() {
        if (_queue->count() == 0)
            return NULL;
        
        return static_cast<ILData*>(_queue->objectAtIndex(0));
    }
    
    
    // Opportunistic reading & cursor.
    
    void ConsumptionQueue::getAvailableDataWithMaximumLength(size_t length, void **bytes, size_t *actualLength) {
        
        ILData* top = this->topOfQueue();
        if (!top) {
            *bytes = NULL;
            *actualLength = 0;
            return;
        }
        
        void* theBytes = top->bytes() + _topOffset;
        size_t remainingLength = top->length() - _topOffset;
        size_t theLength = (length < remainingLength? length : remainingLength);
        
        *bytes = theBytes;
        *actualLength = theLength;
    }
    
    ILData* ConsumptionQueue::availableDataWithMaximumLength(size_t length) {
        
        ILData* top = this->topOfQueue();
        if (!top)
            return NULL;
        
        size_t remainingLength = top->length() - _topOffset;
        
        if (length >= remainingLength)
            return top;
        else
            return new ILData(top->bytes() + _topOffset, length, false);
        
    }
    
    size_t ConsumptionQueue::availableDataLength() {        
        ILData* top = this->topOfQueue();
        if (!top)
            return 0;
        
        size_t remainingLength = top->length() - _topOffset;
        return remainingLength;
    }
    
    void ConsumptionQueue::dequeueDataOfLength(size_t length) {
        
        ILData* top = this->topOfQueue();
        if (!top)
            return;
        
        _topOffset += length;
        
        while (top && _topOffset >= top->length()) {
            _topOffset -= top->length();
            _queue->removeObjectAtIndex(0);
            top = this->topOfQueue();
        }
        
    }
    
    static inline void ConsumptionQueueAppendSmartly(ILData* newData, ILData*& data, bool& didCopy) {
        
        if (!data)
            data = newData;
        else {
            if (!didCopy) {
                data = data->copy();
                didCopy = true;
            }
            
            data->appendBytes(newData->bytes(), newData->length());
        }
        
    }
    
    ILData* ConsumptionQueue::dataWithLength(size_t length) {
        ILData* result = NULL;
        bool didCopy = false;
		bool isFirst = true;
        
        ILListIterator* i = _queue->iterate();
        ILData* part;
        while ((part = ILAs(ILData, i->next()))) {
			
			if (isFirst && _topOffset != 0) {
				part = new ILData(part->bytes() + _topOffset, part->length() - _topOffset, false);
				isFirst = false;
			}
			
            if (length >= part->length()) {
                ConsumptionQueueAppendSmartly(part, result, didCopy);
                length -= part->length();
            } else {
                ILData* portion = new ILData(part->bytes(), length);
                ConsumptionQueueAppendSmartly(portion, result, didCopy);
                break;
            }
        }
        
        return result;
    }
	
	size_t ConsumptionQueue::offsetForFirstByteWithValue(uint8_t value) {
		size_t location = 0;
		bool first = true;
		
		ILListIterator* i = _queue->iterate();
        ILData* part;
        while ((part = ILAs(ILData, i->next()))) {
			size_t len = part->length();
			uint8_t* bytes = part->bytes();
			
			if (first) {
				bytes += _topOffset;
				len -= _topOffset;
				first = false;
			}
			
			uint8_t* offsetBytes = (uint8_t*) memchr(bytes, value, len);
			if (!offsetBytes)
				location += len;
			else
				return location + (offsetBytes - bytes);
		}
		
		return ILNotFound;
	}
}
