@page deck_search_syntax_help Deck Search Syntax Help

## Deck Search Syntax Help
-----
The search bar recognizes a set of special commands.<br>
In this list of examples below, each entry has an explanation and can be clicked to test the query. Note that all
searches are case insensitive.
<dl>
<dt>Display Name (The deck name, or the filename if the deck name isn't set):</dt>
<dd>[red deck wins](#red deck wins) <small>(Any deck with a display name containing the words red, deck, and wins)</small></dd>
<dd>["red deck wins"](#%22red deck wins%22) <small>(Any deck with a display name containing the exact phrase "red deck wins")</small></dd>

<dt>Deck <u>N</u>ame:</dt>
<dd>[n:aggro](#n:aggro) <small>(Any deck with a name containing the word aggro)</small></dd>
<dd>[n:red n:deck n:wins](#n:red n:deck n:wins) <small>(Any deck with a name containing the words red, deck, and wins)</small></dd>
<dd>[n:"red deck wins"](#n:%22red deck wins%22) <small>(Any deck with a name containing the exact phrase "red deck wins")</small></dd>

<dt><u>F</u>ile Name:</dt>
<dd>[f:aggro](#f:aggro) <small>(Any deck with a filename containing the word aggro)</small></dd>
<dd>[f:red f:deck f:wins](#f:red f:deck f:wins) <small>(Any deck with a filename containing the words red, deck, and wins)</small></dd>
<dd>[f:"red deck wins"](#f:%22red deck wins%22) <small>(Any deck with a filename containing the exact phrase "red deck wins")</small></dd>

<dt>Relative <u>P</u>ath (starting from the deck folder):</dt>
<dd>[p:aggro](#p:aggro) <small>(Any deck that has "aggro" somewhere in its relative path)</small></dd>
<dd>[p:edh/](#p:edh/) <small>(Any deck with "edh/" in its relative path, A.K.A. decks in the "edh" folder)</small></dd>

<dt>Deck Contents (Uses [card search expressions](#cardSearchSyntaxHelp)):</dt>
<dd><a href="#[[plains]]">[[plains]]</a> <small>(Any deck that contains at least one card with "plains" in its name)</small></dd>
<dd><a href="#[[t:legendary]]">[[t:legendary]]</a> <small>(Any deck that contains at least one legendary)</small></dd>
<dd><a href="#[[t:legendary]]>5">[[t:legendary]]>5</a> <small>(Any card that contains at least 5 legendaries)</small></dd>
<dd><a href="#[[]]:100">[[]]:100</a> <small>(Any deck that contains exactly 100 cards)</small></dd>

<dt>Negate:</dt>
<dd>[soldier -aggro](#soldier -aggro) <small>(Any deck filename that contains "soldier", but not "aggro")</small></dd>

<dt>Branching:</dt>
<dd>[t:aggro OR o:control](#t:aggro OR o:control) <small>(Any deck filename that contains either aggro or control)</small></dd>

<dt>Grouping:</dt>
<dd><a href="#red -([[]]:100 or aggro)">red -([[]]:100 or aggro)</a> <small>(Any deck that has red in its filename but is not 100 cards or has aggro in its filename)</small></dd>

</dl>
