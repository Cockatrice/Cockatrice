@page protocol_command_container CommandContainer (Protocol Concept)

@section cc_overview Overview

CommandContainer is the client-to-server message envelope used for all
client requests in the Cockatrice protocol.

Every client-initiated action is transmitted as a CommandContainer.
The server never sends CommandContainer messages.

This is a protocol-level abstraction and should not be confused with the
generated protobuf accessors or wire-level encoding.

@section cc_lifecycle Lifetime and Ownership

- CommandContainers are created by clients and consumed by the server.
- Each container represents one logical request.
- Containers are processed atomically and in order of arrival.

A container may generate:
- Exactly one RESPONSE message, and
- Zero or more EVENT messages.

@section cc_cmd_id Command Identification

The @c cmd_id field is a client-assigned identifier used to correlate
responses with requests.

Rules:
- @c cmd_id is optional but strongly recommended
- The server echoes @c cmd_id only in RESPONSE messages
- EVENT messages are never associated with a @c cmd_id

The server does not enforce uniqueness of @c cmd_id values.

@section cc_context Command Context

Certain command domains require additional context:

- Room commands require @c room_id
- Game commands require @c game_id

Context fields are ignored for command domains that do not require them.

If a required context field is missing or invalid, the server responds with
@c RespContextError.

@section cc_invariants Invariants

The following rules are enforced by the server:

- Exactly one command domain must be populated
- Commands may be batched only within the same domain
- Context fields must match the active command domain

Violations of these rules result in @c RespInvalidCommand.

@section cc_domains Command Domains

CommandContainer supports the following mutually exclusive command domains:

- Session commands
  - Authentication
  - User discovery
  - Private messaging

- Room commands
  - Chat
  - Game creation
  - Room membership management

- Game commands
  - In-game actions
  - State mutations

- Moderator commands
  - User moderation
  - Room moderation

- Admin commands
  - Server administration

@section cc_batching Command Batching

A CommandContainer may contain multiple commands of the same domain.

Batching guarantees:
- Commands are processed in the order they appear in the container
- All commands in the batch share the same context

Partial failure behavior is implementation-defined and may vary by domain.

@section cc_dispatch Dispatch

CommandContainers are dispatched by
Server_ProtocolHandler::processCommandContainer().

Dispatch is performed by inspecting which command domain is populated.
Only the first populated domain is considered.

@section cc_errors Error Handling

Common error responses include:
- @c RespLoginNeeded – client is not authenticated
- @c RespInvalidCommand – malformed container or invalid domain usage
- @c RespContextError – missing or invalid room/game context

@section cc_related Related Concepts

- @ref protocol_server_message "ServerMessage"
- @ref protocol_client_states "Client State Machine"

@see Server_ProtocolHandler::processCommandContainer

