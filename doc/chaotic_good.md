Chaotic Good
============

Chaotic Good is a transport protocol for gRPC.

Its design goals are:

* Utilize multiple TCP connections to maximize throughput
* Ensure memory alignment for reads

This document describes the wire format of the protocol.

Base Layer
----------

The base layer of the protocol is a simple framing format.

One Chaotic Good connection is composed of a number of TCP connections. From here in these TCP connections will be referred to simply as "connections".

The first connection established from a client to a server is called the "control connection". The remaining connections established from a client to a server are called "data connections".

Frames are typically sent over the control connection, with the exception of an initial settings exchange that is also framed on each data connection.

Frame Format
------------

A frame consists of a header and a payload.

The header is 12 bytes long, containing three little-endian 32-bit integers:

+--------------------------+
|  type_and_connection_id  |
+--------------------------+
|  stream_id               |
+--------------------------+
|  payload_length          |
+--------------------------+

Where:

* `type_and_connection_id` is `frame_type` << 16 | `payload_connection_id`
* `payload_connection_id` is the connection identifier for the connection upon which the frame payload is sent: 0 for the control connection, and a unique non-zero value for each data connection
* `frame_type` is the type of the frame - valid values are defined below
* `stream_id` is the stream identifier, or 0 if the frame is not associated with a stream
* `payload_length` is the length of the payload in bytes

Payload is then encoded on the connection specified by `payload_connection_id`. If that connection is the control connection, then the payload is added to the stream directly regardless of length. If the connection is a data connection, then zero bytes are added to the frame payload until the total length of the frame is a multiple of the connections alignment setting (see [Settings](#settings)).

Frame Type: Settings
--------------------

Frame type 0x00 is reserved for the settings exchange.

Settings frames are always sent as the first frame on a connection. They are also the only frames that are sent on the data connections.

Settings frames always have stream id 0, and mark their connection id as 0 also.

Payload for settings frames is always sent directly on the connection, and is not padded to the alignment of the connection.

The settings frame payload is a `Settings` message (defined in [chaotic_good_frame.proto](../src/core/ext/transport/chaotic_good/chaotic_good_frame.proto)):

```proto
message Settings {
    repeated bytes connection_id = 1;
    bool data_channel = 2;
    uint32 alignment = 3;
}
```

Frame Type: ClientInitialMetadata
---------------------------------

Frame type 0x80 is for sending client initial metadata.
This frame is sent exclusively from the client to the server.

stream_id is the stream identifier for the stream to which the metadata is being sent.
This is a client picked identifier that must be monotonically incrementing.

The payload is a `ClientMetadata` message (defined in [chaotic_good_frame.proto](../src/core/ext/transport/chaotic_good/chaotic_good_frame.proto)):

```proto
message ClientMetadata {
    optional string path = 1;
    optional string authority = 2;
    optional uint64 timeout_ms = 3;

    repeated UnknownMetadata unknown_metadata = 100;
}
```

Frame Type: ClientEndOfStream
-----------------------------

Frame type 0x81 is for sending client end of stream.
This frame is sent exclusively from the client to the server.

stream_id is the stream identifier for the stream that is being ended.

The stream is then in a "half closed" state for the client.

Frame Type: ServerInitialMetadata
---------------------------------

Frame type 0x91 is for sending server initial metadata.
This frame is sent exclusively from the server to the client.

stream_id is the stream identifier for the stream to which the metadata is being sent.

The payload is a `ServerMetadata` message (defined in [chaotic_good_frame.proto](../src/core/ext/transport/chaotic_good/chaotic_good_frame.proto)):

```proto
message ServerMetadata {
    optional uint32 status = 1;
    optional bytes message = 2;

    repeated UnknownMetadata unknown_metadata = 100;
}
```

Frame Type: ServerTrailingMetadata
----------------------------------

Frame type 0x92 is for sending server trailing metadata.
This frame is sent exclusively from the server to the client.

stream_id is the stream identifier for the stream to which the metadata is being sent.

The payload is a `ServerMetadata` message (defined in [chaotic_good_frame.proto](../src/core/ext/transport/chaotic_good/chaotic_good_frame.proto)).

Frame Type: Message
-------------------

Frame type 0xa0 is for sending a gRPC message.

stream_id is the stream identifier for the stream to which the message is being sent.

The payload is the gRPC message payload.

It's likely this frame type will be deprecated in favor of BeginMessage and MessageChunk in a future revision of the protocol.

Frame Types: BeginMessage and MessageChunk
------------------------------------------

Frame types BeginMessage (0xa1) and MessageChunk (0xa2) are used to send the body of a gRPC message as a series of frames.

This allows for the message to be sent across multiple different connections, or to be incrementally sent due to flow control.

The BeginMessage frame is sent first, with payload `BeginMessage` (defined in [chaotic_good_frame.proto](../src/core/ext/transport/chaotic_good/chaotic_good_frame.proto)):

```proto
message BeginMessage {
    uint32 length = 1;
}
```

Next, zero or more MessageChunk frames are sent, each with sequential parts of the message.
The payload is complete when the received length of the payload matches the length specified in the BeginMessage frame.
If more bytes are received than specified in the BeginMessage, OR if a Message or BeginMessage frame is received whilst waiting for more bytes, this is a protocol violation and the connection must be terminated.

Frame Type: Cancel
------------------

Frame type 0xff is for canceling a stream and is sent exclusively from the client to the server.
Servers can indicate cancellation by sending a Server Trailing Metadata frame.

stream_id is the stream identifier for the stream to be cancelled.

This frame type never has a payload.

Stream State Machine
--------------------

Streams generally follow the gRPC state machine.

A client stream consists of:

(Client Initial Metadata) -> (Body Frame)* -> (Client End Of Stream | Cancel)

A server stream consists of:

(Server Initial Metadata) -> (Body Frame)* -> (Server Trailing Metadata)

Stream frames are sent in order for their particular stream, but may be interleaved with other frames - either transport frames or other stream frames.

Streams are only ever initiated by the client.
A stream is completely terminated either by the client sending a Cancel frame, or the server sending a Server Trailing Metadata frame.

A Body Frame consists of messages in the streams body.
These include message payloads, but also any flow control frames.
So a Body Frame may be one of:

* Message
* BeginMessage
* MessageChunk

