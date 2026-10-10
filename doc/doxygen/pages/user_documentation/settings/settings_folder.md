@page settings_folder Settings Folder and Backups

Cockatrice keeps all of its preferences in a settings folder on your computer, one `.ini` file per
group of settings (appearance, sound, decks, and so on). Open it from the main window via
Help → Open Settings Folder.

# The Per-File Migration

Older versions of Cockatrice stored every setting in a single `global.ini` file. The settings were
split into per-group `.ini` files so that unrelated groups no longer share one file. The first time
you start a version that uses the new layout, your settings are moved into the new files
automatically and nothing is lost. The old `global.ini` is kept next to the new files as
`global.ini.old`.

# Automatic Backups

Both migrations overwrite files in the settings folder in place, so before either one runs,
Cockatrice copies the whole folder to a timestamped backup folder:

```
<data folder>/settings-backup-20261002-193000/
```

The backup folder sits next to the settings folder, not inside it. Every backup is kept, so if
settings ever end up wrong you can compare against, or copy files back from, the earlier snapshot.
Backups are only created when a migration actually has work to do, so a normal start creates
nothing.

# Downgrading

Settings migrations only run forwards. If you ever go back to an older version of Cockatrice, that
version does not read the new per-group files, so it will start with default settings.

Before downgrading, copy your settings folder somewhere safe, for example:

-   Linux: `~/.local/share/Cockatrice/settings/`
-   macOS: `~/Library/Application Support/Cockatrice/settings/`
-   Windows: `%LOCALAPPDATA%\Cockatrice\settings\`

Portable installs keep the settings folder in the `data` folder next to the executable.

Copying that folder back after downgrading restores your settings. The automatic backup folder from
the migration is there as a second copy of the same data if you need it.