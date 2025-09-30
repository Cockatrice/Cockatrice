@page search_syntax_help Search Syntax Help

## Search Syntax Help
-----
The search bar recognizes a set of special commands similar to some other card databases.<br>
In this list of examples below, each entry has an explanation and can be clicked to test the query. Note that all searches are case insensitive.
<dl>
<dt>Name:</dt>
<dd>[birds of paradise](#birds of paradise) <small>(Any card name containing the words birds, of, and paradise)</small></dd>
<dd>["birds of paradise"](#%22birds of paradise%22) <small>(Any card name containing the exact phrase "birds of paradise")</small></dd>

<dt>Rules Text (<u>O</u>racle):</dt>
<dd>[o:flying](#o:flying) <small>(Any card text that has the word flying)</small></dd>
<dd>[o:"first strike"](#o:%22first strike%22) <small>(Any card text that has the exact phrase "first strike")</small></dd>
<dd>[o:"{T}" o:"add one mana of any color"](#o:%22{T}%22 o:%22add one mana of any color%22) <small>(Any card text that has a tap symbol and the phrase "add one mana of any color")</small></dd>

<dt><u>T</u>ypes:</dt>
<dd>[t:angel](#t:angel) <small>(Any card with the type angel)</small></dd>
<dd>[t:angel t:legendary](#t:angel t:legendary) <small>(Any angel that's also legendary)</small></dd>
<dd>[t:basic](#t:basic) <small>(Any card with the type basic)</small></dd>
<dd>[t:arcane t:instant](#t:arcane t:instant) <small>(Any card with the types arcane and instant)</small></dd>

<dt><u>C</u>olors:</dt>
<dd>[c:w](#c:w) <small>(Any card that is white)</small></dd>
<dd>[c:wu](#c:wu) <small>(Any card that is white or blue)</small></dd>
<dd>[c:wum](#c:wum) <small>(Any card that is white or blue, and multicolored)</small></dd>
<!--
<dd>[c!w](#c!w) <small>(Cards that are only white)</small></dd>
<dd>[c!wu](#c!wu) <small>(Cards that are only white or blue, or both)</small></dd>
<dd>[c!wum](#c!wum) <small>(Cards that are only white and blue, and multicolored)</small></dd>
<dd>[c=wubrg](#c%3Dwubrg) <small>(Cards that are all five colors)</small></dd>
-->
<dd>[c:c](#c:c) <small>(Any colorless card)</small></dd>
<dd>[ci:w](#ci:w) <small>(Any card that has white in it's color identity)</small></dd>

<dt><u>Pow</u>er, <u>Tou</u>ghness, <u>M</u>ana <u>V</u>alue:</dt>
<dd>[tou:1](#tou:1) <small>(Any card with a toughness of 1)</small></dd>
<dd>[pow>=8](#pow>=8) <small>(Any card with a power greater than or equal to 8)</small></dd>
<dd>[mv=7](#mv=7) <small>(Any card with a mana value equal to 7)</small></dd>

<dt><u>R</u>arity:</dt>
<dd>[r:mythic](#r:mythic) <small>(Any card that has the mythic-rare rarity)</small></dd>

<dt><u>F</u>ormat:</dt>
<dd>[f:standard](#f:standard) <small>(Any card that can be played in standard)</small></dd>
<dd>[banned:modern](#banned:modern) <small>(Any card that is banned in modern)</small></dd>
<dd>[restricted:vintage](#restricted:vintage) <small>(Any card that is restricted in vintage)</small></dd>
<dd>[legal:pauper](#legal:pauper) <small>(Any card that is legal in pauper)</small></dd>

<dt><u>E</u>dition:</dt>
<dd>[set:lea](#set:lea) <small>(Cards that appear in Alpha, which has the set code LEA)</small></dd>
<dd>[e:lea or e:leb](#e:lea or e:leb) <small>(Cards that appear in Alpha or Beta)</small></dd>

<dt>Negate:</dt>
<dd>[c:wu -c:m](#c:wu -c:m) <small>(Any card that is white or blue, but not multicolored)</small></dd>

<dt>Branching:</dt>
<dd>[t:sliver or o:changeling](#t:sliver or o:changeling) <small>(Any card that is either a sliver or has changeling)</small></dd>

<dt>Grouping:</dt>
<dd><a href="#t:angel -(angel or c:w)">t:angel -(angel or c:w)</a> <small>(Any angel that doesn't have angel in its name and isn't white)</small></dd>

<dt>Regular Expression:</dt>
<dd>[/^fell/](#/^fell/) <small>(Any card name that begins with "fell")</small></dd>
<dd>[o:/counter target .* spell/](#o:/counter target .* spell/) <small>(Any card text with "counter target *something* spell")</small></dd>
<dd>[o:/for each .* and\/or .*/](#o:/for each .* and\/or .*/) <small>(/'s can be escaped with a \)</small></dd>

</dl>
