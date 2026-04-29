import { useRef } from 'react';
import { screen, fireEvent } from '@testing-library/react';
import { create } from '@bufbuild/protobuf';
import { Data } from '@app/types';

import { createMockWebClient, makeStoreState, renderWithProviders } from '../../../__test-utils__';
import {
  makeArrow,
  makeGameEntry,
  makePlayerEntry,
  makePlayerProperties,
} from '../../../store/game/__mocks__/fixtures';
import GameArrowOverlay from './GameArrowOverlay';
import {
  CardRegistryContext,
  createCardRegistry,
  makeCardKey,
} from '../CardRegistry/CardRegistryContext';

function Harness({ gameId }: { gameId: number }) {
  const ref = useRef<HTMLDivElement>(null);
  return (
    <div ref={ref} data-testid="arrow-harness-root" style={{ position: 'relative', width: 600, height: 400 }}>
      <GameArrowOverlay gameId={gameId} boardRef={ref} />
    </div>
  );
}

function setupRegistryWithTwoCards() {
  const registry = createCardRegistry();
  const elA = document.createElement('div');
  elA.getBoundingClientRect = () =>
    ({ left: 100, top: 100, width: 50, height: 50, right: 150, bottom: 150, x: 100, y: 100, toJSON: () => ({}) } as DOMRect);
  const elB = document.createElement('div');
  elB.getBoundingClientRect = () =>
    ({ left: 300, top: 300, width: 50, height: 50, right: 350, bottom: 350, x: 300, y: 300, toJSON: () => ({}) } as DOMRect);

  // Must be attached to the DOM for the registry subscribers to fire after mount.
  document.body.appendChild(elA);
  document.body.appendChild(elB);

  registry.register(makeCardKey(1, 'table', 10), elA);
  registry.register(makeCardKey(1, 'table', 11), elB);
  return { registry, elA, elB };
}

function stateWithOneArrow() {
  const arrow = makeArrow({
    id: 1,
    startPlayerId: 1,
    startZone: 'table',
    startCardId: 10,
    targetPlayerId: 1,
    targetZone: 'table',
    targetCardId: 11,
    arrowColor: create(Data.colorSchema, { r: 224, g: 75, b: 59, a: 255 }),
  });
  return makeStoreState({
    games: {
      games: {
        1: makeGameEntry({
          players: {
            1: makePlayerEntry({
              properties: makePlayerProperties({ playerId: 1 }),
              arrows: { 1: arrow },
            }),
          },
        }),
      },
    },
  });
}

function wrapWithRegistry(children: React.ReactNode, registry: ReturnType<typeof createCardRegistry>) {
  return (
    <CardRegistryContext.Provider value={registry}>
      {children}
    </CardRegistryContext.Provider>
  );
}

describe('GameArrowOverlay', () => {
  it('renders an SVG root when mounted', () => {
    const { registry } = setupRegistryWithTwoCards();
    renderWithProviders(wrapWithRegistry(<Harness gameId={1} />, registry), {
      preloadedState: stateWithOneArrow(),
    });

    expect(screen.getByTestId('game-arrow-overlay')).toBeInTheDocument();
  });

  it('renders a line for each arrow with endpoints at card centers relative to the board', () => {
    const { registry } = setupRegistryWithTwoCards();
    // Pretend the board rect starts at 0,0 for simplicity; card A center is
    // (125, 125) and card B center is (325, 325) in viewport coords — same in
    // board-relative coords since the harness root is at 0,0.
    renderWithProviders(wrapWithRegistry(<Harness gameId={1} />, registry), {
      preloadedState: stateWithOneArrow(),
    });

    const line = screen.getByTestId('arrow-1');
    expect(line.getAttribute('x1')).toBe('125');
    expect(line.getAttribute('y1')).toBe('125');
    expect(line.getAttribute('x2')).toBe('325');
    expect(line.getAttribute('y2')).toBe('325');
  });

  it('skips arrows whose endpoints are not registered yet', () => {
    const registry = createCardRegistry();
    renderWithProviders(wrapWithRegistry(<Harness gameId={1} />, registry), {
      preloadedState: stateWithOneArrow(),
    });

    expect(screen.queryByTestId('arrow-1')).not.toBeInTheDocument();
  });

  it('dispatches deleteArrow when an arrow line is clicked', () => {
    const webClient = createMockWebClient();
    const { registry } = setupRegistryWithTwoCards();
    renderWithProviders(wrapWithRegistry(<Harness gameId={1} />, registry), {
      preloadedState: stateWithOneArrow(),
      webClient,
    });

    fireEvent.click(screen.getByTestId('arrow-1'));

    expect(webClient.request.game.deleteArrow).toHaveBeenCalledWith(1, { arrowId: 1 });
  });
});
