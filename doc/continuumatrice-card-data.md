# Continuumatrice: isolated profile and card database

Continuumatrice intentionally uses its own Qt application identity:
- Application: `Continuumatrice`
- Organization: `Continuum Universes Studio`
- Organization domain: `continuum-universes.com`

The identity is assigned in `cockatrice/src/main.cpp` **before** `SettingsCache` runs.
The single-instance local socket is also renamed so a running Cockatrice client does not intercept Continuumatrice launches.

## Default Linux paths

For the usual openSUSE / XDG environment (`XDG_DATA_HOME` and `XDG_CACHE_HOME` unset):

| Purpose | Default |
| --- | --- |
| Application data | `~/.local/share/Continuumatrice/` |
| Preferences | `~/.local/share/Continuumatrice/settings/` |
| Primary database | `~/.local/share/Continuumatrice/cards.xml` |
| Additional custom XML databases | `~/.local/share/Continuumatrice/customsets/` |
| Decks | `~/.local/share/Continuumatrice/decks/` |
| Downloaded/local images | `~/.local/share/Continuumatrice/pics/` |
| Local custom images | `~/.local/share/Continuumatrice/pics/CUSTOM/` |
| User themes | `~/.local/share/Continuumatrice/themes/` |
| Cache | `~/.cache/Continuumatrice/` |

Qt follows the XDG environment variables if set; the paths above are defaults, not mandatory constants. A `portable.dat` next to the binary activates Cockatrice's existing portable mode and stores data/cache next to the executable instead. No old Cockatrice data is automatically copied or deleted.

**Note:** The client's executable and file-association/URL scheme are still called Cockatrice. This patch isolates profiles, not every branding or installer integration. Avoid running the upstream Oracle Magic-card downloader against this profile until you intentionally configure it; that is a separate task.

## Card database format

Continuumatrice currently reads **Cockatrice XML v4** (schema `doc/carddatabase_v4/cards.xsd`). This is a database description and card list, not the rendered card images. You do not need to build new parsing logic to playtest your own cards.

A self-contained *test-only* main database `cards.xml` can look like:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<cockatrice_carddatabase version="4">
  <sets>
    <set>
      <name>CTEST</name>
      <longname>Continuumatrice Test Cards</longname>
    </set>
  </sets>
  <cards>
    <card>
      <name>Example Front Card</name>
      <text>Test card for checking battlefield placement.</text>
      <prop>
        <type>Unit</type>
      </prop>
      <set num="001" rarity="common">CTEST</set>
      <tablerow>0</tablerow>
    </card>
    <card>
      <name>Example Support Card</name>
      <prop>
        <type>Unit</type>
      </prop>
      <set num="002">CTEST</set>
      <tablerow>1</tablerow>
    </card>
    <card>
      <name>Example Command Card</name>
      <prop>
        <type>Unit</type>
      </prop>
      <set num="003">CTEST</set>
      <tablerow>2</tablerow>
    </card>
  </cards>
</cockatrice_carddatabase>
```

These names are examples, **not** Continuum Universes lore or playable canon.

For a real card catalogue, generate XML from the card design/export pipeline and store it under `cards.xml` (main database), or under `customsets/*.xml` (additional sets). **A readable main `cards.xml` is needed for the loader to proceed to the optional custom sets**, so do not use custom sets exclusively without one. The loader collects custom-set files recursively and loads them in alphabetical order.

The XML `<tablerow>` maps to:
- `0` = Front
- `1` = Support
- `2` = Command

By default, cards are manually positioned and moved: this client does not evaluate card rules.

## Images

Card images are separate from card metadata:
- Put a local image in `pics/CUSTOM/` with the **same corrected card name** as its XML name (such as `Example Front Card.png`). Custom pictures are indexed recursively. Or
- Provide a `picurl` URL attribute on the printing's `<set>` tag and let the client download it.

For reliable matching across reprints, the importer also recognizes names incorporating the set code, collector number and printing/provider ID. The existing loader supports various filename schemes and file extensions.

## Loading and testing

1. Update and rebuild the fork.
2. Launch Continuumatrice and verify its `Continuumatrice` folder is created.
3. Quit the client before writing a new `cards.xml` or image files.
4. Place your own XML and images in the new profile.
5. Relaunch or reload the card database in the client.
6. Ensure your set is enabled; test dragging cards and checking Front/Support/Command placement.
7. Check normal Cockatrice's profile is still intact and can open separately.

**Magic Set Editor** is an editor for card layouts and card content, but its native project format is not Cockatrice XML v4. You will need a matching export script/template or a conversion pipeline that writes this XML and the image files.
