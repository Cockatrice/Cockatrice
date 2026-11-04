# Running Continuumatrice alongside Cockatrice (Linux)

Continuumatrice is an independent application. You can keep normal upstream **Cockatrice for MTG** installed and run both without mixing profiles or overwriting executables.

## Native CMake targets and output paths

Continuumatrice and its dedicated server use their own CMake target and executable names on every platform:

| Program | CMake target | Linux build output |
| --- | --- | --- |
| Continuumatrice | `continuumatrice` | `build/continuumatrice/continuumatrice` |
| Continuum-Servatrice | `continuum-servatrice` | `build/continuum-servatrice/continuum-servatrice` |

From the repository root on openSUSE:

```bash
git switch master
git pull --ff-only
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DWITH_CLIENT=ON -DWITH_SERVER=ON -DWITH_ORACLE=OFF
cmake --build build --target continuumatrice continuum-servatrice --parallel 16
./build/continuumatrice/continuumatrice
```

The inherited source directories remain `cockatrice/` and `servatrice/`, but the build directories, targets and installed executable names are independent. The old `cockatrice` and `servatrice` **CMake aliases** remain available for existing build scripts; they do not generate upstream-named executables.

Reconfigure the existing build directory after pulling these changes so Ninja regenerates its target list. An old `build/cockatrice/continuumatrice` or `build/servatrice/servatrice` file may remain from an earlier build; do not use it to verify the current target.

Linux installs `bin/continuumatrice` and `bin/continuum-servatrice`. The new `continuumatrice.desktop` launcher installs alongside normal MTG Cockatrice, rather than replacing `cockatrice.desktop`.

The app's Linux icon names are distinct from upstream's. The fork does **not** install Cockatrice's `cockatrice-cod.xml` MIME handler or register the `cockatrice://` URL scheme, so the two clients can coexist.

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

The native CMake targets, executable paths and packaging references have been restored from the previous Continuumatrice product-migration branch. Linux is the primary development platform; Windows and macOS packaging changes still require CI verification.
