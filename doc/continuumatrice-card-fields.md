# Continuumatrice CCG card fields and numeric sorting

This document covers **static card-definition metadata** loaded from Continuumatrice XML v4. It does not change mutable game resources, damage, armour degradation, campaign state or server match mechanics.

## Source mapping

| Continuumatrice field | Native XML v4 property | Field in existing Continuum MSE game | Legacy import fallback | Sorting |
| --- | --- | --- | --- | --- |
| Identity | `identity` | `alias` | `alias` | Locale-aware text |
| Attack (ATK) | `attack` | `strength` | `strength`, or Attack side of `pt` | Numeric |
| Armour | `armour` | `armor` | `armor` | Numeric |
| Health (HP) | `health` | `health` | Health side of `pt` | Numeric |
| ATK/HP | Derived from Attack and Health | MSE `strength` + `health` | `pt` via the individual stat fallbacks | Attack numeric first, then Health numeric |

**Identity** is the CCG's **card alias**. It must not be confused with `essence` (which is the distinct Essence resource/type identity). The `Name` column still contains the actual card name.

The XML importer already supports arbitrary text properties under `<prop>`, so adding these properties does not require a schema version change.

## Example

```xml
<card>
  <name>Example Character</name>
  <text>Static card data demonstration only.</text>
  <prop>
    <type>Character</type>
    <identity>The Guardian</identity>
    <attack>10</attack>
    <armour>3</armour>
    <health>12</health>
    <essence>M</essence>
  </prop>
  <set>EXAMPLE</set>
  <tablerow>0</tablerow>
</card>
```

The table view displays ATK/HP as `10/12`, with separate **Attack**, **Armour**, and **Health** columns. If a card has no authored stats, the combined display remains blank. When one stat is omitted, the missing side is shown as `?`.

The database's sort dropdown discovers model columns dynamically, so the new fields are available for ascending/descending sorting. Numeric fields use numeric comparisons; the combined ATK/HP column sorts by Attack first and Health second. The former `P/T` column position is retained internally for compatibility with existing saved table views.

## Exporter compatibility

The matching Magic Set Editor exporter change is in `Continuum-Universes-MSE-Templates`:

- `card.alias` → `<identity>`
- `card.strength` → `<attack>`
- `card.armor` → `<armour>`
- `card.health` → `<health>`

The MSE internal field identifiers are **not renamed**: that would otherwise risk breaking existing `.mse-set` files, render scripts and card styles.

Older XML exports with `strength`, `armor`, `alias`, or `pt` remain readable. Native properties take precedence whenever present; no automatic rewriting of existing files occurs.

## Test checklist

```bash
cmake --build build --target continuumatrice carddatabase_test model_contract_test --parallel 16
ctest --test-dir build -R 'CardInfoTest.ContinuumIdentityAndStatsHaveNativeAndLegacyPropertyAccess|ModelContract.ContinuumCardStatsAreSortableAndLabeled' --output-on-failure
```

- [ ] Import an MSE-exported set and verify Identity, Attack, Armour, Health and ATK/HP columns.
- [ ] Check Attack `9` precedes `10` and Armour `2` precedes `10`.
- [ ] Check ATK/HP sorts on Attack then Health.
- [ ] Verify legacy fields are still readable and an empty field is not assumed to mean zero.
- [ ] Verify the card details pane displays the new names, including **Armour**.
- [ ] Verify the existing Essence filter still operates independently of the Alias/Identity property.

**Status:** Source changes committed; native compilation, tests and MSE GUI export still need local runtime verification.
