//
//  ILStream.h
//  MoverCore
//
//  Created by ∞ on 23/07/10.
//  Copyright (c) 2010 __MyCompanyName__. All rights reserved.
//

#ifndef ILStream_H
#define ILStream_H 1

#include "PlatformCore.h"

typedef enum {
	/** No error. (Never returned by the methods; use this to initialize a ILStreamError variable.) */
	kILStreamErrorNone = 0,
	
	/** Couldn't read because the end of file was reached. */
	kILStreamErrorEndOfFile,
	
	/** Couldn't read or write because the stream was previously closed. */
	kILStreamErrorIsClosed,
	
	/** Couldn't read or write because this stream was set as non-blocking and the operation would have blocked. */
    kILStreamErrorWouldHaveBlocked,
	
	/** Couldn't read because of an underlying POSIX error. See the errno variable for the exact error. */
    kILStreamErrorPOSIX,
} ILStreamError;

/**
 A stream is an object that encapsulates a POSIX file descriptor. It is constructed with an already-open FD, and it closes it on destruction.
 */
class ILStream : public ILObject {
public:
    ILStream(int fd);
    virtual ~ILStream();
    
	/**	Reads up to 'maximumBytes' from the given stream, returning it as a ILData object.
	 If there's an error, it returns NULL and, if a non-NULL pointer is given to a ILStreamError variable, it's filled with the error.
	 */
    ILData* read(ILSize maximumBytes, ILStreamError* e = NULL);
	
	/** Tries to write the given data to the stream. On success it returns true and, if the 'writtenLength' pointer is not NULL, the pointed-to variable will be filled with the amount of data that was written from the ILData object. On failure, it returns false and, if the 'e' pointer is not NULL, the pointed-to variable is filled with the error.
	 */
    bool write(ILData* data, ILSize* writtenLength, ILStreamError* e = NULL);
	
	/** Writes the given data to the stream, and returns only when all of it was written or an error is reported.
	 
	 For the purposes of this method, a kILStreamErrorWouldHaveBlocked error is not actually an error; this method instead will wait and retry the operation later (but still within the current method call). This implies that this method blocks until all of the data is written, regardless of whether the stream is currently allowed to block (se ::allowsBlocking()). */
	bool writeAllOf(ILData* data, ILStreamError* e = NULL);
	
	/** Reads from the stream, and returns only when the end-of-file error is reported. This method returns a non-NULL result if the end is reached, and a NULL result in case of an error (which is returned through 'e' if 'e' is non-NULL).
	 
	 This method ignores the current value of ::allowsBlocking(), and always blocks, much like ::writeAllOf().
	 */
	ILData* readUntilEnd(ILStreamError* e = NULL);
	
	/** Reads from the stream, and returns only when the given amount of bytes is read or an error is encountered. This method returns a non-NULL result if that many bytes have been read, and a NULL result in case of an error (which is returned through 'e' if 'e' is non-NULL).
	 
	 This method ignores the current value of ::allowsBlocking(), and always blocks, much like ::writeAllOf().
	 
	 If 'endOfFileIsError' is true (the default), then reaching the end of file before 'toRead' bytes have been read is treated as an error and returns a NULL error value; if it's false, then reaching the end of the file does not cause an error and will return a ILData instance that may be less than 'toRead' bytes long.
	*/
	ILData* readBytesUpTo(ILSize toRead, bool endOfFileIsError = true, ILStreamError* e = NULL);
	
	/** Closes the stream. Calling this on a closed stream is a no-op. */
    void close();
    
	// THE FUNCTIONALITY BELOW MAY VARY BY PLATFORM. BEWARE.
	// When unavailable, features will try their utmost to become no-ops without changing the semantics too much. Use the associated can... methods to check whether the feature you want to use is enabled or not.
	
	/** If true, this stream allows blocking and ::read() and ::write() calls will block until the OS thinks it has gathered enough data. If false, this stream will produce a kILStreamErrorWouldHaveBlocked in case you called ::read() and ::write() and the OS would have blocked. 
	 
		If the platform does not support non-blocking I/O, this returns false and calls to ::setAllowsBlocking() with 'true' will be no-ops. Use ::canDisableBlocking() to determine if this is the case.
	 */
    bool allowsBlocking();
	
	/** If true, non-blocking I/O is available for this stream. If false, ::setAllowsBlocking() is a no-op if passed 'true'. */
	bool canDisableBlocking();
	
	/** See ::allowsBlocking(). Returns true if the allows-blocking flag was set correctly, false otherwise. */
    bool setAllowsBlocking(bool x);
	
	
	/** If true, ready state for reading and writing can be determined (that is, ::isReadyForReading() and ::isReadyForWriting() will return false when the stream is not ready). If false, ready state cannot be determined, and the two calls above will always return true. */
	bool canDetermineReadyState();
	
	/** If true, the stream can be written to without blocking (or if the only way to know is to try). If false, the stream cannot be written to without blocking, or cannot be written to at all.
	 
	 This value is available independently of whether blocking is allowed or not (via ::setAllowsBlocking()). */
	bool isReadyForWriting();

	/** If true, the stream can be read from without blocking (or if the only way to know is to try). If false, the stream cannot be read from without blocking, or cannot be read from at all.
	 
	 This value is available independently of whether blocking is allowed or not (via ::setAllowsBlocking()). */
	bool isReadyForReading();
	
	/** Returns the file descriptor associated with this stream. This call can be used simultaneously from multiple threads. TODO: may not be appropriate for all streams. 
     */
	int fileDescriptor();
	
    /** Begins observing the use of this stream. This will cause messages to be produced whenever the stream is read from or written to.
     */
    void beginObservingStream();
    
    /** Ends observing the use of this stream, if this method is called a number of times equal to the number of preceding beginObservingStream calls. This stops messages from being produced.
     */
    void endObservingStream();
    
private:
    uint64_t _countOfObservingClients;
	int _fd;
	ILMutex* _mutex;
	void _determineReadyState(bool* reading, bool* writing);
};

/** Kind of messages sent while the stream is monitoring whenever a read operation was requested. */
extern void* kILStreamDidReadMessage;

/** Kind of messages sent while the stream is monitoring whenever a write operation was requested. */
extern void* kILStreamDidWriteMessage;


/** Stream sources provide new, just-opened streams to callers. A stream source may be reusable, or may be used only once. */
class ILStreamSource : public ILObject {
public:
	ILStreamSource(); virtual ~ILStreamSource();
	
	/** Returns a new, just-opened stream. In case of an error, returns NULL. */
	virtual ILStream* open() = 0;
	
	/** If true, the stream source can produce a new stream by calling its ::open() method (or the only way to know is to try). If false, calling ::open() will yield NULL. */
	virtual bool canOpen();
};

#endif // #ifndef ILStream_H
