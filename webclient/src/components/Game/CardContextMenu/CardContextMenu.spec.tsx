import { screen, fireEvent } from '@testing-library/react';
import { App, Data } from '@app/types';

import { createMockWebClient, renderWithProviders } from '../../../__test-utils__';
import { makeCard } from '../../../store/game/__mocks__/fixtures';
import CardContextMenu from './CardContextMenu';

const defaultProps = {
  isOpen: true,
  anchorPosition: { top: 100, left: 100 },
  gameId: 1,
  localPlayerId: 1,
  ownerPlayerId: 1,
  sourceZone: App.ZoneName.TABLE,
  onClose: () => {},
  onRequestSetPT: () => {},
  onRequestSetAnnotation: () => {},
  onRequestSetCounter: () => {},
  onRequestDrawArrow: () => {},
  onRequestAttach: () => {},
  onRequestMoveToLibraryAt: () => {},
};

describe('CardContextMenu', () => {
  it('does not render when card is null', () => {
    const { container } = renderWithProviders(
      <CardContextMenu {...defaultProps} card={null} />,
    );
    expect(container.querySelector('[data-testid="card-context-menu"]')).toBeNull();
  });

  it('does not render when closed', () => {
    renderWithProviders(
      <CardContextMenu {...defaultProps} isOpen={false} card={makeCard()} />,
    );
    expect(screen.queryByRole('menu')).not.toBeInTheDocument();
  });

  it('renders all expected menu items', () => {
    renderWithProviders(
      <CardContextMenu {...defaultProps} card={makeCard({ tapped: false, faceDown: false })} />,
    );

    expect(screen.getByText('Flip')).toBeInTheDocument();
    expect(screen.getByText('Tap')).toBeInTheDocument();
    expect(screen.getByText('Face Down')).toBeInTheDocument();
    expect(screen.getByText('Doesn\'t Untap')).toBeInTheDocument();
    expect(screen.getByText('Set P/T…')).toBeInTheDocument();
    expect(screen.getByText('Set Annotation…')).toBeInTheDocument();
    expect(screen.getByText('Send to Hand')).toBeInTheDocument();
    expect(screen.getByText('Send to Graveyard')).toBeInTheDocument();
    expect(screen.getByText('Send to Exile')).toBeInTheDocument();
    expect(screen.getByText('Send to Library (top)')).toBeInTheDocument();
    expect(screen.getByText('Send to Library (bottom)')).toBeInTheDocument();
  });

  it('flips the card via flipCard and closes the menu', () => {
    const webClient = createMockWebClient();
    const onClose = vi.fn();
    const card = makeCard({ id: 10, faceDown: false });

    renderWithProviders(
      <CardContextMenu {...defaultProps} card={card} onClose={onClose} />,
      { webClient },
    );

    fireEvent.click(screen.getByText('Flip'));

    expect(webClient.request.game.flipCard).toHaveBeenCalledWith(1, {
      zone: App.ZoneName.TABLE,
      cardId: 10,
      faceDown: true,
    });
    expect(onClose).toHaveBeenCalled();
  });

  it('toggles tap via setCardAttr (untapped → tapped)', () => {
    const webClient = createMockWebClient();
    renderWithProviders(
      <CardContextMenu {...defaultProps} card={makeCard({ id: 5, tapped: false })} />,
      { webClient },
    );

    fireEvent.click(screen.getByText('Tap'));

    expect(webClient.request.game.setCardAttr).toHaveBeenCalledWith(1, {
      zone: App.ZoneName.TABLE,
      cardId: 5,
      attribute: Data.CardAttribute.AttrTapped,
      attrValue: '1',
    });
  });

  it('shows Untap label and sends "0" when the card is already tapped', () => {
    const webClient = createMockWebClient();
    renderWithProviders(
      <CardContextMenu {...defaultProps} card={makeCard({ id: 5, tapped: true })} />,
      { webClient },
    );

    expect(screen.getByText('Untap')).toBeInTheDocument();
    fireEvent.click(screen.getByText('Untap'));

    expect(webClient.request.game.setCardAttr).toHaveBeenCalledWith(1, {
      zone: App.ZoneName.TABLE,
      cardId: 5,
      attribute: Data.CardAttribute.AttrTapped,
      attrValue: '0',
    });
  });

  it('toggles Face Down and shows Face Up when already face-down', () => {
    const webClient = createMockWebClient();
    renderWithProviders(
      <CardContextMenu {...defaultProps} card={makeCard({ id: 5, faceDown: true })} />,
      { webClient },
    );

    expect(screen.getByText('Face Up')).toBeInTheDocument();
    fireEvent.click(screen.getByText('Face Up'));

    expect(webClient.request.game.setCardAttr).toHaveBeenCalledWith(1, {
      zone: App.ZoneName.TABLE,
      cardId: 5,
      attribute: Data.CardAttribute.AttrFaceDown,
      attrValue: '0',
    });
  });

  it('toggles Doesn\'t Untap and shows Allow Untap when already set', () => {
    const webClient = createMockWebClient();
    renderWithProviders(
      <CardContextMenu {...defaultProps} card={makeCard({ id: 5, doesntUntap: true })} />,
      { webClient },
    );

    expect(screen.getByText('Allow Untap')).toBeInTheDocument();
    fireEvent.click(screen.getByText('Allow Untap'));

    expect(webClient.request.game.setCardAttr).toHaveBeenCalledWith(1, {
      zone: App.ZoneName.TABLE,
      cardId: 5,
      attribute: Data.CardAttribute.AttrDoesntUntap,
      attrValue: '0',
    });
  });

  it('requests the PT prompt via parent callback', () => {
    const onRequestSetPT = vi.fn();
    const onClose = vi.fn();
    renderWithProviders(
      <CardContextMenu
        {...defaultProps}
        card={makeCard()}
        onRequestSetPT={onRequestSetPT}
        onClose={onClose}
      />,
    );

    fireEvent.click(screen.getByText('Set P/T…'));

    expect(onRequestSetPT).toHaveBeenCalled();
    expect(onClose).toHaveBeenCalled();
  });

  it('requests the Annotation prompt via parent callback', () => {
    const onRequestSetAnnotation = vi.fn();
    renderWithProviders(
      <CardContextMenu
        {...defaultProps}
        card={makeCard()}
        onRequestSetAnnotation={onRequestSetAnnotation}
      />,
    );

    fireEvent.click(screen.getByText('Set Annotation…'));

    expect(onRequestSetAnnotation).toHaveBeenCalled();
  });

  it('moves to hand via moveCard with x=-1 (append)', () => {
    const webClient = createMockWebClient();
    renderWithProviders(
      <CardContextMenu {...defaultProps} card={makeCard({ id: 7 })} />,
      { webClient },
    );

    fireEvent.click(screen.getByText('Send to Hand'));

    expect(webClient.request.game.moveCard).toHaveBeenCalledWith(1, {
      startPlayerId: 1,
      startZone: App.ZoneName.TABLE,
      cardsToMove: { card: [{ cardId: 7 }] },
      targetPlayerId: 1,
      targetZone: App.ZoneName.HAND,
      x: -1,
      y: 0,
      isReversed: false,
    });
  });

  it('hides mutator items (tap, flip, move, counters, P/T) for opponent-owned cards (desktop parity)', () => {
    renderWithProviders(
      <CardContextMenu
        {...defaultProps}
        localPlayerId={1}
        ownerPlayerId={2}
        card={makeCard({ id: 7 })}
      />,
    );

    // Mutators gone:
    expect(screen.queryByText('Flip')).not.toBeInTheDocument();
    expect(screen.queryByText('Tap')).not.toBeInTheDocument();
    expect(screen.queryByText('Set P/T…')).not.toBeInTheDocument();
    expect(screen.queryByText('Set counter…')).not.toBeInTheDocument();
    expect(screen.queryByText('Send to Hand')).not.toBeInTheDocument();
    expect(screen.queryByText('Attach to card…')).not.toBeInTheDocument();

    // Read-only stays:
    expect(screen.getByText('Draw arrow from here')).toBeInTheDocument();
  });

  it('routes moves through the acting (local) player when invoked on an owned card', () => {
    const webClient = createMockWebClient();
    renderWithProviders(
      <CardContextMenu
        {...defaultProps}
        localPlayerId={1}
        ownerPlayerId={1}
        card={makeCard({ id: 7 })}
      />,
      { webClient },
    );

    fireEvent.click(screen.getByText('Send to Hand'));

    expect(webClient.request.game.moveCard).toHaveBeenCalledWith(1, expect.objectContaining({
      startPlayerId: 1,
      targetPlayerId: 1,
    }));
  });

  it('moves to library top vs bottom with distinct x values', () => {
    const webClient = createMockWebClient();
    renderWithProviders(
      <CardContextMenu {...defaultProps} card={makeCard({ id: 7 })} />,
      { webClient },
    );

    fireEvent.click(screen.getByText('Send to Library (top)'));
    expect(webClient.request.game.moveCard).toHaveBeenLastCalledWith(1, expect.objectContaining({
      targetZone: App.ZoneName.DECK,
      x: 0,
    }));

    fireEvent.click(screen.getByText('Send to Library (bottom)'));
    expect(webClient.request.game.moveCard).toHaveBeenLastCalledWith(1, expect.objectContaining({
      targetZone: App.ZoneName.DECK,
      x: -1,
    }));
  });

  it('adds a counter via incCardCounter (+1 on id 0)', () => {
    const webClient = createMockWebClient();
    renderWithProviders(
      <CardContextMenu {...defaultProps} card={makeCard({ id: 9 })} />,
      { webClient },
    );

    fireEvent.click(screen.getByText('Add counter'));

    expect(webClient.request.game.incCardCounter).toHaveBeenCalledWith(1, {
      zone: App.ZoneName.TABLE,
      cardId: 9,
      counterId: 0,
      counterDelta: 1,
    });
  });

  it('removes a counter via incCardCounter (-1 on id 0)', () => {
    const webClient = createMockWebClient();
    renderWithProviders(
      <CardContextMenu {...defaultProps} card={makeCard({ id: 9 })} />,
      { webClient },
    );

    fireEvent.click(screen.getByText('Remove counter'));

    expect(webClient.request.game.incCardCounter).toHaveBeenCalledWith(1, {
      zone: App.ZoneName.TABLE,
      cardId: 9,
      counterId: 0,
      counterDelta: -1,
    });
  });

  it('defers "Set counter…" to the parent callback', () => {
    const onRequestSetCounter = vi.fn();
    renderWithProviders(
      <CardContextMenu
        {...defaultProps}
        card={makeCard()}
        onRequestSetCounter={onRequestSetCounter}
      />,
    );

    fireEvent.click(screen.getByText('Set counter…'));

    expect(onRequestSetCounter).toHaveBeenCalled();
  });

  it('defers "Draw arrow from here" to the parent callback', () => {
    const onRequestDrawArrow = vi.fn();
    renderWithProviders(
      <CardContextMenu
        {...defaultProps}
        card={makeCard()}
        onRequestDrawArrow={onRequestDrawArrow}
      />,
    );

    fireEvent.click(screen.getByText('Draw arrow from here'));

    expect(onRequestDrawArrow).toHaveBeenCalled();
  });

  describe('Attach / Unattach', () => {
    it('defers "Attach to card…" to the parent callback', () => {
      const onRequestAttach = vi.fn();
      const onClose = vi.fn();
      renderWithProviders(
        <CardContextMenu
          {...defaultProps}
          card={makeCard()}
          onRequestAttach={onRequestAttach}
          onClose={onClose}
        />,
      );

      fireEvent.click(screen.getByText('Attach to card…'));

      expect(onRequestAttach).toHaveBeenCalled();
      expect(onClose).toHaveBeenCalled();
    });

    it('does not show "Unattach" when the card is not attached (attachCardId = -1)', () => {
      renderWithProviders(
        <CardContextMenu
          {...defaultProps}
          card={makeCard({ attachCardId: -1 })}
        />,
      );

      expect(screen.queryByText('Unattach')).not.toBeInTheDocument();
    });

    it('shows "Unattach" and dispatches attachCard with only startZone+cardId (desktop parity)', () => {
      const webClient = createMockWebClient();
      const onClose = vi.fn();
      renderWithProviders(
        <CardContextMenu
          {...defaultProps}
          card={makeCard({ id: 11, attachCardId: 99 })}
          onClose={onClose}
        />,
        { webClient },
      );

      fireEvent.click(screen.getByText('Unattach'));

      // Target fields are intentionally absent. The server uses proto2
      // presence (`has_target_player_id()`) to detect "detach"; passing
      // targetPlayerId: -1 would leave presence set and the server would
      // treat the message as an attach with a missing player.
      expect(webClient.request.game.attachCard).toHaveBeenCalledWith(1, {
        startZone: App.ZoneName.TABLE,
        cardId: 11,
      });
      expect(onClose).toHaveBeenCalled();
    });

    it('hides Attach / Unattach when the source card is not on the table', () => {
      renderWithProviders(
        <CardContextMenu
          {...defaultProps}
          sourceZone={App.ZoneName.HAND}
          card={makeCard({ attachCardId: 99 })}
        />,
      );

      expect(screen.queryByText('Attach to card…')).not.toBeInTheDocument();
      expect(screen.queryByText('Unattach')).not.toBeInTheDocument();
    });
  });
});
