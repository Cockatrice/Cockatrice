# Continuumatrice Essence symbols (CCG card catalogue)

Continuumatrice uses **Essence** for card resources, not Magic: The Gathering mana. The canonical Essence symbol order is:

```text
M S O G E A Q C B R L V T P N
```

These are **15 distinct symbols**. `N` is explicitly **colorless Essence**. The other 14 codes currently have **no names, canonical colors or game rules assigned in the client**. Do not infer them from the corresponding MTG letters. All Essence codes are uppercase.

## Native Continuumatrice XML v4

The parser supports arbitrary XML `<prop>` entries, so no change to the existing XML v4 schema format is needed. These three native property names are defined in `libcockatrice/card/game_specific_terms.h`:

| Property | Meaning |
| --- | --- |
| `essencecost` | Card's explicitly authored Essence cost, e.g. `{M}{S}{N}` |
| `essence` | Card's explicitly authored Essence identity/types, e.g. `MSN` |
| `essencevalue` | Optional numeric aggregate value for existing cost-curve UI; this is not calculated from cost |

**Test-only sample**, not a playable/canonical card:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<continuumatrice_carddatabase version="4">
  <sets>
    <set><name>CTEST</name><longname>Essence Import Test</longname></set>
  </sets>
  <cards>
    <card>
      <name>Essence Sample Card</name>
      <text>Test Essence symbol rendering only.</text>
      <prop>
        <type>Test Card</type>
        <essencecost>{M}{S}{N}</essencecost>
        <essence>MSN</essence>
      </prop>
      <set num="001">CTEST</set>
      <tablerow>1</tablerow>
    </card>
  </cards>
</continuumatrice_carddatabase>
```

Do not automatically derive `essence` or `essencevalue` from `essencecost` until gameplay rules define the correct relationship. The card database **stores metadata**, not mutable match resource balances.

Previously authored XML `<manacost>`, `<colors>`, and `<cmc>` properties remain readable by the inherited `CardInfo` API for compatibility, but new Continuumatrice cards should use the native properties. Format compatibility does **not** mean MTG `W` or `U` are accepted as Essence symbols.

## Icons and custom asset assignment

All 15 Essence letters have small bundled SVG **placeholder** icons under:

```text
cockatrice/resources/icons/essence/
  M.svg S.svg O.svg G.svg E.svg A.svg Q.svg C.svg B.svg
  R.svg L.svg V.svg T.svg P.svg N.svg
```

Each is rendered using an independent `icons/essence/` asset namespace. In the currently selected Continuumatrice theme you may override any SVG with an SVG, PNG, JPG or JPEG using the same stem:

```text
~/.local/share/Continuumatrice/themes/<YourTheme>/icons/essence/
    M.svg
    S.png
    N.svg
```

The current bundled placeholders show just the letter. `N` is neutral gray; the others use a shared neutral treatment. They are **not** final Essence artwork or a commitment to elemental color assignments. The chart interface uses a separately generated *chart-only* palette so fifteen statistical series can be distinguished.

## Features updated

- Card database properties: native Essence cost, identity and optional value, with fallbacks for older XML.
- Card information display and deck editor column headers: `Essence`, `Essence Cost`, `Essence Value`.
- Visual Deck Storage and Visual Database filters share a compact `Essence: Any` dropdown with a 5×3 grid of all 15 icons. Select multiple icons without closing the popup, clear selections within the popup, and change filtering mode **inside that same popup**. Only one compact control occupies the toolbar. The dropdown summarizes selected Essence codes and supports N independently from C.
- Cost parser: single Essence symbols in bare form (`MSN`) or braces (`{M}{S}{N}`); numeric tokens may be preserved as generic amounts.
- Deck identity: unique, ordered Essence codes (including `N`).
- Search filters: filter for specific Essence codes, or `colorless` meaning `N`. An empty identity is not automatically `N`.
- Analytics: counts all 15 Essence cost symbols. Production counts only literal `Add {M}`-style grants to avoid guessing any resource rules.

Hybrid, phyrexian, and other MTG-specific mana-cost syntax are **not** introduced as Essence mechanics. Nor is automatic calculation of resource costs, card affordability, or game turn rules included.

## Compact Essence filter UI

The former five-button MTG `WUBRG` filters are replaced in both Visual Deck Storage and Visual Database. On either screen, click `Essence: Any` to open a **5-column × 3-row** grid of the complete Essence set. Each icon can be toggled independently and the menu stays open for multiple selections. Use **Clear Essence selection** to reset. The button summarizes your selection, and the filtering mode remains available **inside** the popup (Exact Match / Includes / Excludes in deck storage, or Include / Exclude in the database).

The picker is a single reusable Qt widget (`essence_filter_picker.h/.cpp`). This keeps both screens synchronized with `Essence::symbols()` without repeating a hand-maintained UI list. The UI uses SVG assets from `icons/essence/`, never old MTG `icons/mana/` assets.


## Build and verification

From `feature/continuumatrice-essence`:

```bash
cmake -S . -B build --fresh -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DWITH_CLIENT=ON -DWITH_SERVER=ON -DWITH_ORACLE=OFF -DTEST=ON
cmake --build build --target continuumatrice continuum-servatrice essence_types_test --parallel 16
ctest --test-dir build -R EssenceTypesTest --output-on-failure
./build/continuumatrice/continuumatrice
```

Verify that all fifteen icons render, `N` loads and filters separately from `C`, the XML importer accepts `essencecost` and `essence`, and the analytics show no WUBRG assumptions.

**Status:** Source-level implementation committed. User-side Qt compilation and visual/runtime testing remain required.

## November UI stability fixes

The database toolbar now keeps its Essence filtering mode **inside** the picker popup, rather than forcing two separate controls into a wrapping flow layout. Both Visual Deck Storage and Visual Database use one compact toolbar button with a selection summary and tooltip. The popup grid is unchanged (15 icons, five columns × three rows).

When the final selected Essence is cleared, the filter becomes **Any**, including in Exact Match / Include-Exclude mode; it must not leave behind exclusions for all fifteen symbols.

A new Continuumatrice profile starts with a valid, deliberately empty XML card catalogue. The in-memory database now treats a successful empty snapshot as `LoadStatus::Ok`, not `NotLoaded`. This allows the card browser and toolbar to initialize even before the user's first MSE card import. `carddatabase_test` covers this distinction.
