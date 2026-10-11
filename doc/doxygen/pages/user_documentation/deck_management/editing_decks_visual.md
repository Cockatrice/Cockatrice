@page editing_decks_visual Visual Deck Editor

\image html vde_overlap_layout_type_grouped.png width=1200px

# Editing Basic Deck Information

Editing basic deck information is done through the deck dock widget (DeckEditorDeckDockWidget).

\image html deckeditordeckdockwidget.png

This widget allows editing:

- The name
- The comments
- The banner card, which is used to represent the deck in the visual deck storage
- The tags, which are used for filtering in the visual deck storage

# Adding Cards

Adding cards is done by either using the "Quick search and add card" search bar at the top of the "Visual Deck View" tab
or by clicking on a picture of a card in the "Visual Database Display" tab.

See @ref visual_database_display for more information on how to utilize the visual database display.

# Modifying the Deck List

To modify or remove cards in the deck list, the tree list view in the deck dock widget can be used.

\image html deck_dock_deck_list.png

Just above the list, at the top right, there are four buttons to manipulate the currently selected card(s):

- Increment Card
- Decrement Card
- Remove Card
- Switch Card between Mainboard and Sideboard

\image html deck_dock_deck_list_buttons.png

Additionally, there is a combo box above the list, which may be used to change how cards are grouped in the list
display. This is only for visual display and does not affect how the list is saved.

\image html deck_dock_deck_list_group_by.png

# Modifying the visual deck layout

The visual deck editor displays cards visually, as opposed to simply in list form in the Deck Dock Widget. Each entry in
the deck list is represented by a picture. These entries are grouped together under their respective sub-groups.
Sub-groups may be collapsed (i.e. the pictures contained within them are hidden) by clicking on the name of the group in
the banner.

Cards may be displayed in a "Flat" layout, which displays each picture next to each other and ensures full
visibility for each card.

\image html vde_flat_layout_type_grouped.png width=1200px

or in an "Overlap" layout, which overlaps cards on top of each other (leaving the top 20% of
the card uncovered so names remain readable) and arranges them in stacks to save space and allow for an easy overview.

\image html vde_overlap_layout_type_grouped.png width=1200px

Additionally, it is possible to change how the cards in the deck list are grouped by selecting a different grouping
method from the combo box, either in the top left of the "Visual Deck View" tab or above the list view in the deck dock
widget.

Example - Cards grouped by suit/color:

\image html vde_flat_layout_color_grouped.png width=1200px

Furthermore, it is possible to change how the cards are sorted within the sub-group. This is done by clicking on the
button with the cogwheel icon next to the combo box that adjusts grouping in the top left of the "Visual Deck View" tab.
This presents a list of available sort criteria, which may be rearranged to change their priorities.

# Modifying printings

For more information on modifying the printings in a deck see @ref editing_decks_printings

# Deck Analytics

The visual deck editor offers a "Deck Analytics" tab, which displays information about:

- The mana curve
- The mana devotion
- The mana base

\image html vde_deck_analytics.png width=1200px

# Sample Hand

The visual deck editor offers a "Sample Hand" tab, which allows simulating drawing a configurable amount of cards from
the deck, which reduces the need to launch a single player game for testing purposes.

\image html vde_sample_hand.png width=1200px