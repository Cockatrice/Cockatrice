import { screen, fireEvent } from '@testing-library/react';

import { createMockWebClient, makeStoreState, renderWithProviders } from '../../__test-utils__';
import {
  makeGameEntry,
  makePlayerEntry,
  makePlayerProperties,
} from '../../store/game/__mocks__/fixtures';
import DeckSelectDialog from './DeckSelectDialog';

function stateWith(playerProps: Parameters<typeof makePlayerProperties>[0] = {}) {
  return makeStoreState({
    games: {
      games: {
        1: makeGameEntry({
          localPlayerId: 1,
          players: {
            1: makePlayerEntry({
              properties: makePlayerProperties({ playerId: 1, ...playerProps }),
            }),
          },
        }),
      },
    },
  });
}

describe('DeckSelectDialog', () => {
  it('does not render content when closed', () => {
    renderWithProviders(
      <DeckSelectDialog isOpen={false} gameId={1} />,
      { preloadedState: stateWith() },
    );

    expect(screen.queryByRole('dialog')).not.toBeInTheDocument();
  });

  it('renders textarea, Submit Deck, and Ready controls when open', () => {
    renderWithProviders(
      <DeckSelectDialog isOpen gameId={1} />,
      { preloadedState: stateWith() },
    );

    expect(screen.getByLabelText('deck list')).toBeInTheDocument();
    expect(screen.getByRole('button', { name: /submit deck/i })).toBeInTheDocument();
    expect(screen.getByRole('button', { name: /^ready$/i })).toBeInTheDocument();
  });

  it('disables Submit Deck until the textarea has non-whitespace content', () => {
    renderWithProviders(
      <DeckSelectDialog isOpen gameId={1} />,
      { preloadedState: stateWith() },
    );

    const submit = screen.getByRole('button', { name: /submit deck/i });
    expect(submit).toBeDisabled();

    fireEvent.change(screen.getByLabelText('deck list'), {
      target: { value: '4 Lightning Bolt\n' },
    });
    expect(submit).not.toBeDisabled();
  });

  it('dispatches deckSelect with the textarea content when Submit Deck is clicked', () => {
    const webClient = createMockWebClient();
    renderWithProviders(
      <DeckSelectDialog isOpen gameId={1} />,
      { preloadedState: stateWith(), webClient },
    );

    fireEvent.change(screen.getByLabelText('deck list'), {
      target: { value: '4 Island\n4 Mountain' },
    });
    fireEvent.click(screen.getByRole('button', { name: /submit deck/i }));

    expect(webClient.request.game.deckSelect).toHaveBeenCalledWith(1, {
      deck: '4 Island\n4 Mountain',
    });
  });

  it('keeps Ready disabled until the player has a deckHash', () => {
    const { rerender } = renderWithProviders(
      <DeckSelectDialog isOpen gameId={1} />,
      { preloadedState: stateWith({ deckHash: '' }) },
    );

    expect(screen.getByRole('button', { name: /^ready$/i })).toBeDisabled();

    rerender(<DeckSelectDialog isOpen gameId={1} />);
  });

  it('enables Ready once deckHash is populated and shows the hash text', () => {
    renderWithProviders(
      <DeckSelectDialog isOpen gameId={1} />,
      { preloadedState: stateWith({ deckHash: 'abc123' }) },
    );

    expect(screen.getByRole('button', { name: /^ready$/i })).not.toBeDisabled();
    expect(screen.getByText(/abc123/)).toBeInTheDocument();
  });

  it('dispatches readyStart when Ready is clicked', () => {
    const webClient = createMockWebClient();
    renderWithProviders(
      <DeckSelectDialog isOpen gameId={1} />,
      { preloadedState: stateWith({ deckHash: 'abc123' }), webClient },
    );

    fireEvent.click(screen.getByRole('button', { name: /^ready$/i }));

    expect(webClient.request.game.readyStart).toHaveBeenCalledWith(1, { ready: true });
  });

  it('switches the label to "Unready" and stays enabled when the player is already ready', () => {
    renderWithProviders(
      <DeckSelectDialog isOpen gameId={1} />,
      { preloadedState: stateWith({ deckHash: 'abc123', readyStart: true }) },
    );

    const ready = screen.getByRole('button', { name: /unready/i });
    expect(ready).toHaveTextContent('Unready');
    expect(ready).not.toBeDisabled();
  });

  it('dispatches readyStart({ready:false}) when Unready is clicked (un-ready toggle)', () => {
    const webClient = createMockWebClient();
    renderWithProviders(
      <DeckSelectDialog isOpen gameId={1} />,
      {
        preloadedState: stateWith({ deckHash: 'abc123', readyStart: true }),
        webClient,
      },
    );

    fireEvent.click(screen.getByRole('button', { name: /unready/i }));

    expect(webClient.request.game.readyStart).toHaveBeenCalledWith(1, { ready: false });
  });
});
