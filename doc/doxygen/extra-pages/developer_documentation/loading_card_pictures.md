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

The size of both of these caches can be configured by the user on the "Storage" settings page.

# PixmapCacheKeys and ProviderIDs

Every card picture that is loaded ends up in the QPixmapCache under a key that identifies the exact printing it belongs
to. The key is produced by ExactCard::getPixmapCacheKey() and has the following shape:

```text
card_<card name>_<provider ID>
```

For example, the _Example Card_ printing with provider ID `0b23cdc8-d413-4fb1-8470-474221b10fe2` is stored
under `card_Example Card_0b23cdc8-d413-4fb1-8470-474221b10fe2`. If the printing has no provider ID, the key
drops the suffix and falls back to `card_<card name>`.

The **provider ID** is the Scryfall UUID of the printing. Oracle maps the `scryfallId` of every printing to the `uuid`
property when building the card database, and deck files persist it as the `uuid` attribute of each card entry. Because
the provider ID is part of the pixmap cache key, two different printings of the same card never share a cache entry.
This is exactly what allows the printing selector and exact-card lookups to display the picture of the precise printing
a card was added as.

The base key holds the full-size image. When a widget asks for a scaled version, the scaled pixmap is stored under an
additional key of the form `<key>_<width>x<height>`, with the size adjusted for the device pixel ratio of the screen,
so each widget size is only ever scaled once.

The cache key is also used for bookkeeping outside of the pixmap cache itself:

- CardPictureLoaderWorker keeps a set of keys that are currently being loaded so the same card is never queued twice.
- CardPictureLoader tracks, per key, the last time loading failed. A failed load stores a NULL pixmap under the key; as
  long as that marker is present, subsequent requests for the card show the "failed" card back and are only re-queued
  after the retry interval of 300 seconds has passed.
- When the CardInfo of a loaded card is destroyed, its cache entries and failure markers are removed.

# The Redirect Cache

Many picture URLs - in particular the Gatherer and Scryfall URLs from the default set of templates - redirect to a CDN
or to a different host. To avoid following the same redirect for every single card, CardPictureLoaderWorker remembers
redirects and applies them without an extra network round trip.

