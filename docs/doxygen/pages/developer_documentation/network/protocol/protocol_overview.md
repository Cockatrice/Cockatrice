@page developer_reference_protocol_overview Protocol (Overview)

# Cockatrice Server Protocol – Overview

## Purpose

This document describes the Cockatrice client/server protocol as implemented
by the Cockatrice server. It is intended for developers implementing clients
or modifying the server.

The protocol is **stateful**, **event-driven**, and **asynchronous**.

---

## High-Level Properties

- Encoding: Google Protocol Buffers
- Transport: TCP or WebSocket (transport-agnostic at protocol level)
- Directionality:
    - Clients send `CommandContainer`
    - Server sends `ServerMessage`
- Responses and events are interleaved
- Clients must handle unsolicited events at any time

---

## Message Envelopes

### Client → Server: CommandContainer

A `CommandContainer` represents one logical client request.

Protocol invariants:
- Exactly one command domain may be used per container
- Multiple commands of the same domain may be batched
- Context is provided via `room_id` or `game_id` when required

Command domains:
- Session commands
- Room commands
- Game commands
- Moderator commands
- Admin commands

---

### Server → Client: ServerMessage

A `ServerMessage` represents either:
- A response to a command (`RESPONSE`)
- An unsolicited event (`*_EVENT`)

Protocol invariants:
- Events may be delivered before or after responses
- Responses reference the original command via `cmd_id`
- Events are never correlated to a command

---

## Asynchronous Model

Clients MUST NOT assume request/response ordering.

Example valid sequence:

1. Client sends JOIN_ROOM
2. Server sends room chat history events
3. Server sends room join notifications
4. Server sends JOIN_ROOM response

---

## Versioning

The server reports a protocol version during connection initialization.
Clients MUST verify compatibility before sending commands.
