# CLAUDE.md

Guidance for Claude Code when working inside `webclient/` — the React/TypeScript SPA (Webatrice) that connects to a Servatrice server over a WebSocket. It is a self-contained application; the only thing it shares with the rest of the repo (C++ desktop/server stack) is the protobuf protocol in `../libcockatrice_protocol/`. Anything outside `webclient/` is out-of-scope unless a task explicitly touches the protocol.

All commands below are run from this directory.

## Commands

```bash
npm start            # Vite dev server (runs proto:generate + prebuild.js first)
npm run build        # production build (same prebuild hooks)
npm test             # vitest run (one-shot)
npm run test:watch   # vitest watch
npm run lint         # eslint src/
npm run lint:fix
npm run golden       # lint + test — the CI-equivalent gate to run before declaring work done
npm run proto:generate   # `npx buf generate` — regenerates TS bindings into src/generated/proto
```

Single test file: `npx vitest run path/to/file.spec.ts`. Filter by name: `npx vitest run -t "partial test name"`.

The dev server has `server.open: true`, so `npm start` pops a browser tab automatically.

## Architecture

The webclient is a Redux Toolkit + RxJS app. Its defining abstraction is a layered WebSocket client that speaks the Cockatrice protobuf protocol to Servatrice. Understanding the layering is essential before editing anything under `src/websocket/`, `src/api/`, or `src/store/`.

### Protocol layer

- **`src/generated/proto/`** — auto-generated from `../libcockatrice_protocol/**/*.proto` by `buf` (see `buf.gen.yaml`). Never edit by hand. Runtime is `@bufbuild/protobuf` (Protobuf-ES); the codebase was recently migrated off the older `protobufjs`, so if you find any stray references to the old runtime, they're bugs.
- **`src/types/` is the only public surface for generated code.** `src/types/data.ts` hand-rolls an `export *` barrel over every proto file that consumers use, and `src/types/index.ts` re-exports it as `Data`, plus `Enriched` (protocol types extended with client-only fields) and `App` (pure client types). Import as `import { Data, Enriched } from '@app/types'` and use `Data.Command_Login`, `Data.ServerInfo_User`, etc. **Never import directly from `@app/generated/proto/*` outside `src/types/`.** When a new proto file starts being consumed, add an `export *` line to `src/types/data.ts` — there is a standing TODO to replace this rollup with a protobuf-es plugin.

### WebSocket layer (`src/websocket/`)

A strict inbound/outbound split sits on top of a transport core:

- **`services/`** — transport: `WebSocketService` (socket lifecycle), `ProtobufService` (encode/decode + request/response correlation), `KeepAliveService` (ping/pong), `command-options` (per-command response config). This layer has no knowledge of Redux.
- **`commands/`** — *outbound*. Organised by scope (`session/`, `room/`, `game/`, `admin/`, `moderator/`). Each command builds a protobuf request and hands it to `ProtobufService.send{Session,Room,Game,Admin,Moderator}Command` along with a `CommandOptions` describing how to handle the response.
- **`events/`** — *inbound*. Handlers for server-pushed events, same scopes. They translate protobuf events into calls on the persistence layer.
- **`persistence/`** — the **only** bridge from the websocket layer into app state. `SessionPersistence`, `RoomPersistence`, `GamePersistence`, `AdminPersistence`, `ModeratorPersistence` dispatch Redux actions and/or write to Dexie.
- **`WebClient.ts`** — singleton facade that wires the services, commands, events, and persistence together.

**Layering invariant (enforced on this branch, not aspirational):**

1. Containers and components call `src/api/*` services — never `src/websocket/*` directly.
2. Commands and event handlers call `*Persistence` methods — never `store.dispatch` directly.
3. Only `*.dispatch.ts` helpers inside `src/store/` and persistence code may touch the Redux store.

If you find yourself wanting to skip a layer (dispatching from an event handler, calling a command from a container, reaching into `src/generated/proto/` from a component), stop — the refactor on `webclient-websocket-layer` exists precisely to eliminate those shortcuts. There are currently zero violations; keep it that way.

### ProtobufService: request/response correlation

- Every outbound `CommandContainer` gets a monotonically increasing `cmdId` (cast to `BigInt` for the proto field). A `Map<number, callback>` stores the response handler keyed by that ID; when `ServerMessage.RESPONSE` arrives, `processServerResponse` looks up and invokes the callback, then deletes the entry.
- There is **no timeout or retry**. `resetCommands()` (called on reconnect) zeros `cmdId` and clears the pending map, silently dropping any in-flight callbacks. Code that needs reconnection resilience has to handle it at a higher layer.
- `sendCommand` is a no-op write if the transport isn't open — it still registers the callback, so a stale pending entry can accumulate until the next reset.
- Inbound event dispatch is extension-based: `processRoomEvent` / `processSessionEvent` / `processGameEvent` iterate `RoomEvents` / `SessionEvents` / `GameEvents` (tuples of `[extension, handler]`) and invoke the first handler whose extension is set on the message. Adding a new event handler means appending to those arrays.

