// Port of desktop Cockatrice's table-zone grid math (cockatrice/src/game/zones/table_zone.cpp).
// The server wire protocol encodes table-zone card positions as integers:
//   x = stackColumn * MAX_SUBPOS + subPosition   (subPosition ∈ {0, 1, 2})
//   y ∈ {0, 1, 2}                                (pre-inversion row index)
// Desktop packs up to MAX_SUBPOS cards into a single stack column via small
// horizontal offsets; once full, drops overflow to the next stack. We replicate
// those semantics exactly so cards rendered side-by-side on desktop land on the
// same grid points in the webclient.

import { Data } from '@app/types';

// Nominal pixel constants at the reference zoom (146x204 card). Cards scale
// with lane height in CSS via `aspect-ratio: 146/204`; at runtime the drop
// handler derives the effective card width from the row's measured height
// (effectiveCardWidth = laneHeight × 146/204) and passes it to mapToGridX.
// PADDING_X and MARGIN_LEFT match fixed CSS gap/padding so they don't scale.
export const CARD_WIDTH_PX = 146; // desktop CardDimensions::WIDTH = 72
export const CARD_HEIGHT_PX = 204;
export const STACKED_CARD_OFFSET_X_PX = 49; // desktop WIDTH/3 = 24 → 146/3 ≈ 49 preserves ratio
export const PADDING_X_PX = 16; // desktop PADDING_X = 35; smaller so rows fit more stacks at browser widths
export const MARGIN_LEFT_PX = 8; // matches .battlefield__row horizontal padding

export const ROW_COUNT = 3;
export const MAX_SUBPOS = 3; // cards per stack column before overflow

export function clampRow(y: number): number {
  if (y < 0) {
    return 0;
  }
  if (y >= ROW_COUNT) {
    return ROW_COUNT - 1;
  }
  return y;
}

// Width a stack column occupies given how many cards it currently holds.
// A 1-card stack is cardWidth wide; each additional sub-positioned card
// extends the stack by offsetX (up to MAX_SUBPOS - 1 extra). Defaults to the
// nominal 146/49 constants; callers that need to account for lane scaling
// pass the effective width and offset derived from the row's measured height.
export function stackColumnWidth(
  cardCount: number,
  cardWidth: number = CARD_WIDTH_PX,
  offsetX: number = STACKED_CARD_OFFSET_X_PX,
): number {
  if (cardCount <= 1) {
    return cardWidth;
  }
  const extras = Math.min(cardCount - 1, MAX_SUBPOS - 1);
  return cardWidth + extras * offsetX;
}

// Count cards per stack column in a row. `cards` must be the row's cards
// (after filtering attached children, which do not occupy their own slot).
export function stackCountsForRow(cards: Data.ServerInfo_Card[]): Map<number, number> {
  const counts = new Map<number, number>();
  for (const card of cards) {
    const col = Math.floor((card.x ?? 0) / MAX_SUBPOS);
    counts.set(col, (counts.get(col) ?? 0) + 1);
  }
  return counts;
}

// Port of table_zone.cpp:mapToGrid's x-axis walk (lines 336-363).
// `pointerXInRow` is the drop pointer's x-coordinate relative to the row's
// left content edge (i.e. already offset past any CSS padding). We mirror
// desktop's "+ paddingX/2" rounding to snap to the nearest stack.
export function mapToGridX(
  pointerXInRow: number,
  stackCounts: Map<number, number>,
  cardWidth: number = CARD_WIDTH_PX,
  offsetX: number = STACKED_CARD_OFFSET_X_PX,
  paddingX: number = PADDING_X_PX,
): number {
  // Desktop shifts by paddingX/2 so a pointer near a stack boundary rounds
  // to the nearer stack. MARGIN_LEFT is the caller's responsibility.
  const x = pointerXInRow + paddingX / 2;

  let xStack = 0;
  let xNextStack = 0;
  let nextStackCol = 0;
  while (xNextStack <= x) {
    xStack = xNextStack;
    const w = stackColumnWidth(stackCounts.get(nextStackCol) ?? 0, cardWidth, offsetX);
    xNextStack += w + paddingX;
    nextStackCol++;
    // Safety: the loop always terminates because x is finite and each iter
    // grows xNextStack by at least cardWidth + paddingX > 0.
  }
  const stackCol = Math.max(nextStackCol - 1, 0);
  const xDiff = Math.max(0, x - xStack);
  const subPos = Math.min(Math.floor(xDiff / offsetX), MAX_SUBPOS - 1);
  return stackCol * MAX_SUBPOS + subPos;
}

// Port of table_zone.cpp:closestGridPoint (lines 366-375). Rounds x down to
// its stack base, then bumps by +1 / +2 over occupied sub-slots. Returns null
// when all MAX_SUBPOS slots in the stack are taken — desktop handles this by
// dropping the card out of the drag list (card_drag_item.cpp:115), resulting
// in a silent reject. Callers should skip dispatching moveCard on null.
export function closestGridPoint(
  gridX: number,
  occupiedXs: ReadonlySet<number>,
): number | null {
  const base = Math.floor(gridX / MAX_SUBPOS) * MAX_SUBPOS;
  for (let i = 0; i < MAX_SUBPOS; i++) {
    if (!occupiedXs.has(base + i)) {
      return base + i;
    }
  }
  return null;
}

// Port of table_zone.cpp:53-59. Inversion flips y at the grid-math boundary
// only — rendering already reverses rowOrder in useBattlefield, so here we
// invert when sending a move to a mirrored board (or when the user has the
// invertVerticalCoordinate setting on and the board isn't already mirrored).
export function applyInvertY(gridY: number, isInverted: boolean): number {
  const clamped = clampRow(gridY);
  return isInverted ? ROW_COUNT - 1 - clamped : clamped;
}
