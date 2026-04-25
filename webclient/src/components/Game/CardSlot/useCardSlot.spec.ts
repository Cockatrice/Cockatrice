import { renderHook } from '@testing-library/react';
import { App } from '@app/types';

vi.mock('@app/hooks', async (orig) => {
  const actual = await orig<typeof import('@app/hooks')>();
  return {
    ...actual,
    useScryfallCard: () => ({ smallUrl: null }),
  };
});

const useDroppableMock = vi.fn();
const useDraggableMock = vi.fn();
// Per-test override so we can simulate the dnd-kit active-drag state without
// a real DndContext or sensor stack.
let nextIsDragging = false;
vi.mock('@dnd-kit/core', () => ({
  useDroppable: (opts: unknown) => {
    useDroppableMock(opts);
    return { setNodeRef: vi.fn(), isOver: false };
  },
  useDraggable: (opts: unknown) => {
    useDraggableMock(opts);
    return { setNodeRef: vi.fn(), attributes: {}, listeners: {}, isDragging: nextIsDragging };
  },
}));

vi.mock('../CardRegistry/CardRegistryContext', () => ({
  makeCardKey: (p: number, z: string, c: number) => `${p}-${z}-${c}`,
  useRegisterCardRef: () => vi.fn(),
}));

import { makeCard } from '../../../store/game/__mocks__/fixtures';
import { useCardSlot } from './useCardSlot';

beforeEach(() => {
  useDroppableMock.mockClear();
  useDraggableMock.mockClear();
  nextIsDragging = false;
});

describe('useCardSlot droppable guard', () => {
  it('enables the drop target for an unattached TABLE card', () => {
    const card = makeCard({ id: 1 });
    renderHook(() =>
      useCardSlot({ card, draggable: false, ownerPlayerId: 1, zone: App.ZoneName.TABLE }),
    );
    const dropCall = useDroppableMock.mock.calls[0][0] as { disabled: boolean };
    expect(dropCall.disabled).toBe(false);
  });

  it('disables the drop target when the card is itself attached (a child)', () => {
    const card = makeCard({
      id: 2,
      attachPlayerId: 1,
      attachZone: App.ZoneName.TABLE,
      attachCardId: 1,
    });
    renderHook(() =>
      useCardSlot({ card, draggable: false, ownerPlayerId: 1, zone: App.ZoneName.TABLE }),
    );
    const dropCall = useDroppableMock.mock.calls[0][0] as { disabled: boolean };
    expect(dropCall.disabled).toBe(true);
  });

  it('disables the drop target for non-TABLE zones', () => {
    const card = makeCard({ id: 3 });
    renderHook(() =>
      useCardSlot({ card, draggable: false, ownerPlayerId: 1, zone: App.ZoneName.HAND }),
    );
    const dropCall = useDroppableMock.mock.calls[0][0] as { disabled: boolean };
    expect(dropCall.disabled).toBe(true);
  });

  it('disables the drop target when the owner is unknown', () => {
    const card = makeCard({ id: 4 });
    renderHook(() =>
      useCardSlot({ card, draggable: false, ownerPlayerId: undefined, zone: App.ZoneName.TABLE }),
    );
    const dropCall = useDroppableMock.mock.calls[0][0] as { disabled: boolean };
    expect(dropCall.disabled).toBe(true);
  });

  it('disables the drop target while the card itself is being dragged', () => {
    // Without this gate, dnd-kit's default rectIntersection collision picks
    // the dragged card's own droppable over the BattlefieldRow for short
    // drags — the self-drop guard in useGameDnd then silently no-ops the
    // move and the UI appears not to update.
    nextIsDragging = true;
    const card = makeCard({ id: 5 });
    renderHook(() =>
      useCardSlot({ card, draggable: true, ownerPlayerId: 1, zone: App.ZoneName.TABLE }),
    );
    const dropCall = useDroppableMock.mock.calls[0][0] as { disabled: boolean };
    expect(dropCall.disabled).toBe(true);
  });
});
