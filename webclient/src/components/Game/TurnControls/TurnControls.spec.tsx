import { screen, fireEvent } from '@testing-library/react';

vi.mock('../../../hooks/useSettings');

import { useSettings } from '../../../hooks/useSettings';
import { makeSettings, makeSettingsHook } from '../../../hooks/__mocks__/useSettings';
import { LoadingState } from '../../../hooks/useSharedStore';
import { createMockWebClient, makeStoreState, renderWithProviders, makeUser } from '../../../__test-utils__';
import {
  makeGameEntry,
  makePlayerEntry,
  makePlayerProperties,
} from '../../../store/game/__mocks__/fixtures';
import TurnControls from './TurnControls';

function stateWith(opts: {
  localPlayerId?: number;
  activePlayerId?: number;
  started?: boolean;
  conceded?: boolean;
  judge?: boolean;
  spectator?: boolean;
  hostId?: number;
  opponentIds?: number[];
} = {}) {
  const localId = opts.localPlayerId ?? 1;
  const opponentIds = opts.opponentIds ?? [];
  const players: Record<number, ReturnType<typeof makePlayerEntry>> = {
    [localId]: makePlayerEntry({
      properties: makePlayerProperties({
        playerId: localId,
        userInfo: makeUser({ name: `P${localId}` }),
        conceded: opts.conceded ?? false,
      }),
    }),
  };
  for (const id of opponentIds) {
    players[id] = makePlayerEntry({
      properties: makePlayerProperties({
        playerId: id,
        userInfo: makeUser({ name: `P${id}` }),
      }),
    });
  }
  return makeStoreState({
    games: {
      games: {
        1: makeGameEntry({
          localPlayerId: localId,
          activePlayerId: opts.activePlayerId ?? localId,
          started: opts.started ?? true,
          judge: opts.judge ?? false,
          spectator: opts.spectator ?? false,
          hostId: opts.hostId ?? localId,
          players,
        }),
      },
    },
  });
}

const NOOP = () => {};
const DEFAULT_TURN_PROPS = {
  gameId: 1,
  onRequestRollDie: NOOP,
  onRequestConcede: NOOP,
  onRequestUnconcede: NOOP,
  onRequestGameInfo: NOOP,
  onToggleRotate90: NOOP,
  isRotated: false,
};

