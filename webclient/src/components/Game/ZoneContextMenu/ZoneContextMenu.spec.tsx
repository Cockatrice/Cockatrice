import { screen, fireEvent } from '@testing-library/react';
import { App } from '@app/types';

import { createMockWebClient, makeStoreState, renderWithProviders } from '../../../__test-utils__';
import {
  makeGameEntry,
  makePlayerEntry,
  makeZoneEntry,
} from '../../../store/game/__mocks__/fixtures';
import ZoneContextMenu from './ZoneContextMenu';

const defaultProps = {
  isOpen: true,
  anchorPosition: { top: 100, left: 100 },
  gameId: 1,
  playerId: 1,
  zoneName: App.ZoneName.DECK,
  onClose: () => {},
  onRequestDrawN: () => {},
  onRequestDumpN: () => {},
  onRequestRevealTopN: () => {},
  onRequestRevealZone: () => {},
};

function stateWithDeckZone(overrides: Partial<ReturnType<typeof makeZoneEntry>> = {}) {
  const player = makePlayerEntry({
    zones: {
      deck: makeZoneEntry({ name: App.ZoneName.DECK, ...overrides }),
      grave: makeZoneEntry({ name: App.ZoneName.GRAVE }),
      rfg: makeZoneEntry({ name: App.ZoneName.EXILE }),
    },
  });
  return makeStoreState({
    games: {
      games: {
        1: makeGameEntry({ players: { 1: player } }),
      },
    },
  });
}

describe('ZoneContextMenu', () => {
  it('does not render when playerId is null', () => {
    renderWithProviders(
      <ZoneContextMenu {...defaultProps} playerId={null} />,
    );
    expect(screen.queryByRole('menu')).not.toBeInTheDocument();
  });

  it('does not render for unsupported zones (e.g. hand, stack)', () => {
    renderWithProviders(
      <ZoneContextMenu {...defaultProps} zoneName={App.ZoneName.HAND} />,
      { preloadedState: stateWithDeckZone() },
    );
    expect(screen.queryByRole('menu')).not.toBeInTheDocument();
  });

  describe('Deck zone', () => {
    it('renders every deck action when open', () => {
      renderWithProviders(<ZoneContextMenu {...defaultProps} />, {
        preloadedState: stateWithDeckZone(),
      });

      expect(screen.getByText('Draw a card')).toBeInTheDocument();
      expect(screen.getByText('Draw N cards…')).toBeInTheDocument();
      expect(screen.getByText('Shuffle')).toBeInTheDocument();
      expect(screen.getByText('Dump top N…')).toBeInTheDocument();
      expect(screen.getByText('Reveal top card to all')).toBeInTheDocument();
      expect(screen.getByText('Reveal top N to…')).toBeInTheDocument();
      expect(screen.getByText('Always reveal top card')).toBeInTheDocument();
      expect(screen.getByText('Always look at top card')).toBeInTheDocument();
    });

    it('dispatches drawCards(1) on "Draw a card"', () => {
      const webClient = createMockWebClient();
      const onClose = vi.fn();
      renderWithProviders(
        <ZoneContextMenu {...defaultProps} onClose={onClose} />,
        { webClient, preloadedState: stateWithDeckZone() },
      );

      fireEvent.click(screen.getByText('Draw a card'));

      expect(webClient.request.game.drawCards).toHaveBeenCalledWith(1, { number: 1 });
      expect(onClose).toHaveBeenCalled();
    });

    it('dispatches shuffle on the deck zone', () => {
      const webClient = createMockWebClient();
      renderWithProviders(<ZoneContextMenu {...defaultProps} />, {
        webClient,
        preloadedState: stateWithDeckZone(),
      });

      fireEvent.click(screen.getByText('Shuffle'));

      expect(webClient.request.game.shuffle).toHaveBeenCalledWith(1, {
        zoneName: App.ZoneName.DECK,
        start: 0,
        end: -1,
      });
    });

    it('dispatches revealCards(topCards=1, playerId=-1) on "Reveal top card to all"', () => {
      const webClient = createMockWebClient();
      renderWithProviders(<ZoneContextMenu {...defaultProps} />, {
        webClient,
        preloadedState: stateWithDeckZone(),
      });

      fireEvent.click(screen.getByText('Reveal top card to all'));

      expect(webClient.request.game.revealCards).toHaveBeenCalledWith(1, {
        zoneName: App.ZoneName.DECK,
        playerId: -1,
        topCards: 1,
      });
    });

    it('defers prompt-backed items to parent callbacks', () => {
      const onRequestDrawN = vi.fn();
      const onRequestDumpN = vi.fn();
      const onRequestRevealTopN = vi.fn();
      renderWithProviders(
        <ZoneContextMenu
          {...defaultProps}
          onRequestDrawN={onRequestDrawN}
          onRequestDumpN={onRequestDumpN}
          onRequestRevealTopN={onRequestRevealTopN}
        />,
        { preloadedState: stateWithDeckZone() },
      );

      fireEvent.click(screen.getByText('Draw N cards…'));
      expect(onRequestDrawN).toHaveBeenCalled();

      fireEvent.click(screen.getByText('Dump top N…'));
      expect(onRequestDumpN).toHaveBeenCalled();

      fireEvent.click(screen.getByText('Reveal top N to…'));
      expect(onRequestRevealTopN).toHaveBeenCalled();
    });

    it('dispatches changeZoneProperties with the flipped alwaysRevealTopCard', () => {
      const webClient = createMockWebClient();
      renderWithProviders(<ZoneContextMenu {...defaultProps} />, {
        webClient,
        preloadedState: stateWithDeckZone({ alwaysRevealTopCard: false }),
      });

      fireEvent.click(screen.getByText('Always reveal top card'));

      expect(webClient.request.game.changeZoneProperties).toHaveBeenCalledWith(1, {
        zoneName: App.ZoneName.DECK,
        alwaysRevealTopCard: true,
      });
    });

    it('dispatches changeZoneProperties with the flipped alwaysLookAtTopCard', () => {
      const webClient = createMockWebClient();
      renderWithProviders(<ZoneContextMenu {...defaultProps} />, {
        webClient,
        preloadedState: stateWithDeckZone({ alwaysLookAtTopCard: true }),
      });

      fireEvent.click(screen.getByText('Always look at top card'));

      expect(webClient.request.game.changeZoneProperties).toHaveBeenCalledWith(1, {
        zoneName: App.ZoneName.DECK,
        alwaysLookAtTopCard: false,
      });
    });
  });

  describe('Graveyard / Exile zones', () => {
    it('offers "Reveal graveyard to…" on the grave zone', () => {
      const onRequestRevealZone = vi.fn();
      renderWithProviders(
        <ZoneContextMenu
          {...defaultProps}
          zoneName={App.ZoneName.GRAVE}
          onRequestRevealZone={onRequestRevealZone}
        />,
        { preloadedState: stateWithDeckZone() },
      );

      fireEvent.click(screen.getByText('Reveal graveyard to…'));

      expect(onRequestRevealZone).toHaveBeenCalled();
    });

    it('offers "Reveal exile to…" on the exile zone', () => {
      const onRequestRevealZone = vi.fn();
      renderWithProviders(
        <ZoneContextMenu
          {...defaultProps}
          zoneName={App.ZoneName.EXILE}
          onRequestRevealZone={onRequestRevealZone}
        />,
        { preloadedState: stateWithDeckZone() },
      );

      fireEvent.click(screen.getByText('Reveal exile to…'));

      expect(onRequestRevealZone).toHaveBeenCalled();
    });
  });
});
