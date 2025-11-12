@page loading_card_pictures Loading Card Pictures

Pictures associated with CardInfo%s are retrieved either from on-disk or the network through the CardPictureLoader.

In most cases, you don't need to concern yourself with the internals of CardPictureLoader.

Simply using one of the ways described in @ref displaying_cards is enough to automatically queue a request to the
CardPictureLoader when the chosen widget is shown, emitting signals to refresh the widget when the request is finished.

# How requests are triggered

CardPictureLoader::getPixmap() is called exactly two times in the code base, in CardInfoPictureWidget::loadPixmap(), the
base class
for all widget based card picture display, and AbstractCardItem::paintPicture(), the base class for all QGraphicsItem
based card picture
display. See @ref displaying_cards for more information on the difference between these two display methods.

Because both of these calls are made in the paintEvent() methods of their respective classes, this means that requests
are issued as soon as but not before the widget is shown on screen.

It is also possible to "warm up" the cache by issuing card picture load requests to the CardPictureLoader without using
a display widget and waiting for it to be shown by calling CardPictureLoader::cacheCardPixmaps() with a list of
ExactCard%s.

# The QPixmapCache and QNetworkDiskCache

Cockatrice uses the QPixmapCache from the Qt GUI module to store card pictures in-memory and the QNetworkDiskCache from
the Qt Network module to cache network requests for card pictures on-disk.

What this means is that the CardPictureLoader will first attempt to look up a card in the QPixmapCache according to the
ExactCard::getPixmapCacheKey() method of an ExactCard object. If it does not find it in the in-memory cache, it will
issue a load request, which will first look for local images on-disk and then consult the QNetworkDiskCache and if
found, use the stored binary data from the network cache to populate the in-memory pixmap cache under the card's cache
key. If it is not found, it will then proceed with issuing a network request.

The size of both of these caches can be configured by the user in the "Card Sources" settings page.

# PixmapCacheKeys and ProviderIDs

TODO

# The Redirect Cache

TODO

# Local Image Loading

TODO

# URL Generation and Resolution

TODO