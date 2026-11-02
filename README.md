# Continuumatrice

**Continuumatrice** is a standalone, open-source desktop card-game client for **Continuum Universes**, developed by **Continuum Universes Studio** and based on the Cockatrice C++/Qt project.

It is **not** your normal Magic: The Gathering Cockatrice installation. Continuumatrice has an independent executable, application ID, Linux launcher, card database, deck directory, settings, cache, window branding, and URL scheme. It also supports Limbo, Refuge, and Oblivion as additional card piles.

> **Status:** Active fork development. Features and packaging changes in draft pull requests are not released or validated until built and tested.

## What's different

- Front, Support, Command battlefield bands with theme overrides
- Limbo, Refuge, and Oblivion as real server-backed public piles
- Independent profile: `~/.local/share/Continuumatrice` and `~/.cache/Continuumatrice` by default on Linux
- Independent URL handler: `continuumatrice://` (does not claim `cockatrice://`)
- Own app icon, name and install identity; regular Cockatrice remains untouched
- No built-in Magic card import or Scryfall download list by default
- Card data supplied by your own Cockatrice XML v4-compatible files and images; the XML schema is still inherited from Cockatrice
- Matching fork client/server required for the three new pile-zone names

**Game rules are not automatically enforced** by the generic tabletop client. These piles are functional card containers, not an implementation of Continuum Universes rules themselves.

## Build on openSUSE Tumbleweed

Install development packages for Qt 6, Protobuf, a C++20 compiler, CMake and Ninja. Refer to [Linux installation notes](doc/continuumatrice-linux-install.md) for the dependency setup.

```bash
git clone https://github.com/Continuum-Universes-Studio/Continuumatrice.git
cd Continuumatrice

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DWITH_CLIENT=ON -DWITH_SERVER=ON -DWITH_ORACLE=OFF

cmake --build build --target continuumatrice continuum-servatrice --parallel 16
./build/continuumatrice/continuumatrice
```

The `cockatrice` CMake target is a backwards-compatible **build alias**, not the generated executable. An old `build/cockatrice/cockatrice` binary may still exist if you compiled an earlier version: it is stale, not Continuumatrice.

To install under an appropriate prefix:

```bash
cmake --install build
```

Inspect `CMAKE_INSTALL_PREFIX` before installing, particularly on a machine that already has Cockatrice. The fork does not install or remove `/usr/bin/cockatrice` or the upstream `cockatrice.desktop`.

## Card catalogue

The fork accepts XML version 4 for the time being. An empty `cards.xml` is created on a fresh profile; store your Continuum Universes sets in `~/.local/share/Continuumatrice/customsets/`, then reload the card database.

Images live under `~/.local/share/Continuumatrice/pics/`. Custom graphics and themed pile icons are supported; no MTG catalogue is required.

- [Card XML and assets](doc/continuumatrice-card-data.md)
- [Special piles and theme icons](doc/continuumatrice-special-piles.md)
- [Battlefield bands](doc/continuumatrice-battlefield.md)
- [Linux coexistence with Cockatrice](doc/continuumatrice-linux-install.md)

## Independent product boundary

The fork **must not** use upstream Cockatrice software-update releases, automatically import Magic card databases, register `.cod` as its own file type, or interfere with the original Cockatrice installation. A native Continuumatrice card importer and full multi-platform release validation are separate engineering work.

Legacy class names, the `cockatrice/` source directory, `libcockatrice_*` internal libraries, Protobuf protocol, and inherited card XML root element remain for source and wire compatibility; they do not represent the installed product name.

## License and upstream attribution

Continuumatrice is derived from [Cockatrice](https://github.com/Cockatrice/Cockatrice), originally developed by the Cockatrice contributors. Existing copyright headers, contribution history and upstream license obligations are retained. See [LICENSE](LICENSE). The Continuumatrice modifications are maintained by Continuum Universes Studio.

This is an independent fork, **not** an official Cockatrice release.

## Host Continuum-Servatrice

This repository includes the full dedicated multiplayer server, **Continuum-Servatrice**, built with the `continuum-servatrice` CMake target (default `WITH_SERVER=ON`). Its build output is `build/continuum-servatrice/continuum-servatrice`. Use the standalone `continuum_servatrice` MySQL database and the provided server-specific SQL schema/config; do not reuse MTG Servatrice's database or tables.

[Dedicated server hosting and database instructions](doc/continuum-servatrice-hosting.md)
