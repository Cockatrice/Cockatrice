import { screen, fireEvent } from '@testing-library/react';

import { makeStoreState, renderWithProviders, makeUser } from '../../__test-utils__';
import {
  makeGameEntry,
  makePlayerEntry,
  makePlayerProperties,
} from '../../store/game/__mocks__/fixtures';
import GameInfoDialog from './GameInfoDialog';

function stateWithGame(overrides: Partial<Parameters<typeof makeGameEntry>[0]> = {}) {
  return makeStoreState({
    games: {
      games: {
        1: makeGameEntry({
          hostId: 1,
          localPlayerId: 1,
          started: true,
          secondsElapsed: 3723,
          players: {
            1: makePlayerEntry({
              properties: makePlayerProperties({
                playerId: 1,
                userInfo: makeUser({ name: 'Alice' }),
              }),
            }),
            2: makePlayerEntry({
              properties: makePlayerProperties({
                playerId: 2,
                userInfo: makeUser({ name: 'Bob' }),
              }),
            }),
          },
          ...overrides,
        }),
      },
    },
  });
}

describe('GameInfoDialog', () => {
  it('renders game id, started flag, elapsed, and host name', () => {
    renderWithProviders(
      <GameInfoDialog isOpen gameId={1} onClose={() => {}} />,
      { preloadedState: stateWithGame() },
    );

    expect(screen.getByText(/game id/i)).toBeInTheDocument();
    expect(screen.getByText(/elapsed/i)).toBeInTheDocument();
    expect(screen.getByText('01:02:03')).toBeInTheDocument();
    // Alice appears twice: once as the Host <dd>, once in the players list.
    expect(screen.getAllByText('Alice').length).toBeGreaterThan(0);
  });

  it('tags the local player with "you" and the host with "host"', () => {
    renderWithProviders(
      <GameInfoDialog isOpen gameId={1} onClose={() => {}} />,
      { preloadedState: stateWithGame() },
    );

    // Scope to the player-list row, not the Host <dd>.
    const aliceNameSpans = screen
      .getAllByText('Alice')
      .filter((el) => el.classList.contains('game-info-dialog__player-name'));
    const aliceRow = aliceNameSpans[0].closest('li')!;
    expect(aliceRow.textContent).toMatch(/host/);
    expect(aliceRow.textContent).toMatch(/you/);
  });

  it('calls onClose when the close button is clicked', () => {
    const onClose = vi.fn();
    renderWithProviders(
      <GameInfoDialog isOpen gameId={1} onClose={onClose} />,
      { preloadedState: stateWithGame() },
    );

    // Two buttons match /close/i: the header IconButton (aria-label) and the
    // text "Close" button in the dialog footer. Use the footer one explicitly.
    fireEvent.click(screen.getByRole('button', { name: 'Close' }));
    expect(onClose).toHaveBeenCalled();
  });

  it('returns null when the game is missing', () => {
    const { container } = renderWithProviders(
      <GameInfoDialog isOpen gameId={999} onClose={() => {}} />,
      { preloadedState: stateWithGame() },
    );
    expect(container.querySelector('.GameInfoDialog')).toBeNull();
  });
});