describe('TurnControls', () => {
  beforeEach(() => {
    vi.mocked(useSettings).mockReturnValue(makeSettingsHook());
  });

  it('renders core buttons', () => {
    renderWithProviders(<TurnControls {...DEFAULT_TURN_PROPS} />, {
      preloadedState: stateWith(),
    });

    expect(screen.getByRole('button', { name: /pass turn/i })).toBeInTheDocument();
    expect(screen.getByRole('button', { name: /reverse turn/i })).toBeInTheDocument();
    expect(screen.getByRole('button', { name: /next phase/i })).toBeInTheDocument();
    expect(screen.getByRole('button', { name: /^concede$/i })).toBeInTheDocument();
    expect(screen.getByRole('button', { name: /roll die/i })).toBeInTheDocument();
    expect(screen.getByRole('button', { name: /remove arrows/i })).toBeInTheDocument();
    expect(screen.getByRole('button', { name: /rotate 90/i })).toBeInTheDocument();
    expect(screen.getByRole('button', { name: /game info/i })).toBeInTheDocument();
    expect(screen.getByRole('button', { name: /leave game/i })).toBeInTheDocument();
  });

  it('dispatches nextTurn on Pass Turn when the local player is active', () => {
    const webClient = createMockWebClient();
    renderWithProviders(<TurnControls {...DEFAULT_TURN_PROPS} />, {
      preloadedState: stateWith(),
      webClient,
    });

    fireEvent.click(screen.getByRole('button', { name: /pass turn/i }));

    expect(webClient.request.game.nextTurn).toHaveBeenCalledWith(1);
  });

  it('dispatches reverseTurn on Reverse Turn', () => {
    const webClient = createMockWebClient();
    renderWithProviders(<TurnControls {...DEFAULT_TURN_PROPS} />, {
      preloadedState: stateWith(),
      webClient,
    });

    fireEvent.click(screen.getByRole('button', { name: /reverse turn/i }));

    expect(webClient.request.game.reverseTurn).toHaveBeenCalledWith(1);
  });

  it('dispatches setActivePhase(current+1 mod 11) on Next Phase', () => {
    const webClient = createMockWebClient();
    renderWithProviders(<TurnControls {...DEFAULT_TURN_PROPS} />, {
      preloadedState: stateWith(),
      webClient,
    });

    fireEvent.click(screen.getByRole('button', { name: /next phase/i }));

    // activePhase defaults to 0 in fixtures → Next Phase goes to 1.
    expect(webClient.request.game.setActivePhase).toHaveBeenCalledWith(1, { phase: 1 });
  });

  it('disables Pass/Reverse/NextPhase when local player is not active and is not a judge', () => {
    renderWithProviders(<TurnControls {...DEFAULT_TURN_PROPS} />, {
      preloadedState: stateWith({ localPlayerId: 1, activePlayerId: 2 }),
    });

    expect(screen.getByRole('button', { name: /pass turn/i })).toBeDisabled();
    expect(screen.getByRole('button', { name: /reverse turn/i })).toBeDisabled();
    expect(screen.getByRole('button', { name: /next phase/i })).toBeDisabled();
  });

  it('enables Pass/Reverse/NextPhase for a judge even when not the active player (desktop parity)', () => {
    renderWithProviders(<TurnControls {...DEFAULT_TURN_PROPS} />, {
      preloadedState: stateWith({ localPlayerId: 1, activePlayerId: 2, judge: true }),
    });

    expect(screen.getByRole('button', { name: /pass turn/i })).not.toBeDisabled();
    expect(screen.getByRole('button', { name: /next phase/i })).not.toBeDisabled();
  });

  it('routes Concede through the parent confirm handler (no direct dispatch)', () => {
    const webClient = createMockWebClient();
    const onRequestConcede = vi.fn();
    renderWithProviders(
      <TurnControls {...DEFAULT_TURN_PROPS} onRequestConcede={onRequestConcede} />,
      { preloadedState: stateWith(), webClient },
    );

    fireEvent.click(screen.getByRole('button', { name: /^concede$/i }));

    expect(onRequestConcede).toHaveBeenCalled();
    // Direct dispatch only fires from the ConfirmDialog "Confirm" path.
    expect(webClient.request.game.concede).not.toHaveBeenCalled();
  });

  it('routes Unconcede through the parent confirm handler when already conceded', () => {
    const onRequestUnconcede = vi.fn();
    renderWithProviders(
      <TurnControls {...DEFAULT_TURN_PROPS} onRequestUnconcede={onRequestUnconcede} />,
      { preloadedState: stateWith({ conceded: true }) },
    );

    fireEvent.click(screen.getByRole('button', { name: /unconcede/i }));

    expect(onRequestUnconcede).toHaveBeenCalled();
  });

  it('dispatches leaveGame when Leave Game is clicked', () => {
    const webClient = createMockWebClient();
    renderWithProviders(<TurnControls {...DEFAULT_TURN_PROPS} />, {
      preloadedState: stateWith(),
      webClient,
    });

    fireEvent.click(screen.getByRole('button', { name: /leave game/i }));

    expect(webClient.request.game.leaveGame).toHaveBeenCalledWith(1);
  });

  it('fires onRequestRollDie when Roll Die is clicked', () => {
    const onRequestRollDie = vi.fn();
    renderWithProviders(
      <TurnControls {...DEFAULT_TURN_PROPS} onRequestRollDie={onRequestRollDie} />,
      { preloadedState: stateWith() },
    );

    fireEvent.click(screen.getByRole('button', { name: /roll die/i }));

    expect(onRequestRollDie).toHaveBeenCalled();
  });

  it('fires onRequestGameInfo when Game Info is clicked', () => {
    const onRequestGameInfo = vi.fn();
    renderWithProviders(
      <TurnControls {...DEFAULT_TURN_PROPS} onRequestGameInfo={onRequestGameInfo} />,
      { preloadedState: stateWith() },
    );

    fireEvent.click(screen.getByRole('button', { name: /game info/i }));

    expect(onRequestGameInfo).toHaveBeenCalled();
  });

  it('fires onToggleRotate90 when Rotate 90° is clicked and flips label when already rotated', () => {
    const onToggleRotate90 = vi.fn();
    const { rerender } = renderWithProviders(
      <TurnControls {...DEFAULT_TURN_PROPS} onToggleRotate90={onToggleRotate90} />,
      { preloadedState: stateWith() },
    );

    fireEvent.click(screen.getByRole('button', { name: /rotate 90/i }));
    expect(onToggleRotate90).toHaveBeenCalled();

    rerender(<TurnControls {...DEFAULT_TURN_PROPS} onToggleRotate90={onToggleRotate90} isRotated />);
    expect(screen.getByRole('button', { name: /unrotate view/i })).toBeInTheDocument();
  });

  it('disables Remove Arrows when the local player has no arrows', () => {
    renderWithProviders(<TurnControls {...DEFAULT_TURN_PROPS} />, {
      preloadedState: stateWith(),
    });

    expect(screen.getByRole('button', { name: /remove arrows/i })).toBeDisabled();
  });

  it('hides the Kick button for non-hosts', () => {
    renderWithProviders(<TurnControls {...DEFAULT_TURN_PROPS} />, {
      preloadedState: stateWith({ localPlayerId: 1, hostId: 2, opponentIds: [2] }),
    });

    expect(screen.queryByRole('button', { name: /kick/i })).not.toBeInTheDocument();
  });

  it('shows Kick for hosts and opens an opponent picker', () => {
    renderWithProviders(<TurnControls {...DEFAULT_TURN_PROPS} />, {
      preloadedState: stateWith({ localPlayerId: 1, hostId: 1, opponentIds: [2, 3] }),
    });

    fireEvent.click(screen.getByRole('button', { name: /kick/i }));

    expect(screen.getByText('P2')).toBeInTheDocument();
    expect(screen.getByText('P3')).toBeInTheDocument();
  });

  it('dispatches kickFromGame with the chosen opponent', () => {
    const webClient = createMockWebClient();
    renderWithProviders(<TurnControls {...DEFAULT_TURN_PROPS} />, {
      preloadedState: stateWith({ localPlayerId: 1, hostId: 1, opponentIds: [2, 3] }),
      webClient,
    });

    fireEvent.click(screen.getByRole('button', { name: /kick/i }));
    fireEvent.click(screen.getByText('P3'));

    expect(webClient.request.game.kickFromGame).toHaveBeenCalledWith(1, { playerId: 3 });
  });

  describe('spectator gating', () => {
    it('disables Concede/Unconcede/RollDie for pure spectators (desktop parity)', () => {
      renderWithProviders(<TurnControls {...DEFAULT_TURN_PROPS} />, {
        preloadedState: stateWith({ spectator: true }),
      });

      expect(screen.getByRole('button', { name: /^concede$/i })).toBeDisabled();
      expect(screen.getByRole('button', { name: /roll die/i })).toBeDisabled();
    });

    it('keeps Leave Game enabled for spectators (they may stop spectating)', () => {
      renderWithProviders(<TurnControls {...DEFAULT_TURN_PROPS} />, {
        preloadedState: stateWith({ spectator: true }),
      });

      expect(screen.getByRole('button', { name: /leave game/i })).not.toBeDisabled();
    });

    it('lets judges roll dice even though they are flagged as spectators (desktop parity)', () => {
      renderWithProviders(<TurnControls {...DEFAULT_TURN_PROPS} />, {
        preloadedState: stateWith({ spectator: true, judge: true }),
      });

      expect(screen.getByRole('button', { name: /roll die/i })).not.toBeDisabled();
    });

    // Desktop: judges can't concede — they have no local player to concede as.
    // Our `canConcede = !isSpectator && …` gate already excludes judges who
    // are flagged spectator; this test pins the behavior.
    it('disables Concede for judges flagged as spectators (no local player to concede)', () => {
      renderWithProviders(<TurnControls {...DEFAULT_TURN_PROPS} />, {
        preloadedState: stateWith({ spectator: true, judge: true, conceded: false }),
      });

      expect(screen.getByRole('button', { name: /^concede$/i })).toBeDisabled();
    });

    it('disables Unconcede for spectators who are already conceded', () => {
      renderWithProviders(<TurnControls {...DEFAULT_TURN_PROPS} />, {
        preloadedState: stateWith({ spectator: true, conceded: true }),
      });

      // Button renders as "Unconcede" when already conceded; stays disabled
      // because spectators have no concede state in the first place.
      expect(screen.getByRole('button', { name: /unconcede/i })).toBeDisabled();
    });

    it('disables Reverse Turn for pure spectators (they are never the active player)', () => {
      renderWithProviders(<TurnControls {...DEFAULT_TURN_PROPS} />, {
        // Spectator is typically not the active player; canAdvance gates on
        // (isJudge || activePlayerId === localPlayerId) so Reverse is off.
        preloadedState: stateWith({ spectator: true, localPlayerId: 1, activePlayerId: 2 }),
      });

      expect(screen.getByRole('button', { name: /reverse turn/i })).toBeDisabled();
    });
  });

  describe('Invert Rows toggle', () => {
    it('calls updateSettings with invertVerticalCoordinate=true when off', () => {
      const update = vi.fn().mockResolvedValue(undefined);
      vi.mocked(useSettings).mockReturnValue(
        makeSettingsHook({
          status: LoadingState.READY,
          value: makeSettings({ invertVerticalCoordinate: false }),
          update,
        }),
      );

      renderWithProviders(<TurnControls {...DEFAULT_TURN_PROPS} />, {
        preloadedState: stateWith(),
      });

      fireEvent.click(screen.getByRole('button', { name: /invert rows/i }));

      expect(update).toHaveBeenCalledWith({ invertVerticalCoordinate: true });
    });

    it('calls updateSettings with invertVerticalCoordinate=false when on', () => {
      const update = vi.fn().mockResolvedValue(undefined);
      vi.mocked(useSettings).mockReturnValue(
        makeSettingsHook({
          status: LoadingState.READY,
          value: makeSettings({ invertVerticalCoordinate: true }),
          update,
        }),
      );

      renderWithProviders(<TurnControls {...DEFAULT_TURN_PROPS} />, {
        preloadedState: stateWith(),
      });

      fireEvent.click(screen.getByRole('button', { name: /invert rows/i }));

      expect(update).toHaveBeenCalledWith({ invertVerticalCoordinate: false });
    });

    it('reflects the current value via aria-pressed', () => {
      vi.mocked(useSettings).mockReturnValue(
        makeSettingsHook({
          status: LoadingState.READY,
          value: makeSettings({ invertVerticalCoordinate: true }),
        }),
      );

      renderWithProviders(<TurnControls {...DEFAULT_TURN_PROPS} />, {
        preloadedState: stateWith(),
      });

      expect(screen.getByRole('button', { name: /invert rows/i })).toHaveAttribute(
        'aria-pressed',
        'true',
      );
    });

    it('is disabled while settings are still loading', () => {
      vi.mocked(useSettings).mockReturnValue(
        makeSettingsHook({ status: LoadingState.LOADING, value: undefined }),
      );

      renderWithProviders(<TurnControls {...DEFAULT_TURN_PROPS} />, {
        preloadedState: stateWith(),
      });

      expect(screen.getByRole('button', { name: /invert rows/i })).toBeDisabled();
    });
  });
});
