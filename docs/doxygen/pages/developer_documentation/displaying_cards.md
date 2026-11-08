@page displaying_cards Displaying Cards

Cockatrice offers a number of widgets for displaying cards. To pick the right one for your purpose, it is first
important
to determine the context in which you will be displaying the card.

# In-client

In the client (like in your custom widgets), you can use the existing card display widgets.

## Simple Display

The most general purpose of these is @ref CardInfoPictureWidget, which simply displays a picture of a card.

The textual equivalent to this is @ref CardInfoTextWidget, which displays all the properties of a card in text form.

## Detailed Display

The convenience class @ref CardInfoDisplayWidget displays both of these widgets in a tabbed container, which allows
choosing between either visual, textual, or both representations at the same time. It is the class that is most
familiar to our users when a single card is highlighted or inspected, since it is used in the deck editor and in-game
for these purposes.

If you would like the user to be able to inspect a single card in your custom widget, you should
expose a @ref CardInfoDisplayWidget to them and incorporate it as a dock widget inside your custom widget.

@ref CardInfoDisplayWidget is great for ensuring that the user has all the relevant details of a card available.
It is crucial anywhere the user might encounter a card for the first time or needs to make an informed decision.

## Visual Display

However, there is a reason why it is possible to have a visual-only representation of the card in the
@ref CardInfoDisplayWidget.

Cards are, by design, meant to be represented as images and cards which we can reasonably
expect the user to know (for example, cards contained in a user made deck or explicitly chosen by the user) can be
represented with just their image as a sort of "visual shorthand".

The simplest way to do this is of course, the above-mentioned @ref CardInfoPictureWidget which simply displays the
picture without modifications.

However, there exist a couple of other convenience classes which subclass @ref CardInfoPictureWidget to accomplish some
things which you might find useful as well.

- @ref CardInfoPictureWithTextOverlayWidget displays text overlaid on top of the card picture and scales this text to
  fit automatically.
- @ref DeckPreviewCardPictureWidget is a @ref CardInfoPictureWithTextOverlayWidget which also features a
  raise-on-mouse-enter animation, controlled by the global setting (TODO: specify which).
- @ref CardInfoPictureArtCropWidget attempts to display an 'art crop' of the card by cropping the upper region of the
  card and only displaying that.

## Groups of Cards

Sometimes it is useful to display images of cards in groups, for example to represent the deck in the @ref
VisualDeckEditorWidget or during confirmation in the pre-game lobby.

To this end, Cockatrice offers three convenience classes to display cards for an index in a @ref DeckListModel in a
group and one to display all cards in a particular zone of a @ref DeckListModel.

The generic way to do this is @ref CardGroupDisplayWidget, which displays cards in @ref QVBoxLayout. This class is very
generic and it is probably a better choice to either subclass it or use one of the two existing implementations.

@ref FlatCardGroupDisplayWidget displays cards using a horizontal @ref FlowWidget. This means it will fill available
screen space, left to right with no margins, with card pictures, skipping to a new line if it overflows available screen
space horizontally.

@ref OverlappedCardGroupDisplayWidget displays cards using an @ref OverlapWidget, which displays widgets on top of each
other in vertical columns until it exceeds available screen space in which case it will start a new column. The amount
of overlap as well as the overlap direction is configurable.

@ref DeckCardZoneDisplayWidget can be used to visualize all cards in a zone with either a @ref
FlatCardGroupDisplayWidget or a @ref OverlappedCardGroupDisplayWidget and headed by a @ref BannerWidget. It also allows
grouping and sorting.

# In-game

The in-game view is a @ref QGraphicsScene, which means any widget we want to display inside of it should be a @ref
QGraphicsObject.

- @ref CardItem



