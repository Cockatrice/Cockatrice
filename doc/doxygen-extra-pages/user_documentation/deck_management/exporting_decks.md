@page exporting_decks Exporting Decks

# Where to export?

There are two screens in the client which can be used to import decks, depending on the context.

- The deck editor tab
- The deck storage tab (not to be confused with the visual deck storage tab)

# The Deck Editor Tab

The deck editor tabs (Classic and Visual) offer three ways of export a deck:

- To a file on your local storage
- To your clipboard
- To an online service

## Local File Storage

To save a deck to a file on your local storage, select the "Save Deck" action in the "Deck Editor" or "Visual Deck
Editor" menu in the application menu bar at the top of the screen. Alternatively, you can use the shortcut Ctrl/Cmd + S
to access this action.

Selecting this action will open a file picker dialog provided by your operating system. Simply enter a file name and
select a format (.cod is recommended) and confirm.

Just below the "Save Deck" action described above is the "Save Deck as..." option, which allows saving an existing file
under a different filename, which is useful for saving a different version or copy of a deck.

## From Clipboard

To save a deck to your clipboard, select the "Save deck to clipboard..." action in the "Deck Editor" or "Visual Deck
Editor" menu in the application menu bar at the top of the screen. Alternatively, you can use the shortcut Ctrl/Cmd +
Shift + C or Ctrl/Cmd + Shift + R to access this action.

Selecting this action will save the currently open deck list to your clipboard.

Saving the decklist without annotations will export the decklist, with each card being described in the following format

```
CARD_AMOUNT CARD_NAME (SET_SHORT_NAME) CARD_COLLECTOR_NUMBER
```

There is also the (no set info) option, which will simply export each card as

```
CARD_AMOUNT CARD_NAME
```

Mainboard and sideboard are delimited by a newline like so:

```
1 MainboardCard
1 OtherMainboardCard

1 SideboardCard
```

Saving the decklist as annotated will insert comments (marked with // in front of them).
It will first insert the name and any comments associated with the deck before separating each deck section into its own
newline delimited and annotated group.

Example: TODO: Adjust this to be non mtg based.

```
// Deck Name

// Deck Comment

// 10 Maindeck
// 6 Artifact
2 The Darkness Crystal (FIN) 335
2 The Fire Crystal (FIN) 337
2 Black Mage's Rod (FIN) 90

// 6 Sorcery
2 Nibelheim Aflame (FIN) 339
4 Cornered by Black Mages (FIN) 93

// 6 Sideboard
// 6 Creature
SB: 4 Blazing Bomb (FIN) 130
SB: 1 Garland, Knight of Cornelia (FIN) 221
SB: 1 Undercity Dire Rat (FIN) 123
```

## From an online service

To export a deck to an online service, select the "Send deck to online service..." action in the "Deck Editor" or "
Visual Deck Editor" menu in the application menu bar at the top of the screen.

Selecting this action will open your browser with the selected service open and the deck list information from the
client supplied to it.

Currently supported services are DeckList and TappedOut.