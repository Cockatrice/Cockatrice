@page importing_decks Importing Decks

# Where to import?

There are three screens in the client which can be used to import decks, depending on the context.

- The deck editor tab
- The pre-game lobby tab
- The deck storage tab (not to be confused with the visual deck storage tab)

# The Deck Editor Tab

The deck editor tabs (Classic and Visual) offer three ways of importing a deck:

- From a file on your local storage
- From your clipboard
- From an online service

## Local File Storage

To load a deck from a file on your local storage, select the "Load Deck" action in the "Deck Editor" or "Visual Deck
Editor" menu in the application menu bar at the top of the screen. Alternatively, you can use the shortcut Ctrl/Cmd + O
to access this action.

Selecting this action will open a file picker dialog provided by your operating system. Simply select a supported file
here and it will be loaded.

Just below the "Load Deck" action described above is the "Load recent deck" option, which keeps a record of the last 10
loaded decks for quick access.

## From Clipboard

To load a deck from your clipboard, select the "Load deck from clipboard..." action in the "Deck Editor" or "Visual Deck
Editor" menu in the application menu bar at the top of the screen. Alternatively, you can use the shortcut Ctrl/Cmd +
Shift + V to access this action.

Selecting this action will open a new text editor dialog with the contents of your clipboard pasted inside it.

The import dialog expects each line to be a card with the following format:

TODO

Each card should be on a separate line and there should be no empty lines between cards. The first empty line between
two blocks of cards will be considered as the divider between mainboard and sideboard.

Selecting "Parse Set Name and Number (if available)" will automatically parse these options and attempt to resolve them
to valid provider IDs found in the card database. If this option is unselected, Cockatrice will import all cards as
versions without provider IDs, which means they will display to everyone according to their own user defined set
preferences, rather than being the same defined printing for everyone.

## From an online service

To load a deck from an online service, select the "Load deck from online service..." action in the "Deck Editor" or "
Visual Deck Editor" menu in the application menu bar at the top of the screen.

Selecting this action will open a dialog containing the contents of your clipboard pasted into it. If your clipboard
currently contains a supported URL, the dialog will accept it and close on its own, otherwise you may adjust the URL and
confirm.

The action will automatically import the deck from the online service without any other required user action.

Currently supported services are Archidekt, Deckstats, Moxfield, and TappedOut.