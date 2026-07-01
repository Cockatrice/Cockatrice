@page protocol_response Response (Protocol Concept)

## Overview

`Response` is the server-to-client message sent immediately after a command, using the same `cmd_id` to link to the originating command.  
It provides a `Response::ResponseCode` and optionally a `Response::ResponseType` to guide client handling.

## Fields

- **cmd_id** (`uint64`, required) — Command ID this response corresponds to
- **response_code** (`Response::ResponseCode`, optional) — Outcome of the command

## Response Codes

All possible outcome codes are defined in Response::ResponseCode

These describe the result of the command, e.g., success, failure, or special conditions like bans or registration requirements.

## Response Types

Responses are routed according to Response::ResponseType

Each `ResponseType` corresponds to a high-level category, like `JOIN_ROOM` or `DECK_UPLOAD`.

## Extensions

- Protobuf extensions from 100 to max are reserved for future use
