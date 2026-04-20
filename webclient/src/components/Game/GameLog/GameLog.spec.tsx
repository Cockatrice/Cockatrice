import { act, screen, fireEvent } from '@testing-library/react';
import type { Enriched } from '@app/types';
import { create } from '@bufbuild/protobuf';
import { Data } from '@app/types';

import { createMockWebClient, makeStoreState, renderWithProviders, makeUser } from '../../../__test-utils__';
import {
  makeGameEntry,
  makePlayerEntry,
  makePlayerProperties,
} from '../../../store/game/__mocks__/fixtures';
import { Actions } from '../../../store/game/game.actions';
import GameLog from './GameLog';

function stateWithMessages(
  players: ReturnType<typeof makePlayerEntry>[],
  messages: Enriched.GameMessage[],
  secondsElapsed = 0,
) {
  const byId: Record<number, ReturnType<typeof makePlayerEntry>> = {};
  for (const p of players) {
    byId[p.properties.playerId] = p;
  }
  return makeStoreState({
    games: {
      games: {
        1: makeGameEntry({ players: byId, messages, secondsElapsed }),
      },
    },
  });
}

describe('GameLog', () => {
  it('shows an empty hint when no messages are present', () => {
    renderWithProviders(<GameLog gameId={1} />, {
      preloadedState: stateWithMessages([], []),
    });

    expect(screen.getByText(/no messages/i)).toBeInTheDocument();
  });

  it('renders each message with the speaking player name', () => {
    const alice = makePlayerEntry({
      properties: makePlayerProperties({
        playerId: 1,
        userInfo: makeUser({ name: 'Alice' }),
      }),
    });
    renderWithProviders(<GameLog gameId={1} />, {
      preloadedState: stateWithMessages(
        [alice],
        [
          { playerId: 1, message: 'gl hf', timeReceived: 0 },
          { playerId: 1, message: 'yolo', timeReceived: 0 },
        ],
      ),
    });

    expect(screen.getByText('gl hf')).toBeInTheDocument();
    expect(screen.getByText('yolo')).toBeInTheDocument();
    expect(screen.getAllByText('Alice:').length).toBe(2);
  });

  it('renders a fallback author label when the speaker is not in the player list', () => {
    renderWithProviders(<GameLog gameId={1} />, {
      preloadedState: stateWithMessages(
        [],
        [{ playerId: 99, message: 'hello', timeReceived: 0 }],
      ),
    });

    expect(screen.getByText('p99:')).toBeInTheDocument();
  });

  it('disables the chat input when gameId is undefined', () => {
    renderWithProviders(<GameLog gameId={undefined} />, {
      preloadedState: makeStoreState({ games: { games: {} } }),
    });

    expect(screen.getByLabelText('game chat input')).toBeDisabled();
  });

  it('enables the chat input when gameId is provided', () => {
    renderWithProviders(<GameLog gameId={1} />, {
      preloadedState: stateWithMessages([], []),
    });

    expect(screen.getByLabelText('game chat input')).not.toBeDisabled();
  });

  it('submits the chat draft and clears the input', () => {
    const webClient = createMockWebClient();
    renderWithProviders(<GameLog gameId={1} />, {
      preloadedState: stateWithMessages([], []),
      webClient,
    });

    const input = screen.getByLabelText('game chat input') as HTMLInputElement;
    fireEvent.change(input, { target: { value: '  hello world  ' } });
    fireEvent.submit(input.closest('form')!);

    expect(webClient.request.game.gameSay).toHaveBeenCalledWith(1, { message: 'hello world' });
    expect(input.value).toBe('');
  });

  it('does not dispatch for a whitespace-only message', () => {
    const webClient = createMockWebClient();
    renderWithProviders(<GameLog gameId={1} />, {
      preloadedState: stateWithMessages([], []),
      webClient,
    });

    const input = screen.getByLabelText('game chat input') as HTMLInputElement;
    fireEvent.change(input, { target: { value: '   ' } });
    fireEvent.submit(input.closest('form')!);

    expect(webClient.request.game.gameSay).not.toHaveBeenCalled();
  });

  describe('event-log rendering (desktop MessageLogWidget parity)', () => {
    // Desktop renders game events (card moves, tap, concede, etc.) in the
    // same log surface as chat, but without a leading speaker label and in a
    // distinct italic style. Regression guard — GameLog was chat-only before
    // this milestone.
    it('renders event messages without a leading author label', () => {
      renderWithProviders(<GameLog gameId={1} />, {
        preloadedState: stateWithMessages(
          [],
          [
            { playerId: 1, message: 'Alice plays Bolt.', timeReceived: 0, kind: 'event' },
          ],
        ),
      });

      expect(screen.getByText('Alice plays Bolt.')).toBeInTheDocument();
      expect(screen.queryByText(/^p\d+:$/)).not.toBeInTheDocument();
      expect(screen.queryByText('Alice:')).not.toBeInTheDocument();
    });

    it('tags event lines with the event modifier class', () => {
      renderWithProviders(<GameLog gameId={1} />, {
        preloadedState: stateWithMessages(
          [],
          [
            { playerId: 0, message: 'The game has started.', timeReceived: 0, kind: 'event' },
          ],
        ),
      });

      const line = screen.getByText('The game has started.').closest('.game-log__line')!;
      expect(line.className).toContain('game-log__line--event');
    });

    it('interleaves chat and event lines in order', () => {
      const alice = makePlayerEntry({
        properties: makePlayerProperties({
          playerId: 1,
          userInfo: makeUser({ name: 'Alice' }),
        }),
      });
      renderWithProviders(<GameLog gameId={1} />, {
        preloadedState: stateWithMessages(
          [alice],
          [
            { playerId: 1, message: 'gl', timeReceived: 0, kind: 'chat' },
            { playerId: 1, message: 'Alice plays Bolt.', timeReceived: 1, kind: 'event' },
            { playerId: 1, message: 'hf', timeReceived: 2, kind: 'chat' },
          ],
        ),
      });

      const lines = Array.from(
        document.querySelectorAll<HTMLElement>('.game-log__line'),
      );
      expect(lines).toHaveLength(3);
      expect(lines[0].textContent).toContain('Alice:');
      expect(lines[1].className).toContain('game-log__line--event');
      expect(lines[2].textContent).toContain('Alice:');
    });
  });

  describe('elapsed game timer', () => {
    beforeEach(() => {
      vi.useFakeTimers({ shouldAdvanceTime: true });
    });

    afterEach(() => {
      vi.useRealTimers();
    });

    it('renders the initial secondsElapsed snapshot in HH:MM:SS form', () => {
      renderWithProviders(<GameLog gameId={1} />, {
        preloadedState: stateWithMessages([], [], 3723),
      });

      expect(screen.getByTestId('game-log-timer')).toHaveTextContent('01:02:03');
    });

    it('advances locally once per second between server events', () => {
      renderWithProviders(<GameLog gameId={1} />, {
        preloadedState: stateWithMessages([], [], 0),
      });

      expect(screen.getByTestId('game-log-timer')).toHaveTextContent('00:00:00');

      act(() => {
        vi.advanceTimersByTime(2000);
      });

      expect(screen.getByTestId('game-log-timer')).toHaveTextContent('00:00:02');
    });

    it('does not render the timer when there is no active game', () => {
      renderWithProviders(<GameLog gameId={undefined} />, {
        preloadedState: makeStoreState({ games: { games: {} } }),
      });

      expect(screen.queryByTestId('game-log-timer')).not.toBeInTheDocument();
    });

    // Mirrors desktop's setGameTime resync: the local 1Hz ticker drifts until
    // the server pushes a fresh `secondsElapsed`, at which point the display
    // snaps to the server value. Regression guard for that snap behavior.
    it('resyncs displayed time when Redux pushes a new secondsElapsed', () => {
      const { store } = renderWithProviders(<GameLog gameId={1} />, {
        preloadedState: stateWithMessages([], [], 10),
      });

      expect(screen.getByTestId('game-log-timer')).toHaveTextContent('00:00:10');

      // Local ticker drifts forward between server events.
      act(() => {
        vi.advanceTimersByTime(3000);
      });
      expect(screen.getByTestId('game-log-timer')).toHaveTextContent('00:00:13');

      // Server pushes a fresh snapshot (real reducer path).
      act(() => {
        store.dispatch(Actions.gameStateChanged({
          gameId: 1,
          data: create(Data.Event_GameStateChangedSchema, { secondsElapsed: 120 }),
        }));
      });

      // Display snaps to the server value, not the drifted local value.
      expect(screen.getByTestId('game-log-timer')).toHaveTextContent('00:02:00');
    });
  });
});
