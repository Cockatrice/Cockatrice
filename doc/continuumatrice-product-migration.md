# Continuumatrice product migration and coexistence

This is a **product fork**, not a renamed Cockatrice binary. The upstream Cockatrice runtime, MTG cards, profiles, update feeds, launchers and installers must remain independent.

## Runtime identity

| Component | Continuumatrice |
| --- | --- |
| CMake target / installed binary | `continuumatrice` |
| Linux build output | `build/continuumatrice/continuumatrice` (source folder remains upstream named) |
| Linux app launcher | `continuumatrice.desktop` |
| Qt application | `Continuumatrice` |
| Qt organization | `Continuum Universes Studio` |
| User data on Linux (default XDG) | `~/.local/share/Continuumatrice/` |
| User cache on Linux | `~/.cache/Continuumatrice/` |
| Single-instance socket | `ContinuumatriceSingleInstance` |
| Deep-link scheme | `continuumatrice://` |
| Card XML root (native) | `continuumatrice_carddatabase` version `4` |
| Card XML root (import compatibility) | `cockatrice_carddatabase` version `4` |
| Fork server executable (when built) | `continuum-servatrice` |
| Fork server config | `continuum-servatrice.ini` |

Both `<cockatrice_carddatabase version="4">` and `<continuumatrice_carddatabase version="4">` load, but new XML saved by the fork uses the new root. The parser format and gameplay features still derive from the Cockatrice codebase; this change is not an automatic rules engine.

The source directories `cockatrice/` and `servatrice/`, common `libcockatrice_*` targets, and historical class names are left as *internal* implementation details to avoid needless churn and protocol breakage. They do not identify the installed product.

## Card data

- A new profile receives a **valid empty XML v4 starter database**, with no MTG cards.
- Put Continuum Universes card sets in `~/.local/share/Continuumatrice/customsets/`, or import from the Card Database menu.
- The default image download URL list is empty. Older fork profiles have known Scryfall/Gatherer URLs removed while custom URLs are preserved.
- The upstream MTG Oracle importer is not shipped: `WITH_ORACLE=OFF` is the default and `WITH_ORACLE=ON` is deliberately refused until there is a replacement.
- The inherited first-run Magic importer and automatic public-server/Commander-bracket fetches are disabled.
- Application release checks point only to the Continuumatrice GitHub repository; automatic update checks are off until a fork release is available.

**An independent Continuum Universes catalogue importer remains to be implemented.** Static XML v4 import is already available.

## System integration

Linux: `continuumatrice` binary, independent .desktop file, own SVG icon and `continuumatrice://` handler. No takeover of `.cod` or `cockatrice://`.

Windows: packaging script uses a separate installation directory, uninstall registry key, executable, URL protocol and process-lock detection. It must **never** remove or overwrite `cockatrice.exe` or the upstream `Uninstall\Cockatrice` key.

macOS: separate app executable/bundle identifier and `continuumatrice://` link handling; `.cod` handler removed to avoid competition with Cockatrice.

The Windows and macOS installers are **not yet verified**. Source and packaging changes should not be presented as validated shipping installers.

## Build

```bash
cmake -S . -B build --fresh -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DWITH_CLIENT=ON -DWITH_SERVER=ON -DWITH_ORACLE=OFF
cmake --build build --target continuumatrice continuum-servatrice --parallel 16
./build/continuumatrice/continuumatrice
```

An internal compatibility target `cockatrice` still depends on `continuumatrice` to accommodate older CI invocations, but never emits a `cockatrice` executable. Previously compiled copies of `build/cockatrice/cockatrice` are stale.

## Release checklist

- [ ] Configure and compile Continuumatrice with openSUSE Qt 6 using a clean CMake cache.
- [ ] Confirm `build/continuumatrice/continuumatrice` runs and the app name and original SVG icon appear.
- [ ] Confirm old `/usr/bin/cockatrice`, `~/.local/share/Cockatrice` and MTG card images remain untouched.
- [ ] Open both clients simultaneously; confirm distinct sockets, settings, card databases, decks and caches.
- [ ] Load both legacy and new-root XML and verify custom images and Front/Support/Command mapping.
- [ ] Test Limbo, Refuge and Oblivion card moves, view windows, counters, mirrored positions and reconnect.
- [ ] Confirm startup does not request MTG artwork, Oracle updates, Commander brackets, or upstream server lists.
- [ ] Check a `continuumatrice://` link and ensure `cockatrice://` still belongs to MTG Cockatrice.
- [ ] Install and uninstall user-local Linux app; ensure no MTG desktop-file or icon collision.
- [ ] Test Windows installer in a VM with upstream Cockatrice preinstalled, including install, upgrade, uninstall and `.cod` association preservation.
- [ ] Test macOS app bundle identity and installation alongside upstream Cockatrice.
- [ ] Verify GPL requirements, notice retention and upstream attribution.

**Status:** Committed source changes; no completed Qt compilation, package build, or runtime validation in the assistant environment. Release is blocked pending these tests.

## Continuum-Servatrice dedicated server

CMake uses `add_subdirectory(servatrice "${CMAKE_BINARY_DIR}/continuum-servatrice")`, retaining the inherited source files but placing generated build files and the executable in their own product folder. `WITH_SERVER=ON` is the default; `WITH_SERVER=OFF` is a deliberate opt-out for client-only builds. 

The MySQL schema `servatrice/continuum-servatrice.sql` uses `continuum_servatrice_` table prefixes, paired with `servatrice/continuum-servatrice.ini.example`. Default listening ports are 4757/4758. Existing Servatrice implementations, account/room/networking functionality and extended Limbo/Refuge/Oblivion game zones are retained.

The fork server should be hosted against an **independent MySQL database**, not an existing upstream MTG Servatrice database. See [hosting instructions](continuum-servatrice-hosting.md).