### command-options contract (`src/websocket/services/command-options.ts`)

Every `send*Command` call accepts an optional `CommandOptions<R>`:

- `responseExt?: GenExtension<Response, R>` — the response payload extension to unwrap on success.
- `onSuccess?: (response: R, raw: Response) => void` — called when `responseCode === RespOk`. If `responseExt` is absent, the overload becomes `() => void`.
- `onResponseCode?: { [code: number]: (raw: Response) => void }` — per-error-code handlers.
- `onError?: (code: number, raw: Response) => void` — fallback for codes not in `onResponseCode`.
- `onResponse?: (raw: Response) => void` — if set, it handles the raw response and bypasses every other hook. Use this when you need the full response object regardless of code.

If none of the hooks fire for a non-OK response, `handleResponse` logs the failure via `console.error` with the command's proto type name. The practical rule: `onSuccess` funnels into persistence, `onError` funnels into persistence (usually to flip connection state or show a toast), and `onResponse` is rare.

### Public API for UI (`src/api/`)

Thin service wrappers (`AuthenticationService`, `SessionService`, `RoomsService`, `GameService`, `ModeratorService`, `AdminService`) that expose websocket commands to UI code. A few things to know:

- **All command methods are `static` and return `void`.** They're fire-and-forget — the response flows back through the `command-options` callbacks plumbed inside the command itself, into persistence, into the store. Don't try to await them.
- A handful of methods return `boolean` (e.g. `AuthenticationService.isConnected`, `isModerator`) — those are pure sync predicates, not command sends.
- Files use the `.tsx` extension even though they contain no JSX. That's a leftover convention; don't "fix" it.

### State (`src/store/`)

Redux Toolkit store (`store.ts`, `rootReducer.ts`) split by feature. Each slice follows the same file layout:

- `*.actions.ts` — action creators
- `*.reducer.ts` — slice reducer
- `*.selectors.ts` — selectors (mostly plain getters; `createSelector` only for derived lists)
- `*.dispatch.ts` — dispatch helpers called by the persistence layer
- `*.interfaces.ts` / `*.types.ts` — state shape and enums

Slices: `server/`, `rooms/`, `game/`, plus shared `actions/` and `common/` helpers (`SortUtil`, `normalizers`). Consumers import through the `@app/store` barrel — `GameSelectors`, `GameDispatch`, `GameTypes`, and the same prefixed set for `Server`/`Rooms`. **Don't deep-import from `src/store/game/game.selectors.ts` etc.** — go through `@app/store`.

Shape notes worth knowing before you touch a reducer:

- `game/` is deeply normalized: `games[gameId].players[playerId].zones[zoneName].cards`. Selectors are plain getters so lookups stay O(1); `createSelector` is reserved for the few that build derived lists (e.g. `getActiveGameIds`).
- Selectors return module-scope `EMPTY_ARRAY` / `EMPTY_OBJECT` constants for missing data to preserve referential equality and avoid spurious re-renders.
- `rooms/` is *partially* normalized: rooms are keyed by ID, but each room also carries denormalized `gameList` / `userList` arrays. Server updates often omit those lists, so the reducer merges new metadata while preserving the existing arrays. There is a standing TODO to clean this up.
- `server/` is mostly flat maps keyed by username (`messages`, `userInfo`, buddy/ignore lists) plus connection state.

### Local persistence (`src/services/dexie/`)

IndexedDB storage via Dexie for cards, sets, tokens, known hosts, and settings. DTOs live in `DexieDTOs/`. This is separate from the Redux store — used for data that should survive a reload (card database, user settings, host list). Dexie is not mocked in unit tests; code that writes to Dexie is typically exercised only in integration paths.

### UI

- **`containers/`** — route-level, Redux-connected. Top-level routes: `App`, `Initialize`, `Login`, `Server`, `Room`, `Game`, `Player`, `Decks`, `Account`, `Logs`, `Layout`, `Unsupported`. Routing lives in `containers/App/AppShellRoutes.tsx`.
- **`components/`** — presentational, mostly unconnected.
- **`forms/`** — `react-final-form` forms (e.g. `LoginForm`).
- **`dialogs/`** — MUI dialogs.
- **`hooks/`** — shared hooks (e.g. `useAutoConnect`).
- **`i18n.ts` / `i18n-backend.ts`** — `react-i18next` + ICU; translations managed via Transifex.
- UI kit: MUI v7 (`@mui/material`, `@emotion`).

