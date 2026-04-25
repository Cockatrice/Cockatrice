import { renderHook } from '@testing-library/react';
import type { DragEndEvent } from '@dnd-kit/core';
import { App, Data } from '@app/types';

import { makeCard } from '../../store/game/__mocks__/fixtures';
import {
  CARD_WIDTH_PX,
  MARGIN_LEFT_PX,
  PADDING_X_PX,
  STACKED_CARD_OFFSET_X_PX,
} from '../../components/Game/Battlefield/gridMath';

const { mockUseWebClient } = vi.hoisted(() => ({ mockUseWebClient: vi.fn() }));
vi.mock('@app/hooks', async (importOriginal) => {
  const actual = await importOriginal<typeof import('@app/hooks')>();
  return { ...actual, useWebClient: mockUseWebClient };
});

import { useGameDnd } from './useGameDnd';

// Build a minimal DragEndEvent with only the fields useGameDnd reads. The
// real dnd-kit Event type has many more fields; we cast via `unknown` so
// tests stay tied to actual handler inputs, not dnd-kit internals.
type DropShape = {
  sourceCard: Data.ServerInfo_Card;
  sourcePlayerId: number;
  sourceZone: string;
  targetPlayerId: number;
  targetZone: string;
  targetRow?: number;
  rowCards?: Data.ServerInfo_Card[];
  // Drop-point is expressed as the card center's x relative to the row's
  // content edge (same convention as gridMath.mapToGridX).
  pointerXInRow?: number;
  attachTarget?: boolean;
  targetCardId?: number;
};

const ROW_LEFT = 500;
const CARD_WIDTH_ON_SCREEN = CARD_WIDTH_PX;

function buildEvent(d: DropShape): DragEndEvent {
  const pointerXInRow = d.pointerXInRow ?? 0;
  // Reverse the math computePointerXInRow does: cardCenterX - overRect.left - MARGIN_LEFT.
  const cardCenterXAbs = pointerXInRow + ROW_LEFT + MARGIN_LEFT_PX;
  const activeLeft = cardCenterXAbs - CARD_WIDTH_ON_SCREEN / 2;
  return {
    active: {
      id: d.sourceCard.id,
      data: {
        current: {
          card: d.sourceCard,
          sourcePlayerId: d.sourcePlayerId,
          sourceZone: d.sourceZone,
        },
      },
      rect: {
        current: {
          translated: {
            left: activeLeft,
            top: 0,
            width: CARD_WIDTH_ON_SCREEN,
            height: 204,
            right: activeLeft + CARD_WIDTH_ON_SCREEN,
            bottom: 204,
          },
          initial: null,
        },
      },
    },
    over: {
      id: `battlefield-${d.targetPlayerId}-${d.targetRow ?? 0}`,
      // Row height = 204 so the effective card width derived by useGameDnd
      // (laneHeight × 146/204) equals CARD_WIDTH_PX = 146, keeping the
      // pointer-X math aligned with the test's logical gridMath expectations.
      rect: {
        left: ROW_LEFT,
        top: 0,
        width: 1000,
        height: 204,
        right: ROW_LEFT + 1000,
        bottom: 204,
      } as any,
      data: {
        current: {
          targetPlayerId: d.targetPlayerId,
          targetZone: d.targetZone,
          row: d.targetRow ?? 0,
          rowCards: d.rowCards ?? [],
          attachTarget: d.attachTarget,
          targetCardId: d.targetCardId,
        },
      },
      disabled: false,
    } as any,
    delta: { x: 0, y: 0 },
    collisions: null,
    activatorEvent: new Event('pointerdown'),
  } as unknown as DragEndEvent;
}

function makeWebClient() {
  return {
    request: {
      game: {
        moveCard: vi.fn(),
        attachCard: vi.fn(),
      },
    },
  } as any;
}

function setupHook() {
  const webClient = makeWebClient();
  mockUseWebClient.mockReturnValue(webClient);
  const onDragStart = vi.fn();
  const { result } = renderHook(() => useGameDnd({ gameId: 42, onDragStart }));
  return { webClient, onDragStart, handleDragEnd: result.current.handleDragEnd };
}

