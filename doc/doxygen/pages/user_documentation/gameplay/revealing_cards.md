@page revealing_cards Revealing Cards

The **Hand** menu offers **Reveal a chosen number of random cards to...**. Choose **All players** or a specific player, then choose how many cards to reveal. The count dialog displays the chosen recipient.

The server chooses distinct cards without replacement. Two copies of the same card are separate physical cards and can both be chosen. The operation does not reorder, move, or otherwise change the hand.

When you choose a specific player, that player receives the selected card identities. Judges and spectators configured to see all hidden information retain their usual access. Ordinary participants other than the chosen player, including the hand's owner, receive a notice with the count and recipient, but do not learn which cards were chosen. Revealing to all players sends the selected identities to everyone, using the ordinary reveal window.

If the hand becomes too small or the recipient leaves before the server processes the request, the request fails without a partial reveal. Cancelling the count dialog sends nothing.

This operation requires a server that supports multiple random reveals. An older server rejects it without revealing any cards. The existing **Reveal random card to...** action and its shortcut remain available for single-card reveals.
