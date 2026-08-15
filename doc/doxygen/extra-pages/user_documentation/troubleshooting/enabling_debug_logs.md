@page enabling_debug_logs Enabling Debug Logs

Cockatrice ships with a "diagnostics mode" that prints detailed debug messages about what the client is doing. This is
extremely useful when asking for help, because it shows exactly what happened: which URL the card picture loader tried,
whether it found or missed a file on disk, whether a download succeeded or failed, which redirects were followed, and
much more. Each of these messages belongs to a category, and you can enable or disable categories individually.

Don't worry, this sounds more technical than it is. You only need to do two things: create one small text file, and tell
Cockatrice where it is. There are no installation steps and you can undo everything later (see [When you are
done](#when-you-are-done)).

# Step 1: Create the file

Open a plain text editor (Notepad on Windows, TextEdit on macOS, or any text editor on Linux) and paste the following
content:

```ini
[Rules]
# The default log level is info
*.debug = false

# Turn on debug level logs for the card picture loader and all its sub categories
card_picture_loader.* = true
```

Save the file with the exact name `qtlogging.ini` in a place you can find again, for example your Documents folder.

\attention The file name matters: it must be `qtlogging.ini`, not `qtlogging.ini.txt`. If your text editor adds a
`.txt` extension automatically, you need to stop it from doing so (see below). On macOS, TextEdit must be switched to
plain text mode first via 'Format → Make Plain Text'.

The file contains one rule per line. The `*.debug = false` rule turns off debug messages everywhere by default, and the
`card_picture_loader.* = true` line then re-enables them for the card picture loader. The `.*` at the end means "this
category and all of its sub categories". To enable a different category instead, just replace that line with the
category name of your choice, for example `card_database.loading = true` or `window_main.startup = true`.

# Step 2: Tell Cockatrice where the file is

Cockatrice does not know about the file yet. You have to point it there by setting an environment variable called
`QT_LOGGING_CONF` to the full location of your file. How to do this depends on your operating system:

**Windows**

1. Press the Windows key, type "environment variables", and open "Edit the system environment variables".
2. Click "Environment Variables...", then under "User variables" click "New...".
3. Set "Variable name" to `QT_LOGGING_CONF` and "Variable value" to the full path of your file, for example
   `C:\Users\YourName\Documents\qtlogging.ini`.
4. Confirm all dialogs, then close and reopen Cockatrice.

Alternatively, if Cockatrice is installed in a folder you can write to, you can simply place the `qtlogging.ini` file
directly next to the Cockatrice executable (in the same folder as `cockatrice.exe`) and skip the environment variable
altogether. Note that this copy may be replaced when you update the client.

**macOS**

Open the Terminal app (it is in 'Applications → Utilities') and run the following two commands, replacing the path
with the full location of your file:

```text
launchctl setenv QT_LOGGING_CONF /path/to/qtlogging.ini
open -a Cockatrice
```

The setting stays active until you log out or restart your Mac. If you have multiple users on the same Mac, be aware
that this setting only applies to your user account.

**Linux**

For a quick test, open a terminal and start Cockatrice with the file on the command line, replacing the path with the
full location of your file:

```text
QT_LOGGING_CONF=/path/to/qtlogging.ini cockatrice
```

If this works and you want it to apply every time you start Cockatrice, add the following line to your `~/.profile`
file and log in again:

```text
export QT_LOGGING_CONF="/path/to/qtlogging.ini"
```

# Step 3: See the logs

Now that debug logging is enabled, open Cockatrice and trigger the behavior you are investigating, for example by
opening a deck, reloading the card database, or starting a game.

The easiest way to see the logs is to use the built-in log viewer inside Cockatrice itself: open 'Help → View Debug
Log'. A window appears that shows the log messages live and keeps the most recent entries. It even has a 'Copy to
clipboard' button so you can paste the output into a bug report or a Discord message. This works the same on every
operating system.

If you prefer to capture everything to a file instead, start Cockatrice with the `--debug-output` option:

```text
cockatrice --debug-output
```

Cockatrice then writes the full log to a file called `qdebug.txt` in the folder it was started from.

# Which categories are available?

Every message Cockatrice logs belongs to a category. The following table lists the most useful ones for troubleshooting,
grouped by area. Enable a category by adding a line like `category = true` to your `qtlogging.ini` file (or use a `.*`
suffix, e.g. `card_picture_loader.*`, to include all sub categories).

| What you want to see | Categories |
|----------------------|------------|
| Card picture loading (URLs, local file hits/misses, downloads, redirects) | `card_picture_loader.*` |
| Card database loading and parsing | `card_database`, `card_database.loading`, `card_database.loading.success_or_failure`, `cockatrice_xml.*` |
| Card, set, and deck information | `card_info`, `card_list`, `deck_loader` |
| Startup sequence and update checks | `window_main.startup.*`, `release_channel`, `spoiler_background_updater` |
| User interface and themes | `theme_manager`, `sound_engine`, `flow_layout`, `flow_widget.*`, `pixel_map_generator`, `card_info_picture_widget` |
| Networking and servers | `local_client`, `remote_client`, `tapped_out_interface`, `servers_settings` |
| In-game logic | `player`, `game_scene.*`, `card_zone.*`, `view_zone`, `game_event_handler` |
| Dialogs and tabs | `dlg_settings`, `dlg_update`, `dlg_tip_of_the_day`, `tab_game`, `tab_message`, `tab_supervisor` |
| Settings and shortcuts | `settings_cache`, `shortcuts_settings` |
| Deck and card filtering | `filter_string`, `deck_filter_string`, `syntax_help` |

For example, to investigate why a card database update seems to fail, enable the card database categories:

```ini
[Rules]
*.debug = false

card_database = true
card_database.loading = true
card_database.loading.success_or_failure = true
cockatrice_xml.* = true
```

# When you are done

To turn the diagnostics back off, just reverse what you did: remove the `QT_LOGGING_CONF` environment variable (or
unset it again via `launchctl unsetenv QT_LOGGING_CONF` on macOS) and/or delete the `qtlogging.ini` file, then restart
Cockatrice. Leaving it on is harmless, but the extra logging can make the client slightly slower.

For the full details on how Cockatrice logging works, including the complete list of categories, see @ref logging.
