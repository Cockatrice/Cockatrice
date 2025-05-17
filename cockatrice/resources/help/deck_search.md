## Deck Search Syntax Help
-----
The search bar recognizes a set of special commands.<br>
In this list of examples below, each entry has an explanation and can be clicked to test the query. Note that all
searches are case insensitive.
<dl>
<dt>Filename:</dt>
<dd>[red deck wins](#red deck wins) <small>(Any deck filename containing the words red, deck, and wins)</small></dd>
<dd>["red deck wins"](#%22red deck wins%22) <small>(Any deck filename containing the exact phrase "red deck wins")</small></dd>

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
