@page custom_card_pictures Custom Card Pictures

There are three ways to make Cockatrice use custom artwork for your cards:

- Placing image files in the **CUSTOM pictures folder**.
- Providing a **custom card database** that points each printing at a picture URL via the `picurl` property.
- Writing your **own download URL templates**.

Each of these is described below. If pictures are missing or wrong, see @subpage fixing_card_pictures instead.

# Custom Pictures Folder (CUSTOM)

Any image file placed in the CUSTOM folder is used as the card picture, and no download is attempted for cards that
match a file there.

- The folder is `<pictures directory>/CUSTOM/`. The pictures directory is configured on the 'General' settings tab,
  under 'Directories' → 'Pictures directory'.
- Accepted formats are PNG, JPG and JPEG.
- Files are indexed by their name, so you can organize them into subfolders freely.
- New or changed files are picked up automatically within a few seconds — no client restart is required.

The file name must match the card using one of the naming schemes below. Both `_` and `-` are accepted as separators,
and the file extension is ignored when matching:

| Scheme                        | Example file name                                      |
|-------------------------------|--------------------------------------------------------|
| Card Name                     | `Example Card.png`                          |
| Card Name + Set               | `Example Card_DDL.png`                      |
| Card Name + Set + Collector   | `Example Card_DDL_43.png`                   |
| Set + Collector + Card Name   | `DDL_43_Example Card.png`                   |
| Card Name + Provider ID       | `Example Card_0b23cdc8-d413-4fb1-8470-474221b10fe2.png` |

The name used for matching is the *corrected* card name. In practice this means punctuation is stripped, so the
"Example // Card" card is matched by a file named `ExampleCard.png`, not `Example // Card.png`.

\attention A file in the CUSTOM folder always wins over downloaded pictures, even if it is the wrong image. Delete the
file if you want to see the downloaded artwork again.

The naming conventions are the same as those recognized in the set-named subfolders and in `downloadedPics`, and are
documented for developers in @ref loading_card_pictures.

# Custom Card Database (picurl)

If you maintain your own card database (see the
[Custom Cards & Sets](https://github.com/Cockatrice/Cockatrice/wiki/Custom-Cards-&-Sets) wiki), each printing's `<set>`
tag can carry a `picurl` attribute containing a full URL for that printing's picture:

```xml
<set picurl="https://example.com/cards/example-card.jpg" ...>
```

Cockatrice tries this URL **before** the configured download URL templates, so it is the most direct way to provide
custom artwork for a specific printing.

- The URL must start with `http://` or `https://`.
- When you change a `picurl` for a card whose picture was already downloaded and cached, delete the stored images
  (Storage tab → 'Delete Saved Images' / 'Delete Cached Images') so Cockatrice fetches the new URL.

# Custom Download URL Templates

The built-in download URLs are templates: Cockatrice replaces reference points in the URL with information about the
card and its printing. You can write your own templates in 'Cockatrice → Settings' (Ctrl + Shift + P by default), on
the 'Deck Editor' tab, in the 'URL Download Priority' section.

The following reference points are available:

| Reference point          | Description | Example |
|--------------------------|-------------|---------|
| `!name!` | Card name | `Example Card` |
| `!name_lower!` | Card name, lower case | `example card` |
| `!corrected_name!` | Corrected card name | `ExampleCard` (instead of "Example // Card") |
| `!corrected_name_lower!` | Corrected card name, lower case | `examplecard` |
| `!sflang!` | Scryfall language code for the current client language; defaults to English when the language has no localized images | `en`, `zhs` |
| `!setcode!` / `!setcode_lower!` | Set code | `EXM` / `exm` |
| `!setname!` / `!setname_lower!` | Full set name | `Exemplary Set` / `exemplary set` |
| `!set:<property>!` | A property of this printing, e.g. `muid` (Gatherer multiverse ID), `uuid` (Scryfall UUID), `num` (collector number), `rarity` | `373549` |
| `!prop:<property>!` | A property of the card, e.g. `side` (front/back), `colors`, `cmc`, `coloridentity`, `type`, `pt`, and the format legality statuses | `front` |

The `!set:...!` and `!prop:...!` reference points support two modifiers:

- `_fill_with_<text>` pads the value with the given text, right-aligned, e.g. `!set:num_fill_with_000!` turns collector
  number `1` into `001`. If the value is longer than the fill text, the template is skipped.
- `_substr_<start>_<length>` extracts a substring, e.g. `!set:num_substr_2_2!` takes two characters starting at the
  third. If the substring would extend past the end of the value, the template is skipped.

Substituted values are URL-encoded. A template that asks for a property the card or printing does not have is skipped,
and the next template in the list is tried instead.

\attention Custom URLs must start with `http://` or `https://` to be accepted.

Some working examples:

```text
https://cards.scryfall.io/large/!prop:side!/!set:uuid_substr_0_1!/!set:uuid_substr_1_1!/!set:uuid!.jpg
https://api.scryfall.com/cards/!set:uuid!?format=image&face=!prop:side!
https://api.scryfall.com/cards/multiverse/!set:muid!?format=image
https://gatherer.wizards.com/Handlers/Image.ashx?multiverseid=!set:muid!&type=card
https://gatherer.wizards.com/Handlers/Image.ashx?name=!name!&type=card
```

See the [Custom Picture Download URLs](https://github.com/Cockatrice/Cockatrice/wiki/Custom-Picture-Download-URLs)
wiki for more examples and ideas.

\attention Keep in mind that templates using `!name!` or `!set:muid!` resolve by name or multiverse ID, not by the
exact printing. Only the Scryfall `!set:uuid!` templates always return the exact printing requested. See
@subpage fixing_card_pictures for more on this.