The redirect cache is a hash map from original URL to redirect URL plus timestamp. It is persisted to a `cache.ini`
file (Qt's INI format, under the `redirects` array) inside the redirect cache directory
(`SettingsCache::instance().paths().getRedirectCachePath()`, i.e. `<cache directory>/redirects/`). The cache is loaded when the worker
starts, pruned of entries older than the configured TTL, and written back to disk when the application quits.

Entries are added whenever a network reply reports a redirection (see below) and are consulted before any request is
made: both CardPictureLoaderWorker::queueRequest() and CardPictureLoaderWorker::makeRequest() check for a cached
redirect first and jump straight to the final URL.

The TTL is the "Redirect Cache TTL" setting on the "Storage" settings page and defaults to 30 days. Lowering it makes
Cockatrice re-resolve redirects sooner, which can help when a download URL changed its redirect target.

Because Cockatrice tracks redirects itself, the QNetworkAccessManager is configured with Qt's `ManualRedirectPolicy`.
Redirects found in a reply are handled manually:

- A recursive redirect (a URL redirecting to itself) is treated as a failed load.
- Otherwise the redirect is recorded in the redirect cache and the request is re-issued against the target URL.
- A successful reply with one of the redirect status codes 301, 302, 303, 305, 307 or 308 is handled the same way.

Clearing the network cache (CardPictureLoader::clearNetworkCache()) also clears the redirect cache.

# Local Image Loading

Before any network request is issued, CardPictureLoaderWorker hands the ExactCard to CardPictureLoaderLocal, which
tries to find a matching picture on disk. If a local picture is found, it is used and no network request is made.

CardPictureLoaderLocal searches two locations:

- The **CUSTOM folder** (`<pictures directory>/CUSTOM/`). Every file in it is indexed recursively by its base name
  (both `baseName` and `completeBaseName`, so a file named `ExampleCard.jpg` is indexed as `ExampleCard`). The index is rebuilt
  every 10 seconds, so new files are picked up without restarting the
  client (changing the configured pictures directory only reassigns the search paths; the next timer tick rebuilds the index).
- The **set-named subfolders** of the pictures directory: `<pictures directory>/<set code>/` and
  `<pictures directory>/downloadedPics/<set code>/`.

For each candidate folder, the loader generates file-name variants from the card's corrected name, set code, collector
number and provider ID using the import naming schemes (Card Name + Provider ID, Card Name + Set + Collector,
Set + Collector + Card Name, Card Name + Set, Card Name), each tried with both `_` and `-` as separator. A file matches
if its name starts with one of the variants - the extension is free - and the first variant that yields a readable
image wins. For example, the file `Example Card_EXM_43.png` in the `EXM` set folder matches the card with
corrected name `Example Card`, set code `EXM` and collector number `43`.

\attention The file-name variants use the *corrected* card name, so split cards are stored under their joined name: the
"Example // Card" card is matched by a file named `ExampleCard.*`.

The naming schemes are duplicated in the user-facing page @ref custom_card_pictures, which also documents how to
set up a custom card database that provides pictures via the CUSTOM folder and the `picurl` printing property.

When the filesystem cache method is selected on the "Storage" settings page, downloaded images are additionally written
into `<pictures directory>/downloadedPics/` using the configured export naming scheme (as `.png` files). Existing files
are never overwritten, so a provider outage can permanently leave a wrong image in that folder until it is deleted
manually - the user-facing troubleshooting guide @ref fixing_card_pictures covers how to do this.

# URL Generation and Resolution

When no local image is available and downloading is enabled, the network loader starts working through a list of
candidate URLs. This list is managed by CardPictureToLoad and is built in two steps.

First, CardPictureToLoad::extractSetsSorted() collects all sets the card has printings in and sorts them by set
priority. Unless the user disabled per-printing art ("Override all card art with personal set preference (Pre-ProviderID
change behavior)"), the set that
matches the requested printing's provider ID is moved to the front, so the exact printing is always attempted first.

For each set, CardPictureToLoad::populateSetUrls() builds an ordered URL list:

1. A custom URL defined for that printing via the `picurl` property in the card database, if present.
2. The configured download URL templates, in priority order (Deck Editor → "URL Download Priority").

URL templates are transformed into concrete URLs by CardPictureToLoad::transformUrl(), which substitutes reference
points. The following placeholders are available:

| Placeholder | Description | Example |
|-------------|-------------|---------|
| `!name!` | Card name | `Example Card` |
| `!name_lower!` | Card name, lower case | `example card` |
| `!corrected_name!` | Corrected card name | `ExampleCard` (instead of "Example // Card") |
| `!corrected_name_lower!` | Corrected card name, lower case | `examplecard` |
| `!sflang!` | Scryfall language code for the current client language; defaults to English when the language has no localized images | `en`, `zhs` |
| `!setcode!` / `!setcode_lower!` | Set code | `EXM` / `exm` |
| `!setname!` / `!setname_lower!` | Full set name | `Exemplary Set` / `exemplary set` |
| `!set:<property>!` | A property of this printing, e.g. `muid` (Gatherer multiverse ID), `uuid` (Scryfall UUID), `num` (collector number), `rarity` | `373549` |
| `!prop:<property>!` | A property of the card, e.g. `side` (front/back), `colors`, `cmc`, `coloridentity`, `type`, `pt`, and the format legality statuses | `front` |

The `!set:...!` and `!prop:...!` placeholders also support two modifiers:

- `_fill_with_<text>` pads the value with the given text, right-aligned, e.g. `!set:num_fill_with_000!` turns collector
  number `1` into `001`. If the value is longer than the fill text, the template is invalidated.
- `_substr_<start>_<length>` extracts a substring, e.g. `!set:num_substr_2_2!` takes two characters starting at the
  third. If the substring would extend past the end of the value, the template is invalidated.

Substituted values are percent-encoded. If a template asks for a property the card or printing does not have (or one of
the modifiers invalidates it), the template yields no URL and is skipped; the next template is tried instead.

\attention Custom URLs should start with `http://` or `https://`. The scheme is not validated before the URL is handed
to QNetworkAccessManager, so a template without an absolute scheme may silently fail to download; prefer HTTPS where the
provider allows it.

The resolution order is: for the current set, try each URL in the list; when all URLs for a set are exhausted, move to
the next set; when every set is exhausted, the load fails. A failed load is reported through the NULL-pixmap mechanism
described in the PixmapCacheKeys and ProviderIDs section above.

Several mechanisms influence the resolution process:

- **Rate limiting.** The worker allows roughly 10 requests per second globally. A server that answers with HTTP 429
  gets its per-host allowance halved; the first 429 for a host is waited out (honoring the `Retry-After` header if
  present) and the same URL retried, while a second 429 makes the loader fall through to the other configured sources.
  When all sources are exhausted the request is deferred with some random jitter and retried once the back-off expires.
- **Redirects.** Replies with a redirect status (301, 302, 303, 305, 307, 308) are followed and recorded in the
  redirect cache as described in the Redirect Cache section above.
- **Blacklisted images.** Gatherer returns the card back image for cards it does not know. A few known MD5 hashes of
  that image are blacklisted, so such a "successful" download is treated as not found instead of being shown.
- **WebP.** Images detected as WebP (RIFF/WEBP header) are decoded through QMovie instead of QImageReader.
- **Downloads disabled.** When "Download card pictures on the fly" is disabled and the network cache method is active,
  requests use Qt's `AlwaysCache` policy so that only previously cached images are served.

A user-facing reference for writing download URL templates, including more worked examples, is available at
@ref custom_card_pictures.
