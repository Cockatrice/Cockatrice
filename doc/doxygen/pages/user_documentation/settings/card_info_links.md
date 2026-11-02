@page card_info_links Card Info Web Pages

Card Info's **Open Card Page** button opens the selected card in your default browser.
The button is available in Image, Description, and Both views, including Card Info panels in the deck editors and games.
The default Scryfall search uses the card's name and your selected card language.

Use the **...** button beside it and choose **Configure URL...** to set your preferred website.
This setting is shared by Card Info panels and saved for future sessions.
**Reset to Scryfall** restores the default search.
Configuration remains available when no card is selected or a URL cannot be generated.

## URL templates

Templates use the same syntax as
[custom picture download URLs](https://github.com/Cockatrice/Cockatrice/wiki/Custom-Picture-Download-URLs).
Substituted values are percent-encoded for URLs. Templates must use `https://` or `http://`.

| Placeholder | Value |
| --- | --- |
| `!name!` | Card name |
| `!name_lower!` | Lowercase card name |
| `!setcode_lower!` | Selected printing's lowercase set code |
| `!set:num!` | Selected printing's collector number |
| `!set:uuid!` | Selected printing's database UUID, when available |
| `!prop:...!` | Named card property |
| `!set:...!` | Named property of the selected printing |
| `!sflang!` | Selected card language code |

The existing substring and padding rules also work, for example `!set:uuid_substr_0_2!` and `!set:num_fill_with_000!`.
Use database property names supported by your card database; a website may require a different identifier.

The default name/language search is:

```text
https://scryfall.com/search?q=%21%22!name!%22+lang%3A!sflang!
```

For a specific Scryfall set and collector number, use:

```text
https://scryfall.com/card/!setcode_lower!/!set:num!
```

Set and printing placeholders use the printing selected in Card Info, even when image settings override its artwork with a personal preference.
If the selected card lacks a required field, **Open Card Page** is disabled and its tooltip explains how to configure another template.
A name-based URL is useful for custom cards or cards without printing metadata.
