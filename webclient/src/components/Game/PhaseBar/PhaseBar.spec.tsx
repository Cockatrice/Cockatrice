import { screen, fireEvent } from '@testing-library/react';
import { App, Data } from '@app/types';

import { createMockWebClient, makeStoreState, renderWithProviders } from '../../../__test-utils__';
import {
  makeCard,
  makeGameEntry,
  makePlayerEntry,
  makePlayerProperties,
  makeZoneEntry,
} from '../../../store/game/__mocks__/fixtures';
import PhaseBar from './PhaseBar';

function stateWith(opts: {
  phase?: number;
  localPlayerId?: number;
  activePlayerId?: number;
  started?: boolean;
  judge?: boolean;
} = {}) {
  const localId = opts.localPlayerId ?? 1;
  return makeStoreState({
    games: {
      games: {
        1: makeGameEntry({
          activePhase: opts.phase ?? 0,
          localPlayerId: localId,
          activePlayerId: opts.activePlayerId ?? localId,
          started: opts.started ?? true,
          judge: opts.judge ?? false,
          players: {
            [localId]: makePlayerEntry({
              properties: makePlayerProperties({ playerId: localId }),
            }),
          },
        }),
      },
    },
  });
}

describe('PhaseBar', () => {
  it('renders 11 phase buttons plus PASS', () => {
    renderWithProviders(<PhaseBar gameId={1} />, {
      preloadedState: stateWith(),
    });

    const buttons = screen.getByTestId('phase-bar').querySelectorAll('button');
    expect(buttons).toHaveLength(12);
    expect(buttons[11].textContent).toBe('PASS TURN');
  });

  it('renders phases in desktop-Cockatrice order', () => {
    renderWithProviders(<PhaseBar gameId={1} />, {
      preloadedState: stateWith(),
    });

    const labels = Array.from(
      screen.getByTestId('phase-bar').querySelectorAll('button'),
    ).map((b) => b.textContent);
    expect(labels.slice(0, 11)).toEqual([
      'UNTAP', 'UPKP', 'DRAW', 'M1', 'CMBT', 'ATTK',
      'BLCK', 'DMGE', 'ECMB', 'M2', 'END',
    ]);
  });

  it('applies the active modifier only to the button matching activePhase', () => {
    renderWithProviders(<PhaseBar gameId={1} />, {
      preloadedState: stateWith({ phase: App.Phase.DeclareAttackers }),
    });

    const active = document.querySelector('.phase-bar__btn--active')!;
    expect(active.getAttribute('data-phase')).toBe(String(App.Phase.DeclareAttackers));
    expect(document.querySelectorAll('.phase-bar__btn--active')).toHaveLength(1);
  });

  it('renders no active button when gameId is undefined', () => {
    renderWithProviders(<PhaseBar gameId={undefined} />, {
      preloadedState: makeStoreState({}),
    });
    expect(document.querySelectorAll('.phase-bar__btn--active')).toHaveLength(0);
  });

  it('enables buttons when the local player is the active player and the game has started', () => {
    renderWithProviders(<PhaseBar gameId={1} />, {
      preloadedState: stateWith({ started: true }),
    });

    const buttons = screen.getByTestId('phase-bar').querySelectorAll('button');
    buttons.forEach((b) => expect(b).not.toBeDisabled());
  });

  it('disables every button when the game has not started', () => {
    renderWithProviders(<PhaseBar gameId={1} />, {
      preloadedState: stateWith({ started: false }),
    });

    const buttons = screen.getByTestId('phase-bar').querySelectorAll('button');
    buttons.forEach((b) => expect(b).toBeDisabled());
  });

  it('disables every button when the local player is not the active player (non-judge)', () => {
    renderWithProviders(<PhaseBar gameId={1} />, {
      preloadedState: stateWith({
        localPlayerId: 1,
        activePlayerId: 2,
      }),
    });

    const buttons = screen.getByTestId('phase-bar').querySelectorAll('button');
    buttons.forEach((b) => expect(b).toBeDisabled());
  });

  it('enables buttons for a judge regardless of active player (matches desktop)', () => {
    renderWithProviders(<PhaseBar gameId={1} />, {
      preloadedState: stateWith({
        localPlayerId: 1,
        activePlayerId: 2,
        judge: true,
      }),
    });

    const buttons = screen.getByTestId('phase-bar').querySelectorAll('button');
    buttons.forEach((b) => expect(b).not.toBeDisabled());
  });

  it('dispatches setActivePhase when a phase button is clicked', () => {
    const webClient = createMockWebClient();
    renderWithProviders(<PhaseBar gameId={1} />, {
      preloadedState: stateWith(),
      webClient,
    });

    fireEvent.click(screen.getByText('ATTK'));

    expect(webClient.request.game.setActivePhase).toHaveBeenCalledWith(1, {
      phase: App.Phase.DeclareAttackers,
    });
  });

  it('dispatches nextTurn when PASS is clicked', () => {
    const webClient = createMockWebClient();
    renderWithProviders(<PhaseBar gameId={1} />, {
      preloadedState: stateWith(),
      webClient,
    });

    fireEvent.click(screen.getByText('PASS TURN'));

    expect(webClient.request.game.nextTurn).toHaveBeenCalledWith(1);
  });

  describe('desktop double-click built-ins (phases_toolbar.cpp)', () => {
    function stateWithTapped(cards: ReturnType<typeof makeCard>[]) {
      return makeStoreState({
        games: {
          games: {
            1: makeGameEntry({
              activePhase: App.Phase.Untap,
              localPlayerId: 1,
              activePlayerId: 1,
              started: true,
              players: {
                1: makePlayerEntry({
                  properties: makePlayerProperties({ playerId: 1 }),
                  zones: {
                    table: makeZoneEntry({ name: 'table', cards, cardCount: cards.length }),
                  },
                }),
              },
            }),
          },
        },
      });
    }

    it('double-click on UNTAP dispatches setCardAttr AttrTapped=0 for every tapped card', () => {
      const webClient = createMockWebClient();
      renderWithProviders(<PhaseBar gameId={1} />, {
        preloadedState: stateWithTapped([
          makeCard({ id: 1, tapped: true }),
          makeCard({ id: 2, tapped: false }),
          makeCard({ id: 3, tapped: true }),
        ]),
        webClient,
      });

      fireEvent.doubleClick(screen.getByText('UNTAP'));

      expect(webClient.request.game.setCardAttr).toHaveBeenCalledTimes(2);
      expect(webClient.request.game.setCardAttr).toHaveBeenCalledWith(1, {
        zone: 'table',
        cardId: 1,
        attribute: Data.CardAttribute.AttrTapped,
        attrValue: '0',
      });
      expect(webClient.request.game.setCardAttr).toHaveBeenCalledWith(1, {
        zone: 'table',
        cardId: 3,
        attribute: Data.CardAttribute.AttrTapped,
        attrValue: '0',
      });
    });

    it('double-click on UNTAP is a no-op when no cards are tapped', () => {
      const webClient = createMockWebClient();
      renderWithProviders(<PhaseBar gameId={1} />, {
        preloadedState: stateWithTapped([makeCard({ id: 1, tapped: false })]),
        webClient,
      });

      fireEvent.doubleClick(screen.getByText('UNTAP'));

      expect(webClient.request.game.setCardAttr).not.toHaveBeenCalled();
    });

    it('double-click on DRAW dispatches drawCards({ number: 1 })', () => {
      const webClient = createMockWebClient();
      renderWithProviders(<PhaseBar gameId={1} />, {
        preloadedState: stateWith(),
        webClient,
      });

      fireEvent.doubleClick(screen.getByText('DRAW'));

      expect(webClient.request.game.drawCards).toHaveBeenCalledWith(1, { number: 1 });
    });

    it('double-click does nothing when the local player is not active', () => {
      const webClient = createMockWebClient();
      renderWithProviders(<PhaseBar gameId={1} />, {
        preloadedState: stateWith({ localPlayerId: 1, activePlayerId: 2 }),
        webClient,
      });

      fireEvent.doubleClick(screen.getByText('UNTAP'));
      fireEvent.doubleClick(screen.getByText('DRAW'));

      expect(webClient.request.game.setCardAttr).not.toHaveBeenCalled();
      expect(webClient.request.game.drawCards).not.toHaveBeenCalled();
    });

    it('double-click on other phases (UPKP, M1, etc.) does not fire any built-in', () => {
      const webClient = createMockWebClient();
      renderWithProviders(<PhaseBar gameId={1} />, {
        preloadedState: stateWith(),
        webClient,
      });

      fireEvent.doubleClick(screen.getByText('UPKP'));
      fireEvent.doubleClick(screen.getByText('M1'));
      fireEvent.doubleClick(screen.getByText('END'));

      expect(webClient.request.game.setCardAttr).not.toHaveBeenCalled();
      expect(webClient.request.game.drawCards).not.toHaveBeenCalled();
    });
  });
});
