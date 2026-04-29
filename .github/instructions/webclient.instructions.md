---
applyTo: "webclient/**"
---

# Webclient instructions

Applies to the React/TypeScript SPA in `webclient/` (Webatrice) — a **browser port of the desktop Cockatrice client**. It connects to the **same Servatrice server** as desktop over a WebSocket. UI behavior, and especially how UI code drives the websocket layer (commands, response handling, event-driven state changes), **must match the desktop client** unless a scope reduction is explicitly agreed per milestone. Divergence is a defect by default — see [#desktop-parity-mandate](#desktop-parity-mandate). The package is otherwise self-contained; the only thing it shares with the rest of the repo (C++ desktop/server stack) is the protobuf protocol in `../libcockatrice_protocol/`, and anything outside `webclient/` is out of scope unless a task explicitly touches the protocol.

Canonical AI-tool instruction surface for this package — invariants, policy, and external facts. When a source comment ends with `See .github/instructions/webclient.instructions.md#<anchor>`, the section with that anchor lives here. Source comments tagged `// @critical` guard cross-file invariants; do not remove them without updating the relevant section. For commands, stack, and getting-started, see [webclient/README.md](../../webclient/README.md).

## Desktop parity mandate

The webclient is a port of the desktop Cockatrice C++ client — same server, same game, same users. This is a **hard baseline**, not an ambiguity tie-breaker. Every webclient behavior difference from desktop is treated as a defect unless it has been explicitly scoped out for the current milestone.

**UI ↔ websocket parity is the sharpest edge of this rule.** Command shapes, field-level defaults (what the client sends vs. omits), response/event handling, and the resulting state transitions must mirror desktop. A webclient that issues a subtly different command, or reacts differently to the same event, breaks multi-client play — a desktop player and a webclient player joined to the same Servatrice room must see consistent game state.

**Desktop is the spec.** The reference implementation lives at `../cockatrice/src/` (relative to the repo root). Before proposing any UX or websocket-interaction decision that isn't obvious from the webclient code, read the corresponding desktop source.

**Divergence protocol:**

1. If desktop behavior is expensive to replicate in the current milestone, propose a **scope reduction explicitly** (e.g. "M4 ships default red arrows; color picker defers to M6"). Get agreement before coding. Record deferred parity gaps in [webclient/plans/gameboard-deferrables.md](../../webclient/plans/gameboard-deferrables.md) as "parity gap — deferred to <milestone>".
2. Phase-end reviews treat Cockatrice-parity findings as **blockers** by default. Elevate them; don't defer unless the user has explicitly OK'd the gap.
3. The only categorically valid reasons to diverge without a scope-reduction sign-off are: a browser security constraint (e.g. no raw TCP), a fundamental input-model difference (touch vs. mouse), or an accessibility requirement desktop doesn't meet.

This section subsumes the one-line "matches the Cockatrice desktop client" note in [#startup--session-invariants](#startup--session-invariants); that remains as a concrete example of the rule, not a standalone source of truth.

## Architecture

### Protocol layer

`src/generated/proto/` is buf-generated from `../libcockatrice_protocol/` (gitignored, never hand-edit). Runtime is `@bufbuild/protobuf`. `src/types/` re-exports the bindings namespaced as `Data` (raw proto), `Enriched` (UI/domain composition — proto extended with client-only sibling fields), and `App` (pure client types; no proto dependency). Consumer pattern: `import { Data, Enriched, App } from '@app/types'` then `Data.ServerInfo_User`, `Enriched.GameEntry`, `App.RouteEnum`. **UI, store, hooks, and api code must import proto types through `@app/types`, never `@app/generated` directly. `src/websocket/` is the exception and imports `@app/generated` by design.**

Websocket protocol/transport types (`StatusEnum`, `WebSocketConnectReason`, the `*ConnectOptions` family, signal payload contexts `PendingActivationContext` / `LoginSuccessContext`, `GameEventMeta`, the `I*Request` / `I*Response` contracts, `WebClientConfig`) live separately under `@app/websocket/types` and are exposed as a single `WebsocketTypes` namespace: `import { WebsocketTypes } from '@app/websocket/types'` then `WebsocketTypes.StatusEnum`, `WebsocketTypes.LoginConnectOptions`, etc. This is the only public surface of the websocket layer's types — store, hooks, api, and UI code must access websocket types through this namespace. **Don't re-export websocket types through `Enriched`**; that namespace is strictly UI/domain composition. `@app/websocket` (the broader index) only exposes runtime values (`WebClient`, command groups, `setPendingOptions`, etc.) — not types. Inside `src/websocket/` use relative paths to specific files under `types/` (e.g. `from '../types/StatusEnum'`) rather than either alias.

### WebSocket layer (`src/websocket/`)

Outbound commands in `commands/<scope>/`, inbound handlers in `events/<scope>/`, transport in `services/`, type declarations in `types/` (request/response contracts, `StatusEnum`, `WebSocketConnectReason`, connect-options union, signal contexts — all exposed to outside consumers as the `WebsocketTypes` namespace via `@app/websocket/types`). `WebClient` is a singleton; `new WebClient(...)` is called only inside `WebClientProvider` ([webclient/src/hooks/useWebClient.tsx](../../webclient/src/hooks/useWebClient.tsx)), never at module load.

**Layering invariant (enforced, zero violations today — keep it that way):**

1. Containers and components call `useWebClient()` to get the `WebClient`, then `client.request.<scope>.<method>(…)`. Never import from `@app/websocket` in UI code (`@app/websocket/types` is fine — type-only); never call `new WebClient(...)` outside `WebClientProvider`.
2. `src/api/request/*RequestImpl` methods translate UI intent into `src/websocket/commands/*` calls. `src/api/response/*ResponseImpl` methods are invoked by command callbacks and event handlers and dispatch to the store.
3. Only `*.dispatch.ts` helpers inside `src/store/` and the `*ResponseImpl` classes may touch the Redux store.

If you find yourself wanting to skip a layer (dispatching from an event handler, calling `@app/websocket` from a container, reaching into `@app/generated` from a component/store), stop. `eslint.boundaries.mjs` enforces this via the element types `api` / `components` / `containers` / `hooks` / `services` / `store` / `types` / `websocket` / `websocket-types`; `websocket-types` is deliberately a narrower surface than `websocket` so UI/store can reach protocol types without pulling in transport internals.

### ProtobufService: request/response correlation

- Every outbound `CommandContainer` gets a monotonically increasing `cmdId` (cast to `BigInt` for the proto field — the wire type is `int64`). A `Map<number, callback>` stores the response handler keyed by that ID; `processServerResponse` looks up and invokes the callback on `RESPONSE`, then deletes the entry. The `number` ↔ `BigInt` sides stay in sync because the counter never realistically exceeds `Number.MAX_SAFE_INTEGER`.
- **No timeout or retry** at the transport layer. `resetCommands()` (called on reconnect) zeros `cmdId` and clears the pending map, silently dropping any in-flight callbacks. Reconnection resilience is a caller concern.
- `sendCommand` is a no-op write if the transport isn't open — it still registers the callback, so a stale pending entry can accumulate until the next reset.
- Inbound event dispatch is extension-based: `processRoomEvent` / `processSessionEvent` / `processGameEvent` iterate the relevant registry array (entries built with `makeEntry(ext, handler)`) and invoke the first handler whose extension is set on the message. Adding a new handler means appending a `makeEntry(ExtSymbol, handler)` line to the relevant registry.

### command-options contract (`src/websocket/services/command-options.ts`)

Every `send*Command` call accepts an optional `CommandOptions<R>`:

- `responseExt?: GenExtension<Response, R>` — the response payload extension to unwrap on success.
- `onSuccess?: (response: R, raw: Response) => void` — called when `responseCode === RespOk`. If `responseExt` is absent, the overload becomes `() => void`.
- `onResponseCode?: { [code: number]: (raw: Response) => void }` — per-error-code handlers.
- `onError?: (code: number, raw: Response) => void` — fallback for codes not in `onResponseCode`.
- `onResponse?: (raw: Response) => void` — if set, handles the raw response and bypasses every other hook. Use when you need the full response object regardless of code.

If none of the hooks fire for a non-OK response, `handleResponse` logs via `console.error` with the command's proto type name. Practical rule: `onSuccess` funnels into a `*ResponseImpl` method, `onError` funnels into a `*ResponseImpl` method (usually to flip connection state or show a toast), `onResponse` is rare.

### Public API for UI (`src/api/`)

One `*RequestImpl` / `*ResponseImpl` class per scope (session / rooms / game / admin / moderator; plus `AuthenticationRequestImpl` — auth has no inbound events). Request methods return `void` — fire-and-forget; response flows back via `command-options` callbacks → `*ResponseImpl` → store. `*ResponseImpl` classes are the only place outside `src/store/*.dispatch.ts` that calls `*Dispatch` helpers. **UI code never imports from `src/api/` directly — use `useWebClient()`.** Never call `client.response.*` from UI.

### State (`src/store/`)

Slices: `server/`, `rooms/`, `game/`. Consumers import through the `@app/store` barrel (`GameSelectors`, `GameDispatch`, `GameTypes`, same for `Server`/`Rooms`). **Don't deep-import from `src/store/<slice>/*` — add the symbol to the barrel's `index.ts` instead.** This rule generalizes: deep paths through any `@app/*` barrel target are a smell.

Shape notes worth knowing before you touch a reducer:

- `game/` is deeply normalized: `games[gameId].players[playerId].zones[zoneName].cards`. Selectors are plain getters so lookups stay O(1); `createSelector` is reserved for the few that build derived lists (e.g. `getActiveGameIds`).
- Selectors return module-scope `EMPTY_ARRAY` / `EMPTY_OBJECT` constants for missing data to preserve referential equality and avoid spurious re-renders.
- `rooms/` is *partially* normalized: rooms are keyed by ID but each room also carries denormalized `gameList` / `userList` arrays. Server updates often omit those lists, so the reducer merges new metadata while preserving the existing arrays. Standing TODO to clean this up.
- `server/` is mostly flat maps keyed by username (`messages`, `userInfo`, buddy/ignore lists) plus connection state.

### Local persistence

Dexie (IndexedDB) holds cards, sets, tokens, known hosts, and settings; separate from Redux (persists across reloads). Stubbed globally in `setupTests.ts` so unit specs never hit a real IndexedDB.

### UI

Route-level containers in `containers/` (one subdir per route plus `AppShell` root and shared `Layout`); routing in `containers/App/AppShellRoutes.tsx`. Two hooks are load-bearing: **`useWebClient`** (context accessor — the only way UI code is allowed to reach the server; see the Layering invariant) and **`useAutoLogin`** (owns the once-per-session gate; see [#startup--session-invariants](#startup--session-invariants)). `WebClientProvider` ([webclient/src/hooks/useWebClient.tsx](../../webclient/src/hooks/useWebClient.tsx)) owns the singleton; `WebClientContext` is exported so integration tests can inject a pre-built `WebClient`. UI kit: MUI v9 + `@emotion`; i18n via `react-i18next` + ICU (Transifex).

## Build pipeline and generated files

`npm start` / `npm run build` run `prestart`/`prebuild` hooks: `proto:generate` followed by `node prebuild.js`. `prebuild.js` writes `src/server-props.json` (git SHA), merges `src/**/*.i18n.json` into `src/i18n-default.json` (**throws on duplicate keys** — namespace your i18n keys), and copies country flags from `../cockatrice/resources/countries`.

| File | Tracked? | Regenerate with |
|---|---|---|
| `src/generated/proto/**` | Gitignored | `npm run proto:generate` |
| `src/server-props.json` | Gitignored | `npm start` / `npm run build` (prebuild writes it) |
| `src/i18n-default.json` | **Committed** | `npm run translate` (or the prebuild hook) |

`.env.development`, `.env.production`, `.env.test` exist but are empty. No `import.meta.env` configuration surface; server URLs resolve through the login UI / `server-props.json`.

## Testing

Vitest + Testing Library + jsdom. [webclient/src/setupTests.ts](../../webclient/src/setupTests.ts) registers jest-dom matchers and installs a global Dexie mock.

Unit specs run under [webclient/vite.config.ts](../../webclient/vite.config.ts) with `test.isolate: true`: every spec file gets a fresh module graph, but tests **within the same file share it**. `vi.clearAllMocks()` (clears call logs) runs in the global `afterEach` and is safe. **Never add `vi.resetAllMocks()` to `setupTests.ts`** — it resets `vi.fn()` instances created inside `vi.mock(...)` factories at file load, breaking any spec that mocks something once (e.g. `store.dispatch`) and expects it to persist across tests in the file.

Integration specs run under [webclient/vitest.integration.config.ts](../../webclient/vitest.integration.config.ts) via `npm run test:integration` — slower; exercise the wired-up `WebClient` against fakes in `src/__test-utils__/`.

**Globals that leak within a file.** `vi.restoreAllMocks()` only restores `vi.spyOn` targets. Bare `Object.defineProperty` writes (e.g. on `window.location`) and global reassignments (e.g. `globalThis.WebSocket = ...`) leak between tests in the same file — `setupTests.ts` does not auto-restore them. Use `withMockLocation` from [webclient/src/__test-utils__/globalGuards.ts](../../webclient/src/__test-utils__/globalGuards.ts) for scoped overrides that clean up after themselves.

**Shared scaffolding.** [webclient/src/__test-utils__/](../../webclient/src/__test-utils__/) provides render helpers, a mock-client builder, and global guards. Prefer these over hand-rolling providers — the integration suite depends on injecting pre-built `WebClient` instances through them. Store slices have co-located `__mocks__/fixtures.ts` files exposing `make*` factories that build protobuf messages via `create(Schema, overrides)`; reuse them instead of hand-rolling proto objects.

`npm run golden` (lint + unit + integration) is the CI gate — run it before declaring work done.

## Protocol changes

When a task edits `.proto` files in `../libcockatrice_protocol/`:

1. Run `npm run proto:generate`.
2. Update any command / event / `*RequestImpl` / `*ResponseImpl` code that consumes the changed messages.
3. Commit consumer changes only — `src/generated/proto/**` is gitignored and must not be committed.

---

## Domain Knowledge

Facts that can't be read off the code — external systems (Servatrice protocol, Protobuf-ES runtime, browser WebSocket semantics) and invariants the code relies on but cannot itself express.

### Initialization order

Protobuf-ES maps proto `int64` / `uint64` fields to native `BigInt`. `BigInt.prototype` has no `toJSON`, so `JSON.stringify` throws on any state that contains one — which Redux DevTools, structured logging, and React error-boundary dumps all do. [webclient/src/polyfills.ts](../../webclient/src/polyfills.ts) installs a `BigInt.prototype.toJSON` that returns `this.toString()`, coercing to string on serialize.

Coercion is one-way: `JSON.parse` does not round-trip back to `BigInt`. That is acceptable because in-memory state still holds real `BigInt`s; only serialized surfaces (devtools, logs) see the coerced form.

The polyfill must execute before any module creates the store, or the first devtools dump throws. Enforced by making `./polyfills` the first import in [webclient/src/index.tsx](../../webclient/src/index.tsx) and [webclient/src/setupTests.ts](../../webclient/src/setupTests.ts).

### Startup / session invariants

Product requirement: **auto-login runs at most once per JS session, and logout within the same session does NOT re-trigger it.** Only a full page refresh does. This matches the Cockatrice desktop client.

The gate lives at module scope in [webclient/src/hooks/useAutoLogin.ts](../../webclient/src/hooks/useAutoLogin.ts) as `autoLoginGate.hasChecked`. It flips to `true` after the startup check completes, regardless of whether the check actually fired a login — so a check that determined "don't auto-connect" (preference off, no saved password, etc.) still latches the gate. The gate is exported as a mutable object so integration tests can reset it without `vi.resetModules()`.

`useAutoLogin` consults settings via `getSettings()` (one-shot), not by subscribing to `settingsStore`. Editing the persisted auto-connect preference is a preference write, not a login signal.

### Data structure invariants

`Enriched.Room` and `Enriched.GameEntry` compose a raw proto (`info`) with client-side sibling fields. The TypeScript types cannot distinguish which fields stay fresh and which go stale, so this is a convention:

- **`info` is a wire snapshot at one point in time.** For `Room` it's the last `UPDATE_ROOMS` / `JOIN_ROOM` payload; for `GameEntry` it's the `Event_GameJoined` payload.
- **Fields on `info` that evolve via later events immediately go stale.** Read the sibling, never `info.*`:

| Type | Stale on `info` | Read instead |
|---|---|---|
| `Room` | `info.gameList` | `room.games` |
| `Room` | `info.userList` | `room.users` |
| `Room` | `info.gametypeList` | `room.gametypeMap` |
| `GameEntry` | `info.started` | `game.started` |
| `GameEntry` | `info.activePlayerId` etc. | top-level twin fields |

Adding a new field that updates via events means adding a top-level twin in [webclient/src/types/enriched.ts](../../webclient/src/types/enriched.ts) and never reading `info.<same-name>` after the initial snapshot.

### Reducer merge rules

Servatrice's `UPDATE_ROOMS` event carries room metadata only: the repeated `gameList` / `userList` / `gametypeList` collections on each `ServerInfo_Room` may be absent or stale. The reducer at [webclient/src/store/rooms/rooms.reducer.tsx](../../webclient/src/store/rooms/rooms.reducer.tsx) replaces `info`, `gametypeMap`, and `order` on existing rooms but preserves the normalized `games` and `users` maps, which are maintained by their own events (`updateGames`, `userJoined`, `userLeft`).

### Shared store pattern

`createSharedStore` in [webclient/src/hooks/useSharedStore.ts](../../webclient/src/hooks/useSharedStore.ts) exposes two surfaces with different semantics. Pick the right one per caller:

- **`subscribe` / `getSnapshot` (via `useSharedStore`)** — reactive. The component re-renders on every store update. Use from inside render.
- **`whenReady()`** — one-shot. Resolves with the first loaded value, then never fires again. Use from code that must read the loaded value exactly once and must NOT re-run on later updates (notably, startup orchestrators reading persisted preferences).

Subscribing in a startup orchestrator turns a later user action (ticking a preference) into a re-evaluation of startup logic, which is almost always wrong.

### Protocol quirks

Servatrice-side behavior the client has to accommodate:

- **`ServerOptions` is a bitmask.** [webclient/src/websocket/utils/passwordHasher.ts](../../webclient/src/websocket/utils/passwordHasher.ts) `passwordSaltSupported` uses `&`, not `===`. Don't "fix" it.
- **System-injected user messages can omit the username** (e.g. ban notifications where the target is the current user, or server announcements). [webclient/src/store/common/normalizers.ts](../../webclient/src/store/common/normalizers.ts) `normalizeUserMessage` handles this at the dispatch layer so the store always holds a clean user-facing string.

### WebSocket lifecycle

A failed `WebSocket` connect fires both `onerror` and `onclose`. `onerror` runs first with the richer status; [webclient/src/websocket/services/WebSocketService.ts](../../webclient/src/websocket/services/WebSocketService.ts) guards the `onclose` handler with `hasReportedError` so the generic "Connection Closed" doesn't overwrite the specific "Connection Failed". The flag clears on `onopen` and at the end of each `onclose` cycle.
