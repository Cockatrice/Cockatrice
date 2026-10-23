# GUI tests

Widget-level playback tests for the Cockatrice client, driven by
[QtTesting](https://github.com/Kitware/QtTesting) (`external/qttesting`, a submodule).
Each `*_test.xml` file is a list of events that QtTesting replays against the running
application; a test passes when every event is handled without error.

## Running

```bash
git submodule update --init external/qttesting      # once
cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug -DWITH_SERVER=0 -DWITH_GUI_TEST=ON
cmake --build build/debug -j 24

./Testing/run_test.sh --all                        # headless, offscreen
./Testing/run_test.sh Testing/smoke_test.xml       # one file
./Testing/run_test.sh --visual Testing/smoke_test.xml   # shows the window
```

`WITH_GUI_TEST` is on by default; a checkout without the submodule configures with a
warning and no harness. `COCKATRICE_BIN` points the runner at another build, and
`TEST_TIMEOUT` (seconds, default 180) bounds a single test — a failing event inside a
modal dialog can otherwise leave the process alive. The client also hard-exits a few
seconds after playback returns, whatever the result.

The tests are **not** part of `ctest`: they need a card database and a developer's
settings, so they stay a local loop for now.

## Caveats

- **Not hermetic.** The client reads the local settings and card database, so a test can
  pass on one machine and fail on another. Avoid asserting things that restored
  workspace state or user preferences decide (for example the number of open tabs).
- **English labels only.** Menu entries are addressed by their (translated) text, so the
  tests expect the source language. Widgets are addressed by objectName paths, which are
  stable across translations.
- Actions that open a dialog are followed by a `pause` event: playback continues in the
  dialog's own event loop, and the pause gives the UI time to build itself.
- Widget paths include Qt-internal names where a QTabWidget is involved
  (`qt_tabwidget_stackedwidget`), and the deck views in a game are indexed
  (`tabbedDeckView_0`, `tabbedDeckView_1`).

## Writing a test

A test is a `<events>` list. Widgets are named by their path from the main window, e.g.
`MainWindow/tabSupervisor/stackedWidget/TabGame/phasesMenu`. Useful commands:

| Command | Arguments | Effect |
|---------|-----------|--------|
| `activate` | action or menu name | triggers a QAction (menus are opened first) |
| `key` | Qt key code | presses that key on the focused widget |
| `pause` | milliseconds | waits, letting the UI settle |
| `clickCard` | `card,zone` | left click the card found in that zone |
| `flipCard` | `card,zone` | right-click → *Turn Over* |
| `cardAction` | `card,zone,label` | right-click → first menu entry whose label contains `label` (submenus included) |
| `moveCard` | `card,srcZone,dstZone` | drags the card onto the destination zone |
| `drawArrow` | `srcCard,srcZone,dstCard,dstZone` | right-drag an arrow between two cards |
| `createToken` | `name,color,pt` | creates a token as the active local player |
| `selectItem` | `item label` | selects the tree/list entry with that label |
| `typeText` | `text` | types text one key event at a time |
| `expectProperty` | `property,value` | fails playback unless the widget property matches |

The card, item and property commands are `pqCockatriceEventPlayer`
(`cockatrice/src/game_graphics/pq_cockatrice_event_player.h`). They resolve cards by name
and zone through the `QGraphicsScene` and send real mouse events at the card's current
viewport position, so a test does not depend on the window size or screen resolution.
