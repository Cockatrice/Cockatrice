# GUI tests

Widget-level playback tests for the Cockatrice client, driven by
[QtTesting](https://github.com/Kitware/QtTesting) (`external/qttesting`, a submodule).
Each `*_test.xml` file lists events that QtTesting replays against the running
application. A test passes when every event is handled without error.

## Running

```bash
git submodule update --init external/qttesting      # once
cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug -DWITH_SERVER=0 -DWITH_GUI_TEST=ON
cmake --build build/debug -j 24

./Testing/run_test.sh --all                        # headless, offscreen
./Testing/run_test.sh Testing/smoke_test.xml       # one file
./Testing/run_test.sh --visual Testing/smoke_test.xml   # shows the window
```

`WITH_GUI_TEST` is on by default. Without the submodule the build configures with a
warning and no harness. `COCKATRICE_BIN` points the runner at another build, and
`TEST_TIMEOUT` (seconds, default 180) bounds a single test, because a failing event
inside a modal dialog can leave the process alive. The client also hard-exits a few
seconds after playback returns, whatever the result.

The tests are **not** part of `ctest`. They need a card database and a developer's
settings, so they stay a local loop for now.

## Caveats

- **Not hermetic.** The client reads the local settings and card database, so a test can
  pass on one machine and fail on another. Avoid asserting things that restored
  workspace state or user preferences decide, such as the number of open tabs.
- **English labels only.** Menu entries are addressed by their translated text, so the
  tests expect the source language. Widgets are addressed by objectName paths, which are
  stable across translations.
- Actions that open a dialog are followed by a `pause` event. Playback continues in the
  dialog's own event loop, and the pause gives the UI time to build itself.
- `pause` sleeps the thread, so nothing is delivered while it runs. A reply from a server
  never arrives during a pause. Waiting for the client to talk to a server needs
  `process_events`, which runs the event loop (see *Screenshots* below).
- Widget paths include Qt-internal names where a QTabWidget is involved
  (`qt_tabwidget_stackedwidget`), and the deck views in a game are indexed
  (`tabbedDeckView_0`, `tabbedDeckView_1`).

## Writing a test

A test is an `<events>` list. Widgets are named by their path from the main window, e.g.
`MainWindow/tabSupervisor/stackedWidget/TabGame/phasesMenu`. Useful commands:

| Command | Arguments | Effect |
|---------|-----------|--------|
| `activate` | action or menu name | triggers a QAction (menus are opened first) |
| `key` | Qt key code | presses that key on the focused widget |
| `pause` | milliseconds | sleeps, letting the UI settle (no events are processed) |
| `process_events` | milliseconds | runs the event loop, so timers and network replies are handled |
| `clickCard` | `card,zone` | double click the card found in that zone (plays it from hand) |
| `flipCard` | `card,zone` | right-click → *Turn Over* |
| `cardAction` | `card,zone,label` | right-click → first menu entry whose label contains `label` (submenus included) |
| `moveCard` | `card,srcZone,dstZone` | drags the card onto the destination zone |
| `drawArrow` | `srcCard,srcZone,dstCard,dstZone` | right-drag an arrow between two cards |
| `createToken` | `name,color,pt` | creates a token as the active local player |
| `selectItem` | `item label` | selects the tree/list entry with that label |
| `typeText` | `text` | types text one key event at a time |
| `expectProperty` | `property,value` | fails playback unless the widget property matches |
| `screenshot` | `path` | saves an image of the addressed widget; `$VAR` expands from the environment |
| `dumpTree` | `path` | writes the addressed widget's subtree with the paths tests address |
| `resize` | `WIDTHxHEIGHT` | resizes the addressed widget's window |
| `selectRow` | `row[/row…][,column]` | selects a row of an item view through its selection model |
| `clickRow` | `row[/row…][,column]` | left-clicks a cell, so the view's own handlers run |
| `doubleClickRow` | `row[/row…][,column]` | double-clicks a cell |

The card, item and property commands are `pqCockatriceEventPlayer`
(`cockatrice/src/game_graphics/pq_cockatrice_event_player.h`). They resolve cards by name
and zone through the `QGraphicsScene` and send real mouse events at the card's current
viewport position, so a test does not depend on the window size or screen resolution.

The capture and row commands are `pqWidgetTestEventPlayer`
(`cockatrice/src/interface/pq_widget_test_event_player.h`). Rows are model rows of the
addressed view, so a filtered or sorted model counts the rows it shows. A tree addresses a
row by the path of rows leading to it: `2/0` is the first child of the third top-level
row. A click opens a collapsed path first.

## Assertions

`expectProperty` is an action. It fails playback unless a widget property matches.
QtTesting's own check commands assert model state instead, and they only run when the
event carries `type="check"`. Without it the event is an action, and a check command is
rejected as unknown:

```xml
<event widget="MainWindow/.../deckView" command="modelRowCount" arguments="1" type="check" />
```

`modelRowCount` compares `model()->rowCount()`. `modelItemData` compares the data of one
cell, addressed as `row.column,text` (`modelItemData "0.0,Lightning Bolt"`).

## Screenshots

`run_test.sh` sets `COCKATRICE_SCREENSHOT_DIR` to `build/screenshots`, and `screenshot`
and `dumpTree` write there. Override it in the environment to send captures elsewhere.
That directory is not part of the source tree, so a test run does not leave untracked
files in the repository:

```xml
<event widget="MainWindow" command="resize" arguments="1920x1080" />
<event widget="MainWindow" command="screenshot" arguments="$COCKATRICE_SCREENSHOT_DIR/main-window.png" />
<event widget="MainWindow" command="dumpTree" arguments="$COCKATRICE_SCREENSHOT_DIR/widget-tree.txt" />
```

The addressed widget is what gets grabbed, so `MainWindow` is the whole window and a
panel's path is that panel. The format comes from the file extension. A path with no
known extension is a test failure.

Capture at a fixed, generous size. `1920x1080` is a safe default, and narrower windows
clip columns of wide tables. A headless run renders through the offscreen platform, so
the palette and layout are the real ones, but `raise()`, keyboard-grab and
`propagateSizeHints()` warnings on stderr are normal.

### Waiting for a server

`pause` blocks, so a reply that has to arrive over the network never does. Use
`process_events` with a budget that covers the round trip:

```xml
<event widget="MainWindow" command="process_events" arguments="8000" />  <!-- login -->
<event widget="MainWindow" command="process_events" arguments="4000" />  <!-- report list -->
```

### Paths drift between runs

`pqObjectNaming` names an unnamed widget `<class><index>`, where the index counts
same-class unnamed siblings and a leading digit records visibility (`1` visible, `0`
hidden). The numbering depends on what the previous events left on screen, so a path that
worked in one playback can miss in the next. The player retries a leading `0` as `1`,
which resolves some hidden-page paths but not all of them.

Record the tree in the same playback that clicks, and copy paths from that dump:

```bash
./Testing/run_test.sh Testing/screenshot_test.xml
less build/screenshots/widget-tree.txt
```

The dialog instance matters too, not just the events before it: in
`feature_intro_test.xml` the settings dialog opened through the *What's New* deep link
numbers the user-interface scroll area `0QScrollArea5` while the one opened from the menu
numbers it `1QScrollArea0`, in the same process. A failed lookup prints a
`Possible match:` line with the path it found, so read that before editing the test.

Capture a widget that is scrolled out of view by addressing the widget itself rather than
the viewport (`.../0UserInterfaceSettingsPage0/0QGroupBox3`), not the scroll area it sits
in; a `grab()` renders the subtree even when the viewport would clip it.

### A prepared HOME for upgrade-state tests

A test that depends on what the install looked like *before* this run — a feature
introduction for someone updating from 3.1, a settings migration — cannot use the
developer's real settings, and a clean install is not enough either: on a clean install
the feature-intro planner baselines everything it has never seen, so nothing is due.

Give the client its own HOME. Four files are enough for the "3.1.0, one homemade card"
case:

| File | Contents |
|---|---|
| `network.ini` | `[network] clientVersion=3.1.0` — older than the version that introduces the feature |
| `updates.ini` | `startupUpdateCheck=false`, `newVersionNotification=false`, `startupCardUpdateCheckPromptForUpdate=false`, `alwaysEnableNewSets=false` |
| `paths.ini` | `cardDatabase=` an XML you control, plus the deck/pics/replay dirs under the same HOME |
| `cardDatabase.ini` | every set of that XML enabled **and** known: `[sets]` then `FI\enabled=true`, `FI\isKnown=true`, `FI\sortKey=1` |

The last row is not optional. `MainWindow::startupConfigCheck()` calls
`CardDatabase::checkUnknownSets()` on every non-clean install: enabled-but-unknown sets
raise the modal "new sets found" question, and *no* enabled set enables everything and
raises the modal "Welcome, all sets have been enabled" box followed by Manage Sets. Either
one opens before the first event and the playback never starts — the run exits on the
timeout with no failing event in the log, which reads like a hang in the harness. When a
whole playback produces no event output, attach a debugger instead of guessing:

```bash
gdb -p "$(pgrep -f 'cockatrice --no-connect --test')" -batch -ex "thread apply all bt"
# QDialog::exec ← showNewMessageBox ← MainWindow::cardDatabaseAllNewSetsEnabled is the
# startup modal, not your test
```

Run the playback directly, because `run_test.sh` deliberately does not set `HOME` and
always keeps the developer's settings. A fresh HOME per attempt: a killed run leaves the
crash-session lock and the next launch stops in the crash-report flow.

```bash
bash /tmp/opencode/fi-reset.sh   # writes the four files above
HOME=/tmp/opencode/feature-intro-home \
COCKATRICE_SCREENSHOT_DIR=/tmp/opencode/fi-shots \
QT_QPA_PLATFORM=offscreen \
timeout --foreground 120 build/test/cockatrice/cockatrice \
    --no-connect --test Testing/feature_intro_test.xml
```

The test file documents the same recipe in its header comment; keep the two in step.

### Selecting a row

QtTesting's own `currentChanged` takes `row|column`. `0:0` reaches `OldGetIndex`, which
calls `.at(1)` on a one-element list and aborts the process. Select through `selectRow`
(or click through `clickRow`) instead. Never use `currentChanged`.
