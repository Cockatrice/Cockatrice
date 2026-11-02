# Hosting Continuum-Servatrice

**Continuum-Servatrice** is Continuumatrice's dedicated multiplayer server. It is built from the existing Servatrice C++ implementation, including accounts, lobbies, rooms, replays, game synchronization, TCP and WebSocket networking, and MySQL support. It also registers the three additional public pile zones: Limbo, Refuge, and Oblivion.

You may run it alongside upstream Servatrice, but **never reuse the MTG server's database, schema, account tables, process name, or service configuration.**

## Build client and server together

On Linux with the existing development dependencies, from the repository root:

```bash
cmake -S . -B build --fresh -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DWITH_CLIENT=ON -DWITH_SERVER=ON -DWITH_ORACLE=OFF

cmake --build build --target continuumatrice continuum-servatrice --parallel 16
```

Executable paths:

- Client: `build/continuumatrice/continuumatrice`
- Dedicated server: `build/continuum-servatrice/continuum-servatrice`

The inherited source directories `cockatrice/` and `servatrice/` remain for source compatibility. The **binary build directories and targets are distinct**.

`WITH_SERVER` is `ON` by default; use `-DWITH_CLIENT=OFF -DWITH_SERVER=ON` for a dedicated server-only build (e.g., on a Linux host). This project deliberately excludes the upstream Magic Oracle importer.

## New independent MySQL schema

The repository includes `servatrice/continuum-servatrice.sql`. This is **not** the old `servatrice.sql`: its table identifiers use the `continuum_servatrice_` prefix matching the fork server configuration. Do not apply these SQL scripts to a production MTG Servatrice database.

Create a separate database and limited account using MySQL. The following is an example for local MySQL (choose a real, strong password and do not commit it):

```sql
CREATE DATABASE continuum_servatrice CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
CREATE USER 'continuum_servatrice'@'localhost' IDENTIFIED BY 'REPLACE_WITH_A_STRONG_PASSWORD';
GRANT ALL PRIVILEGES ON continuum_servatrice.* TO 'continuum_servatrice'@'localhost';
```

Execute those statements as a MySQL administrator, for example `mysql -u root -p`. Then initialize the *empty* database:

```bash
mysql -u root -p continuum_servatrice < servatrice/continuum-servatrice.sql
```

This schema contains user and room tables; it is not the card catalogue. Card XML and images live on the client, and users joining the same game should use compatible Continuumatrice card databases.

**Schema lifecycle:** The fork's new base SQL is isolated. Upstream schema migration scripts under `servatrice/migrations/` have not all been converted to the new prefix. Review/adapt each future migration against a **backup** of the fork database; never run the upstream migration scripts blindly on either product.

## Configure server

```bash
mkdir -p ~/.config/continuum-servatrice
cp servatrice/continuum-servatrice.ini.example ~/.config/continuum-servatrice/continuum-servatrice.ini
chmod 600 ~/.config/continuum-servatrice/continuum-servatrice.ini
```

Edit the private INI file, at least:

```ini
[server]
name=Continuum-Servatrice
host=any
port=4757
websocket_host=any
websocket_port=4758

[database]
type=mysql
prefix=continuum_servatrice
hostname=localhost
database=continuum_servatrice
user=continuum_servatrice
password=REPLACE_WITH_YOUR_MYSQL_PASSWORD
```

Those are the **fork's example ports** (TCP 4757, WebSocket 4758), separate from the usual upstream 4747/4748. The server supports other ports. If you run multiple game servers on the same host, explicitly choose distinct bound ports and independently configured databases.

To start in the terminal:

```bash
./build/continuum-servatrice/continuum-servatrice \
  --config "$HOME/.config/continuum-servatrice/continuum-servatrice.ini" \
  --log-to-console
```

A successful start requires database connectivity, a compatible schema, and available ports. Check the server's diagnostics rather than assuming a process that exits is running correctly.

For Internet hosting, use firewall rules scoped to the ports you intend to expose, configure TLS / a secure reverse proxy for WebSockets, and avoid exposing MySQL directly to the Internet. The sample file has inherited settings for authentication, room permissions, email, and moderation; review them before public launch.

## Client/server compatibility

Continuumatrice supports standalone local games without a dedicated server. For remote games, use a **matching Continuum-Servatrice build**. Ordinary upstream Cockatrice servers don't define the `limbo`, `refuge`, or `oblivion` zone IDs.

## Production verification

- [ ] Client and server both compile from the same Git commit.
- [ ] They create `build/continuumatrice` and `build/continuum-servatrice` with correct filenames.
- [ ] Server starts with `--config` and can connect to the **independent** MySQL schema.
- [ ] Account registration/authentication, room joining, game creation and reconnect work.
- [ ] Cards move correctly into Limbo, Refuge and Oblivion for both players and after reconnect.
- [ ] TCP and WebSocket connect on the configured ports.
- [ ] No accesses to upstream MTG databases, profile directories, sockets, or HTTP update services.
- [ ] No migration script alters a production database without backup/review.
- [ ] Service hardening, logging, TLS and restart management are in place before public launch.

**Status:** Committed source changes; dedicated server compilation, MySQL initialization, and runtime validation still need to be performed on the deployment system.