### Path aliases

`tsconfig.json` defines the following (resolved at build time by `vite-tsconfig-paths`):

```
@app/api          @app/components   @app/containers   @app/dialogs
@app/forms        @app/hooks        @app/images       @app/services
@app/store        @app/types        @app/websocket    @app/generated/*
```

Prefer these in new code over relative imports when crossing top-level directory boundaries. Deep paths into a barrel target (e.g. `@app/store/game/...`) are a smell — add the symbol to the relevant `index.ts` barrel instead.

### End-to-end data flow

User action in a container → `src/api/*Service` → `src/websocket/commands/*` → `ProtobufService.send*Command` → socket.
Server reply/event → `src/websocket/events/*` (or the `command-options` callback on the original command) → `src/websocket/persistence/*` → `*.dispatch.ts` helpers → Redux / Dexie → selectors → container re-render.

## Build pipeline and generated files

`npm start` and `npm run build` both run `prestart`/`prebuild` hooks that invoke `proto:generate` and then `node prebuild.js`. `prebuild.js` does three things:

1. Copies shared country flag assets from `../cockatrice/resources/countries` into `src/images/countries`.
2. Writes `src/server-props.json` containing `REACT_APP_VERSION` = current `git rev-parse HEAD`.
3. Walks `src/**/*.i18n.json`, merges them into `src/i18n-default.json`, and **throws on duplicate keys** (`i18n key collision: ${key}`). Namespace your i18n keys — collisions fail the build.

Files you should never edit by hand (all auto-generated, all committed):

- `src/generated/proto/**`
- `src/i18n-default.json`
- `src/server-props.json`

If `npm start` seems to be ignoring a new `.i18n.json` file or a fresh proto, run `npm run proto:generate && node prebuild.js` directly — the hooks only fire on `start`/`build`, not on `test` or `lint`.

`.env.development`, `.env.production`, and `.env.test` exist but are empty. There is currently no env-var configuration surface; server URLs and the like are resolved through the login UI / `server-props.json`, not `import.meta.env`.

## Testing

Vitest + Testing Library + jsdom; `setupTests.ts` registers jest-dom matchers.

**Vitest runs with `test.isolate: false`.** Every spec file in a worker shares the same module graph, so `vi.mock(...)` factories and any mocks they create persist across tests. Consequences:

- The global `afterEach` in `setupTests.ts` calls `vi.clearAllMocks()` + `vi.restoreAllMocks()` + `vi.useRealTimers()`. It deliberately does **not** call `vi.resetAllMocks()`, because that would reset the implementations of `vi.fn()` instances created inside `vi.mock(...)` factories and break every spec that mocks `store.dispatch` once at file load.
- A test that installs a custom `mockReturnValue` / `mockImplementation` should not assume the next test resets it — either overwrite it or rely on `clearAllMocks` wiping only call histories.
- Always use real timers at the end of a test that switched to fake ones; the global teardown will catch leaks, but relying on it is fragile across files.

Other conventions:

- **Fixtures.** Store slices have co-located `__mocks__/fixtures.ts` files (notably `src/store/game/__mocks__/fixtures.ts`) exposing factories like `makeCard`, `makeGameEntry`, `makePlayerProperties`, `makeState`. They build protobuf messages via `create(Schema, overrides)`. Reuse them in new tests instead of hand-rolling proto objects.
- **Websocket mocks.** `src/websocket/__mocks__/` holds shared mock builders (e.g. `makeMockWebSocket`, `makeWebClientMock`, `makeSessionPersistenceMock`). Command and event specs install these with `vi.mock(...)` at the top of the file.
- **Slice tests are per-concern.** Each slice ships parallel `*.actions.spec.ts`, `*.reducer.spec.ts`, `*.selectors.spec.ts`, and `*.dispatch.spec.ts` files; tests don't cross concerns.

`npm run golden` (lint + test) is the CI gate — run it before declaring work done.

## Protocol changes

When a task requires editing `.proto` files in `../libcockatrice_protocol/`, run `npm run proto:generate` afterwards, and:

1. If the change introduces a new proto *file* that code outside `src/types/` needs to consume, add an `export *` line for it in `src/types/data.ts`.
2. Update any command/event/persistence code that consumes the changed messages.
3. Commit the regenerated files under `src/generated/proto/`.
