@page protocol_server_message ServerMessage (Protocol Concept)

## Overview

`ServerMessage` is the server-to-client message envelope used for all
communication originating from the Cockatrice server.

The server never sends `CommandContainer` messages, and clients never send
`ServerMessage` messages.

A `ServerMessage` represents either:

- A direct response to a client command, or
- An unsolicited event emitted by the server

This document describes `ServerMessage` as a protocol-level concept.
It should not be confused with generated protobuf accessors or wire-level
encoding details.

---

## Lifetime and Delivery

`ServerMessage` instances are emitted by the server and delivered
asynchronously to clients.

Delivery guarantees:

- Messages are delivered in the order sent by the server
- Messages may be delivered at any time
- Events may be delivered before or after responses

Clients **must** be prepared to handle `ServerMessage` objects at all times,
regardless of pending requests.

---

## Message Type

Each `ServerMessage` contains **exactly one payload**, identified by the
`message_type` field.

The following message types are supported:

- `RESPONSE`
- `SESSION_EVENT`
- `ROOM_EVENT`
- `GAME_EVENT_CONTAINER`

A message containing multiple payloads or an unknown type is considered
malformed.

---

## RESPONSE

`RESPONSE` messages are direct replies to client-issued `CommandContainer`
messages.

Characteristics:

- Contain a `Response` payload
- Echo the client-assigned `cmd_id`
- Indicate success or failure of the request

A single `CommandContainer` results in **at most one** `RESPONSE`.

`RESPONSE` messages may be preceded or followed by event messages.

---

## Events

Event messages are unsolicited notifications emitted by the server.

Event message types include:

- `SESSION_EVENT`
- `ROOM_EVENT`
- `GAME_EVENT_CONTAINER`

Event messages:

- Are not correlated to a specific client command
- Do not include a `cmd_id`
- May be delivered at any time

Clients **must** process events independently of responses.

---

## Event Scopes

### Session Events

Session events are global to the client session and may include:

- Server notifications
- Private messages
- User status updates

---

### Room Events

Room events are scoped to a specific room and are delivered only to clients
currently present in that room.

---

### Game Events

Game events are scoped to a specific game and are delivered only to
participating clients.

Game events are grouped within a `GameEventContainer` to allow batching.

---

## Event Batching

Some `ServerMessage` types may contain multiple logical events.

- `GameEventContainer` may batch multiple game events
- Other event types represent a single logical event

Clients must process all contained events **in order**.

---

## Error Semantics

Errors are communicated **exclusively** via `RESPONSE` messages.

Event messages are never used to signal command failure.

Common error responses include:

- `RespInvalidCommand`
- `RespLoginNeeded`
- `RespContextError`
- `RespChatFlood`

---

## Related Concepts

- CommandContainer
- Client State Machine
- Response

---

## Reference Proto Definition

```proto
message ServerMessage {
    enum MessageType {
        RESPONSE = 0;
        SESSION_EVENT = 1;
        GAME_EVENT_CONTAINER = 2;
        ROOM_EVENT = 3;
    }

    optional MessageType message_type = 1;
    optional Response response = 2;
    optional SessionEvent session_event = 3;
    optional GameEventContainer game_event_container = 4;
    optional RoomEvent room_event = 5;
}
```