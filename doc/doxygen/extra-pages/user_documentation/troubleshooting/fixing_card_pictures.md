@page fixing_card_pictures Fixing Card Pictures

This guide collects the common causes for card pictures not showing up, showing the wrong printing, or showing the
card back instead of the artwork, and how to fix them.

Work through the steps in order. In most cases the problem is caused by an outdated client, misconfigured download
URLs, stale local images, or a corrupted cache.

# Update Your Client

Picture handling bugs are fixed on a regular basis, so the first thing to try is updating your client.

Use the update check in 'Help → Check for Updates' (or look for the update prompt shown on startup).

If a fix has not yet made it into the latest stable release, it may already be available in the beta release. The beta
ships very frequently and usually receives a follow-up fix within a day or two if something breaks.

See @subpage beta_release for instructions on how to switch to the beta channel.

# Check Your Download URLs

Card pictures are downloaded from a list of URL templates. Each template is tried in order until one produces a valid
image, so the order matters: URLs at the top of the list are tried first.

The list can be found in 'Cockatrice → Settings' (or Ctrl + Shift + P by default), on the 'Deck Editor' tab, in the
'URL Download Priority' section. Make sure 'Download card pictures on the fly' is enabled and that the list contains
valid URLs. If you suspect the list has been modified or corrupted, press 'Reset Download URLs' to restore the
defaults.

For information on how to add your own custom URL templates, see the 'How to add a custom URL' link in the same
settings section, or @subpage custom_card_pictures for a full reference of the URL reference points, the CUSTOM
pictures folder, and custom card databases.

# Check Your Local Picture Folder

Before any network request is made, Cockatrice looks for local image files. If a matching file is found on disk it is
shown instead of anything downloaded, even if it is the wrong image.

The pictures directory is configured on the 'General' settings tab, under 'Directories' → 'Pictures directory'.
Cockatrice checks the following locations, in order:

- The custom pictures folder (recursively indexed by file name).
- `<pictures directory>/<set code>/<card file name>`
- `<pictures directory>/downloadedPics/<set code>/<card file name>`

The following import naming schemes are recognized (using both `_` and `-` as separators). The canonical table with
concrete example file names is on @ref custom_card_pictures:

| Scheme                      | Pattern                    |
| --------------------------- | -------------------------- |
| Card Name + Provider ID     | `{name}_{providerId}`      |
| Card Name + Set + Collector | `{name}_{set}_{collector}` |
| Set + Collector + Card Name | `{set}_{collector}_{name}` |
| Card Name + Set             | `{name}_{set}`             |
| Card Name                   | `{name}`                   |

If a picture you downloaded or placed manually is wrong, stale, or corrupted, delete the offending file. Pay special
attention to the `downloadedPics` subfolder: this is where the filesystem caching method writes downloaded images, and
after a provider outage it can permanently contain the wrong printing until you delete it manually.

See @ref loading_card_pictures for details on how local images are loaded.

# Clear Caches

Cockatrice caches card pictures in three places. All of them can be managed on the 'Storage' settings tab:

- **Network cache** — downloaded images stored on disk. Press 'Delete Cached Images' to clear it.
- **Filesystem / image backup** — downloaded images written directly to `downloadedPics`. Press 'Delete Saved Images'
  to clear it.
- **In-memory (pixmap) cache** — images currently held in RAM. Press 'Clear In-Memory Images' to clear it.

If a provider outage caused the wrong pictures to be downloaded and cached, clearing the network cache (and the
'Delete Saved Images' button if you use the filesystem caching method) will force Cockatrice to download the correct
images again. The redirect cache TTL (also on the Storage tab) controls how long previously seen redirects for
download URLs are remembered; lowering it can help if a URL used to redirect somewhere else.

# Restart the Client

After updating the client, changing the download URLs, moving or deleting local image files, or clearing caches, it is
recommended to restart Cockatrice so that all changes are fully picked up.

# Check Logs

Before changing any settings, check the existing logs first. Rate limit errors and most download errors are already
logged at warn level, so you may find the cause without enabling debug mode.

Open 'Help → View Debug Log' and look for error or warning messages related to card picture loading. If you need more
detail than the default log level provides, see below.

# Enable Picture Loader Debug Logs

If the steps above did not solve the problem, you can turn on a "diagnostics mode" that prints what the picture loader
is actually doing: which URL it is trying, whether it found or missed a file on disk, whether the download succeeded or
failed, and which redirects it followed. This information is extremely useful when asking for help.

See @subpage enabling_debug_logs for a step-by-step guide on how to enable the logs, including instructions
for Windows, macOS, and Linux.

# Provider Accuracy

Cards in Cockatrice are identified by a provider ID, which is the Scryfall UUID of a specific printing. Decks store
this ID, which is why the exact printing a card was added as can be looked up again.

The Scryfall URL templates built into Cockatrice use this provider ID directly (`!set:uuid!`), so they always download
the exact printing that was requested.

The Gatherer URL templates, on the other hand, do **not** use the provider ID. They resolve pictures by multiverse ID
(`!set:muid!`) or by card name (`!name!`) only. As a result they may return a different printing than the one the
provider ID refers to, or no picture at all for cards Gatherer does not know. If you need pictures to match the exact
printing of a card, make sure the Scryfall URLs are at the top of your download URL priority list and consider removing
or demoting the Gatherer URLs.
