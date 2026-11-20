# Local development servers

This page covers the two supported ways to run a local <kbd>Servatrice</kbd> for
manual client and server work:

- **Guest profile** — no database, no mail server, two rooms and chat. Best for
  client UI work, room/chat flows, and quick server regressions.
- **Seeded MySQL stack** — real accounts, staff roles, moderation fixtures, and
  SQL-backed rooms. Best for authentication, permissions, reports, replay, and
  migration work.

Both paths run the binaries from your local build. Neither changes production
configuration.

## Build the server and client

From the repository root:

```bash
cmake -S . -B build/test -DCMAKE_BUILD_TYPE=Debug -DWITH_SERVER=1 -DTEST=1
cmake --build build/test -j 24
```

`build/test` contains both `servatrice` and `cockatrice`. `dev-servatrice.sh`
prefers `build/test`; `dev-client.sh` prefers the client-only `build/debug` tree
and falls back to `build/test`.

## Guest profile (no database)

Start the server:

```bash
./scripts/dev-servatrice.sh
```

The script runs `servatrice/dev.ini`, which listens on `127.0.0.1:4747`, accepts
guest logins, and exposes the **General** and **Testing** rooms with the
`general`, `moderators`, `developer-news`, and `testing` channels.

Start one or more clients in separate terminals:

```bash
./scripts/dev-client.sh
./scripts/dev-client.sh --profile second --connect devplayer2@127.0.0.1:4747
```

Each profile gets isolated `XDG_DATA_HOME`, `XDG_CACHE_HOME`, and
`XDG_CONFIG_HOME` directories under `build/dev/profiles/`, so profiles do not
share settings or card databases. A second plain launch runs independently
rather than handing off to the first. Delete a profile directory to reset it.

Useful script options:

```bash
./scripts/dev-servatrice.sh --port 14747
./scripts/dev-servatrice.sh --config /path/to/servatrice.ini
./scripts/dev-client.sh --no-connect --profile scratch
```

Any argument the scripts do not recognise is passed through to the binary
unchanged.

## Seeded MySQL stack

The stack is defined in `docker-compose.dev.yml`. On first start, MySQL applies
`servatrice/servatrice.sql` and then `servatrice/dev/seed.sql`.

```bash
./scripts/dev-db.sh up
./scripts/dev-servatrice.sh --mysql
```

Then connect with any seeded account. The password for every seeded account is
`password`:

| Account      | Role                          |
| ------------ | ----------------------------- |
| `devadmin`   | admin (implies moderator)     |
| `devmod`     | moderator                     |
| `devjudge`   | judge                         |
| `devdev`     | developer                     |
| `devplayer`  | registered player             |
| `devplayer2` | registered player             |

`--mysql` exports `DATABASE_URL`, which Servatrice already honours: it selects
MySQL, forces the `cockatrice` table prefix and SQL-backed rooms, and switches
authentication to `sql`. No ini edit is needed.

Seeded fixtures also include:

- rooms `General` and `Testing`, their game types, and the same channels as the
  guest profile;
- an open report and a resolved report, with a moderator comment;
- a warning against `devplayer2` and a permanent ban on `192.0.2.10`;
- a buddy/ignore entry and one deck in `devplayer`'s deck storage;
- two chat log rows in `General`.

### Managing the database

```bash
./scripts/dev-db.sh up      # start MySQL, wait for health
./scripts/dev-db.sh seed    # re-apply the fixtures
./scripts/dev-db.sh reset   # drop the volume and start fresh
./scripts/dev-db.sh stop    # stop containers, keep the volume
./scripts/dev-db.sh url     # print DATABASE_URL
```

The schema and seed are not re-run automatically on an existing volume. Use
`reset` when `servatrice/servatrice.sql` changes. Do not apply the schema file a
second time by hand: its `cockatrice_schema_version` insert is not idempotent.
The fixture file itself is idempotent and can be re-applied while iterating.

### External MySQL

Set `COCKATRICE_DEV_DATABASE_URL` to skip the compose stack:

```bash
export COCKATRICE_DEV_DATABASE_URL='mysql://user:password@127.0.0.1/database'
./scripts/dev-servatrice.sh --mysql
```

`up`, `seed`, and `reset` only manage the compose stack. For an external
database, apply `servatrice/servatrice.sql` and `servatrice/dev/seed.sql`
yourself.

> [!NOTE]
> Servatrice reads the host, database, user, and password from `DATABASE_URL`,
> but not its port: MySQL connections use port 3306. Keep the compose stack on
> 3306 or configure the database through the ini file instead.

## Per-run configuration overrides

`servatrice` accepts `--set key=value` (repeatable) and `--print-config`:

```bash
./scripts/dev-servatrice.sh --port 4747 -- --set security/max_users_per_address=50
./build/test/servatrice/servatrice --config servatrice/dev.ini --print-config
```

Overrides live only in the running process; the ini file is never written.
`--print-config` includes override-only keys and redacts values whose key
contains `password`, `secret`, or `legacy_backup_key`.

## Troubleshooting

- **Port already in use** — pass `--port` to the server and `--connect` to the
  clients, or stop the other instance.
- **MySQL port 3306 already in use** — stop the other MySQL instance, or run the
  server against it by setting `database/*` in a custom ini file. `DATABASE_URL`
  does not carry a port.
- **Client opens with stale data** — remove `build/dev/profiles/<name>/`.
- **Server cannot open the database** — the compose volume may predate a schema
  change; run `./scripts/dev-db.sh reset`.
- **`servatrice` target missing** — the client-only `build/debug` tree does not
  contain it; build `build/test` with `-DWITH_SERVER=1`.
