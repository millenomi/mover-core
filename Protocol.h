/*
 *  Protocol.h
 *  MoverCore
 *
 *  Created by ∞ on 02/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "PlatformCore.h"

#ifndef MOVER_PROTOCOL_H
#define MOVER_PROTOCOL_H 1


static const uint8_t kMvrStreamStartingBytes[] = { 'M', 'O', 'V', 'R', '2' };
static const size_t kMvrStreamStartingBytesLength =
sizeof(kMvrStreamStartingBytes) / sizeof(uint8_t);

#define kMvrPacketParserSizeKey @"Size"

#define kMvrProtocolPayloadStopsKey @"Payload-Stops"
#define kMvrProtocolPayloadKeysKey @"Payload-Keys"

#define kMvrIndeterminateProgress ((float) -1.0)

// ----

#define kMvrProtocolExternalRepresentationPayloadKey @"externalRepresentation"
#define kMvrProtocolMetadataTitleKey @"MvrTitle"
#define kMvrProtocolMetadataTypeKey @"MvrType"


#endif // #ifndef MOVER_PROTOCOL_H
