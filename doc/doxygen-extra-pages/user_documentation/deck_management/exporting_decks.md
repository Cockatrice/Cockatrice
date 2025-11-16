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
// Full Deck

// An example comment.

// 63 Maindeck
// 4 Ace
1 Ace of Clubs (PKR) 49
1 Ace of Diamonds (PKR) 50
1 Ace of Hearts (PKR) 51
1 Ace of Spades (PKR) 52

// 13 Face
1 Jack of Clubs (PKR) 53
1 Jack of Diamonds (PKR) 54
1 Jack of Hearts (PKR) 55
1 Jack of Spades (PKR) 56
1 King of Clubs (PKR) 57
1 King of Diamonds (PKR) 58
1 King of Spades (PKR) 60
1 Queen of Clubs (PKR) 61
1 Queen of Diamonds (PKR) 62
1 Queen of Hearts (PKR) 63
1 Queen of Spades (PKR) 64
2 King of Hearts (PKR) 59

// 44 Number
1 0 of Clubs (PKR) 1
1 0 of Diamonds (PKR) 2
1 0 of Hearts (PKR) 3
1 1 of Clubs (PKR) 11
1 1 of Diamonds (PKR) 12
1 1 of Hearts (PKR) 13
1 1 of Spades (PKR) 14
1 10 of Clubs (PKR) 5
1 10 of Diamonds (PKR) 6
1 10 of Hearts (PKR) 7
1 2 of Clubs (PKR) 17
1 2 of Diamonds (PKR) 18
1 2 of Hearts (PKR) 19
1 2 of Spades (PKR) 20
1 3 of Clubs (PKR) 21
1 3 of Diamonds (PKR) 22
1 3 of Hearts (PKR) 23
1 3 of Spades (PKR) 24
1 4 of Clubs (PKR) 25
1 4 of Diamonds (PKR) 26
1 4 of Hearts (PKR) 27
1 4 of Spades (PKR) 28
1 5 of Clubs (PKR) 29
1 5 of Diamonds (PKR) 30
1 5 of Hearts (PKR) 31
1 5 of Spades (PKR) 32
1 6 of Clubs (PKR) 33
1 6 of Diamonds (PKR) 34
1 6 of Hearts (PKR) 35
1 6 of Spades (PKR) 36
1 7 of Clubs (PKR) 37
1 7 of Diamonds (PKR) 38
1 7 of Hearts (PKR) 39
1 7 of Spades (PKR) 40
1 8 of Clubs (PKR) 41
1 8 of Diamonds (PKR) 42
1 8 of Hearts (PKR) 43
1 8 of Spades (PKR) 44
1 9 of Clubs (PKR) 45
1 9 of Diamonds (PKR) 46
1 9 of Hearts (PKR) 47
1 9 of Spades (PKR) 48
1 10 of Spades (PKR) 8
1 0 of Spades (PKR) 4


// 2 Joker
1 1 Joker (PKR) 10
1 2 Joker (PKR) 16
```

## From an online service

To export a deck to an online service, select the "Send deck to online service..." action in the "Deck Editor" or "
Visual Deck Editor" menu in the application menu bar at the top of the screen.

Selecting this action will open your browser with the selected service open and the deck list information from the
client supplied to it.

Currently supported services are DeckList and TappedOut.