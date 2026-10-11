@page editing_decks_printings Printing Selector

\image html printing_selector.png

# Purpose

The PrintingSelector allows editing the PrintingInfo associated with CardInfo%s contained in DeckList%s.

As described in PrintingInfo, a CardInfo can have different variations contained within the same or different CardSet%s.

PrintingSelector allows the user to choose a variation, as well as pinning a variation to be used as the preferred
printing in all cases where the 'default' or 'most preferred' image should be shown.

# Pre-requisites (User Interface)

To use the printing selector, ensure that you are using the Post-ProviderID change behavior (Checkbox as shown in the
image).

\image html printing_selector_pre_providerid.png

\attention This is the default behavior, unless you have explicitly changed it, in which case you will have seen a
warning as follows. Once again, checking this box will *disable* the new behavior and revert you to the legacy system
without the printing selector.

Disabling the printing selector:

\image html printing_selector_enable.png

Enabling the printing selector:

\image html printing_selector_disable.png

# Pre-requisites (Card Database)

The PrintingSelector requires each underlying CardInfo object to have a <set> entry with a *unique* providerID attribute
for each variation that should be displayed.

Here is an example, that first defines three sets, and then the 'Ace of Hearts' card with three variations.

```xml
<set rarity="common" uuid="00000000-0000-0000-0000-000000000033" num="51" muid="AH">PKR</set>
<set rarity="common" uuid="00000000-0000-0000-0001-000000000033" num="51" muid="AHSTD">STD</set>
<set rarity="common" uuid="00000000-0000-0000-0002-000000000033" num="51" muid="AHNBR">NBR</set>
```

```xml
<?xml version='1.0' encoding='UTF-8'?>
<cockatrice_carddatabase version="4">
    <sets>
        <set>
            <name>PKR</name>
            <longname>Cockatrice Poker</longname>
            <settype>Custom</settype>
            <releasedate>2025-11-15</releasedate>
        </set>
        <set>
            <name>STD</name>
            <longname>Standard Poker</longname>
            <settype>core</settype>
            <releasedate>2025-11-14</releasedate>
        </set>
        <set>
            <name>NBR</name>
            <longname>Numbers</longname>
            <settype>core</settype>
            <releasedate>2025-11-14</releasedate>
        </set>
    </sets>
    <cards>
        <card>
            <name>Ace of Hearts</name>
            <text>Ace of Hearts</text>
            <prop>
                <layout>normal</layout>
                <side>front</side>
                <type>Ace</type>
                <maintype>Ace</maintype>
                <manacost>14</manacost>
                <cmc>14</cmc>
                <colors>H</colors>
                <coloridentity>H</coloridentity>
                <format-standard>legal</format-standard>
            </prop>
            <reverse-related>Joker</reverse-related>
            <token>0</token>
            <tablerow>1</tablerow>
            <cipt>0</cipt>
            <upsidedown>0</upsidedown>
            <set rarity="common" uuid="00000000-0000-0000-0000-000000000033" num="51" muid="AH">PKR</set>
            <set rarity="common" uuid="00000000-0000-0000-0001-000000000033" num="51" muid="AHSTD">STD</set>
            <set rarity="common" uuid="00000000-0000-0000-0002-000000000033" num="51" muid="AHNBR">NBR</set>
        </card>
    </cards>
</cockatrice_carddatabase>
```

# Using the Printing Selector

To use the PrintingSelector, first select a card in the DeckEditorDeckDockWidget:

\image html deck_dock_deck_list.png

You may add a variation to the mainboard by clicking the + and - buttons in the top row. The bottom row will add the
variation to your sideboard.

\image html printing_selector.png

You can also right-click a variation to pin it, which will move it to the top of the variation list for easy access and
additionally, use it as the default printing whenever the printing isn't explicitly specified (such as adding a card
from the database in the deck editor).

Only one printing may be pinned at a time.

Additionally, the PrintingSelector offers a navigation bar at the bottom to navigate through cards in your deck, as well
as an option to modify the variations of cards in bulk via the 'Bulk Selection' option.

\image html printing_selector_navigation.png

The search bar at the top may be used to filter the displayed variations and the cogwheel may be clicked to open up
additional settings.

\image html printing_selector_options.png