describe('useGameDnd', () => {
  beforeEach(() => {
    vi.clearAllMocks();
  });

  describe('drops on battlefield', () => {
    it('sends moveCard with gridX=0 when dropping into an empty row', () => {
      const { webClient, handleDragEnd } = setupHook();
      const card = makeCard({ id: 10, x: 0, y: 0 });
      handleDragEnd(
        buildEvent({
          sourceCard: card,
          sourcePlayerId: 1,
          sourceZone: App.ZoneName.HAND,
          targetPlayerId: 1,
          targetZone: App.ZoneName.TABLE,
          targetRow: 1,
          rowCards: [],
          pointerXInRow: 10,
        }),
      );

      expect(webClient.request.game.moveCard).toHaveBeenCalledTimes(1);
      const args = webClient.request.game.moveCard.mock.calls[0][1];
      expect(args.x).toBe(0);
      expect(args.y).toBe(1);
      expect(args.targetZone).toBe(App.ZoneName.TABLE);
    });

    it('sends moveCard with gridX=3 when dropping into the next stack past a 1-card stack', () => {
      const { webClient, handleDragEnd } = setupHook();
      const existing = makeCard({ id: 20, x: 0, y: 0 });
      const dragging = makeCard({ id: 21, x: 0, y: 2 }); // from a different row
      handleDragEnd(
        buildEvent({
          sourceCard: dragging,
          sourcePlayerId: 1,
          sourceZone: App.ZoneName.TABLE,
          targetPlayerId: 1,
          targetZone: App.ZoneName.TABLE,
          targetRow: 0,
          rowCards: [existing],
          // Pointer well inside stack 1's territory.
          pointerXInRow: CARD_WIDTH_PX + PADDING_X_PX + 10,
        }),
      );

      const args = webClient.request.game.moveCard.mock.calls[0][1];
      expect(args.x).toBe(3);
    });

    it('allows in-row reorder (same-row drop no longer silently rejected)', () => {
      const { webClient, handleDragEnd } = setupHook();
      const leftmost = makeCard({ id: 30, x: 0, y: 1 });
      const dragged = makeCard({ id: 31, x: 1, y: 1 });
      handleDragEnd(
        buildEvent({
          sourceCard: dragged,
          sourcePlayerId: 1,
          sourceZone: App.ZoneName.TABLE,
          targetPlayerId: 1,
          targetZone: App.ZoneName.TABLE,
          targetRow: 1,
          // Dragged card still appears in rowCards from Redux during the drag.
          rowCards: [leftmost, dragged],
          // Drop far enough right that the dragged card lands in a new stack.
          pointerXInRow: CARD_WIDTH_PX + PADDING_X_PX + 10,
        }),
      );

      expect(webClient.request.game.moveCard).toHaveBeenCalledTimes(1);
      // Dragged card excluded from occupancy → stack 1 base (3) is free.
      expect(webClient.request.game.moveCard.mock.calls[0][1].x).toBe(3);
    });

    it('places into sub-position 1 when the stack base is occupied by another card', () => {
      const { webClient, handleDragEnd } = setupHook();
      const occupying = makeCard({ id: 40, x: 0, y: 0 });
      const dragging = makeCard({ id: 41, x: 0, y: 2 }); // different row
      handleDragEnd(
        buildEvent({
          sourceCard: dragging,
          sourcePlayerId: 1,
          sourceZone: App.ZoneName.TABLE,
          targetPlayerId: 1,
          targetZone: App.ZoneName.TABLE,
          targetRow: 0,
          rowCards: [occupying],
          // Inside stack 0's territory (mapToGridX returns something in [0,2]).
          pointerXInRow: STACKED_CARD_OFFSET_X_PX / 2,
        }),
      );

      // base 0 occupied → closestGridPoint bumps to 1.
      expect(webClient.request.game.moveCard.mock.calls[0][1].x).toBe(1);
    });

    it('rejects a drop silently when the target stack is fully occupied', () => {
      const { webClient, handleDragEnd } = setupHook();
      const dragging = makeCard({ id: 50, x: 0, y: 2 });
      const full = [
        makeCard({ id: 51, x: 0, y: 0 }),
        makeCard({ id: 52, x: 1, y: 0 }),
        makeCard({ id: 53, x: 2, y: 0 }),
      ];
      handleDragEnd(
        buildEvent({
          sourceCard: dragging,
          sourcePlayerId: 1,
          sourceZone: App.ZoneName.TABLE,
          targetPlayerId: 1,
          targetZone: App.ZoneName.TABLE,
          targetRow: 0,
          rowCards: full,
          pointerXInRow: 0, // mapToGridX → 0; all three sub-slots full.
        }),
      );

      expect(webClient.request.game.moveCard).not.toHaveBeenCalled();
    });

    it('sends the logical y from target.row without re-inverting (mirrored boards)', () => {
      // BattlefieldRow receives `row=rowIdx` where rowIdx iterates rowOrder
      // (mirrored: [2,1,0]). Visual top of a mirrored opponent yields
      // target.row = 2 which IS the correct server-side y. Re-inverting here
      // would send y=0 and render the card at the bottom of the opponent's
      // area — the bug this test guards against.
      const { webClient, handleDragEnd } = setupHook();
      const dragging = makeCard({ id: 60, x: 0, y: 0 });
      handleDragEnd(
        buildEvent({
          sourceCard: dragging,
          sourcePlayerId: 1,
          sourceZone: App.ZoneName.HAND,
          targetPlayerId: 2,
          targetZone: App.ZoneName.TABLE,
          targetRow: 2, // visual top of mirrored opponent = logical y=2
          rowCards: [],
          pointerXInRow: 0,
        }),
      );

      expect(webClient.request.game.moveCard.mock.calls[0][1].y).toBe(2);
    });
  });

  describe('non-TABLE drops', () => {
    it('sends x=0, y=0 when dropping into a pile zone (graveyard)', () => {
      const { webClient, handleDragEnd } = setupHook();
      const card = makeCard({ id: 70, x: 0, y: 0 });
      handleDragEnd(
        buildEvent({
          sourceCard: card,
          sourcePlayerId: 1,
          sourceZone: App.ZoneName.TABLE,
          targetPlayerId: 1,
          targetZone: App.ZoneName.GRAVE,
          targetRow: 0,
          pointerXInRow: 500, // ignored for non-TABLE
        }),
      );

      const args = webClient.request.game.moveCard.mock.calls[0][1];
      expect(args.x).toBe(0);
      expect(args.y).toBe(0);
      expect(args.targetZone).toBe(App.ZoneName.GRAVE);
    });

    it('skips dispatch for same-zone drops on non-TABLE zones', () => {
      const { webClient, handleDragEnd } = setupHook();
      const card = makeCard({ id: 80, x: 0, y: 0 });
      handleDragEnd(
        buildEvent({
          sourceCard: card,
          sourcePlayerId: 1,
          sourceZone: App.ZoneName.HAND,
          targetPlayerId: 1,
          targetZone: App.ZoneName.HAND,
          targetRow: 0,
        }),
      );

      expect(webClient.request.game.moveCard).not.toHaveBeenCalled();
    });
  });

  describe('attach drops', () => {
    it('dispatches attachCard when dropping a table card onto another table card', () => {
      const { webClient, handleDragEnd } = setupHook();
      const source = makeCard({ id: 90, x: 0, y: 0 });
      handleDragEnd(
        buildEvent({
          sourceCard: source,
          sourcePlayerId: 1,
          sourceZone: App.ZoneName.TABLE,
          targetPlayerId: 1,
          targetZone: App.ZoneName.TABLE,
          targetRow: 0,
          attachTarget: true,
          targetCardId: 91,
        }),
      );

      expect(webClient.request.game.attachCard).toHaveBeenCalledTimes(1);
      expect(webClient.request.game.moveCard).not.toHaveBeenCalled();
    });
  });
});
