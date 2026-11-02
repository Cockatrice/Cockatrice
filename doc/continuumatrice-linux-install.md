# Running Continuumatrice alongside Cockatrice (Linux)

Continuumatrice is an independent application. You can keep normal upstream **Cockatrice for MTG** installed and run both without mixing profiles or overwriting executables.

## Binary and build target

The CMake target remains `cockatrice` to avoid breaking its internal dependencies, resource generators, translations, and existing build commands, but on **Linux** the target's output filename is `continuumatrice`.

```bash
git pull --ff-only origin feature/continuumatrice-zone-bands
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DWITH_CLIENT=ON -DWITH_SERVER=OFF -DWITH_ORACLE=OFF
cmake --build build --target cockatrice --parallel 16
./build/cockatrice/continuumatrice
```

If you previously compiled the fork under the old name, `build/cockatrice/cockatrice` can remain as an **outdated, stale executable** in the build directory. Launch `build/cockatrice/continuumatrice` instead. Do not remove your system's normal `/usr/bin/cockatrice` application.

The renamed executable on Linux installs as `bin/continuumatrice`. The new `continuumatrice.desktop` launcher installs alongside the normal `cockatrice.desktop`. Its application name and desktop-file ID are also `continuumatrice`, so Wayland and desktop launchers distinguish the applications.

The fork installs its own `continuumatrice.svg` and `continuumatrice.png` **filenames** for icon lookup, currently reusing the existing Cockatrice artwork until a dedicated brand icon is designed. The launcher names are independent even when the graphics resemble one another.

This fork intentionally does **not** install Cockatrice's `cockatrice-cod.xml` MIME handler or register the `cockatrice://` URL scheme in the new Linux desktop file. The normal MTG client keeps its default deck file and URL associations.

## Data isolation

Assuming typical XDG locations (and no `portable.dat` file):

- Data and databases: `~/.local/share/Continuumatrice/`
- Decks: `~/.local/share/Continuumatrice/decks/`
- Card images: `~/.local/share/Continuumatrice/pics/`
- Custom XML: `~/.local/share/Continuumatrice/customsets/`
- Preferences: `~/.local/share/Continuumatrice/settings/`
- Cache: `~/.cache/Continuumatrice/`
- Linux shared translations: `<install-prefix>/share/continuumatrice/translations`
- Linux shared themes: `<install-prefix>/share/continuumatrice/themes`

Its per-user single-instance local socket uses a Continuumatrice-specific name.

## System installation (optional)

If you want a separate `continuumatrice` command and a graphical launcher, first configure the build with a prefix of your choice (e.g., `/usr/local` for a system install). Then:

```bash
sudo cmake --install build
```

This uses the build's existing `CMAKE_INSTALL_PREFIX`; check `build/CMakeCache.txt` if unsure. The Linux client-specific install rules do not replace `bin/cockatrice` or `cockatrice.desktop`.

For immediate testing, skip installation and run the executable directly from the build tree. This change has not been compiled in this environment; verify with your openSUSE compiler.

## Cross-platform status

This Linux naming change is intentionally scoped to non-Apple Unix. Upstream packaging and installer flows for Windows/macOS still need separate renaming work before those platforms can claim fully isolated installations.
