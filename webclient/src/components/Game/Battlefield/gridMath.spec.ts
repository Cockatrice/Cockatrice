import { makeCard } from '../../../store/game/__mocks__/fixtures';
import {
  CARD_WIDTH_PX,
  PADDING_X_PX,
  STACKED_CARD_OFFSET_X_PX,
  applyInvertY,
  closestGridPoint,
  mapToGridX,
  stackColumnWidth,
  stackCountsForRow,
} from './gridMath';

describe('gridMath', () => {
  describe('stackColumnWidth', () => {
    it('returns a single card width for an empty or 1-card stack', () => {
      expect(stackColumnWidth(0)).toBe(CARD_WIDTH_PX);
      expect(stackColumnWidth(1)).toBe(CARD_WIDTH_PX);
    });

    it('adds one sub-position offset per extra card, up to MAX_SUBPOS', () => {
      expect(stackColumnWidth(2)).toBe(CARD_WIDTH_PX + STACKED_CARD_OFFSET_X_PX);
      expect(stackColumnWidth(3)).toBe(CARD_WIDTH_PX + 2 * STACKED_CARD_OFFSET_X_PX);
      // A 4th card would overflow to the next stack; width is capped at 3.
      expect(stackColumnWidth(4)).toBe(CARD_WIDTH_PX + 2 * STACKED_CARD_OFFSET_X_PX);
    });
  });

  describe('stackCountsForRow', () => {
    it('groups cards by Math.floor(x / MAX_SUBPOS)', () => {
      const cards = [
        makeCard({ id: 1, x: 0 }),
        makeCard({ id: 2, x: 1 }),
        makeCard({ id: 3, x: 3 }),
        makeCard({ id: 4, x: 7 }),
      ];
      const counts = stackCountsForRow(cards);
      expect(counts.get(0)).toBe(2); // x=0,1 → col 0
      expect(counts.get(1)).toBe(1); // x=3 → col 1
      expect(counts.get(2)).toBe(1); // x=7 → col 2
    });

    it('treats missing x as 0', () => {
      const counts = stackCountsForRow([makeCard({ id: 1 })]);
      expect(counts.get(0)).toBe(1);
    });
  });

  describe('mapToGridX', () => {
    it('returns 0 at the left edge of an empty row', () => {
      expect(mapToGridX(0, new Map())).toBe(0);
    });

    it('returns sub-positions 0, 1, 2 within a single empty stack column', () => {
      const counts = new Map<number, number>();
      expect(mapToGridX(0, counts)).toBe(0);
      expect(mapToGridX(STACKED_CARD_OFFSET_X_PX, counts)).toBe(1);
      expect(mapToGridX(2 * STACKED_CARD_OFFSET_X_PX, counts)).toBe(2);
    });

    it('clamps sub-position to MAX_SUBPOS - 1 inside a stack', () => {
      // Far-right of stack 0 (pointer just before stack 1 starts) still returns 2.
      const counts = new Map<number, number>();
      const pointerJustInsideStack0 = CARD_WIDTH_PX - 1;
      expect(mapToGridX(pointerJustInsideStack0, counts)).toBe(2);
    });

    it('advances to the next stack base when pointer crosses padding', () => {
      // One card already in stack 0; width = CARD_WIDTH_PX. Pointer past the
      // padding lands at the base of stack 1 → gridX = 3.
      const counts = new Map<number, number>([[0, 1]]);
      const pointerInStack1 = CARD_WIDTH_PX + PADDING_X_PX;
      expect(mapToGridX(pointerInStack1, counts)).toBe(3);
    });

    it('accounts for variable stack widths when walking past multiple stacks', () => {
      // Stack 0 is full (3 cards → width = CARD + 2*OFFSET).
      // Stack 1 has 1 card (width = CARD).
      // Pointer past both should land in stack 2 at sub-position 0 → gridX = 6.
      const counts = new Map<number, number>([
        [0, 3],
        [1, 1],
      ]);
      const stack0 = stackColumnWidth(3);
      const stack1 = stackColumnWidth(1);
      const pointerInStack2 = stack0 + PADDING_X_PX + stack1 + PADDING_X_PX;
      expect(mapToGridX(pointerInStack2, counts)).toBe(6);
    });

    it('snaps to the nearer stack via desktop-style PADDING_X / 2 rounding', () => {
      // Pointer halfway into the padding between stack 0 and stack 1 should
      // round forward to stack 1 (matches desktop line 340: + PADDING_X / 2).
      const counts = new Map<number, number>([[0, 1]]);
      const pointerInPaddingMiddle = CARD_WIDTH_PX + PADDING_X_PX / 2;
      expect(mapToGridX(pointerInPaddingMiddle, counts)).toBe(3);
    });

    it('treats a pointer left of the content edge as sub-position 0', () => {
      expect(mapToGridX(-50, new Map())).toBe(0);
    });
  });

  describe('closestGridPoint', () => {
    it('rounds an unoccupied gridX down to its stack base', () => {
      expect(closestGridPoint(2, new Set())).toBe(0);
      expect(closestGridPoint(5, new Set())).toBe(3);
    });

    it('increments by 1 when the base is occupied', () => {
      expect(closestGridPoint(0, new Set([0]))).toBe(1);
    });

    it('increments by 2 when base and base+1 are occupied', () => {
      expect(closestGridPoint(0, new Set([0, 1]))).toBe(2);
    });

    it('returns null when all sub-positions in the stack are occupied', () => {
      expect(closestGridPoint(0, new Set([0, 1, 2]))).toBeNull();
    });

    it('resolves within the target stack regardless of the initial sub-position', () => {
      // gridX = 4 rounds to base 3; base is free → returns 3.
      expect(closestGridPoint(4, new Set())).toBe(3);
      // gridX = 4 with base occupied → returns base+1 = 4.
      expect(closestGridPoint(4, new Set([3]))).toBe(4);
      expect(closestGridPoint(5, new Set([3, 4]))).toBe(5);
    });
  });

  describe('applyInvertY', () => {
    it('returns y unchanged when not inverted', () => {
      expect(applyInvertY(0, false)).toBe(0);
      expect(applyInvertY(1, false)).toBe(1);
      expect(applyInvertY(2, false)).toBe(2);
    });

    it('mirrors y across the 3-row span when inverted', () => {
      expect(applyInvertY(0, true)).toBe(2);
      expect(applyInvertY(1, true)).toBe(1);
      expect(applyInvertY(2, true)).toBe(0);
    });

    it('clamps out-of-range y before inverting', () => {
      expect(applyInvertY(-1, false)).toBe(0);
      expect(applyInvertY(99, false)).toBe(2);
      expect(applyInvertY(-1, true)).toBe(2);
    });
  });
});
