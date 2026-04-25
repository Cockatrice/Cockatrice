import { screen, fireEvent, waitFor } from '@testing-library/react';

import { createMockWebClient, makeStoreState, renderWithProviders } from '../../__test-utils__';
import {
  makeGameEntry,
  makePlayerEntry,
  makePlayerProperties,
} from '../../store/game/__mocks__/fixtures';
import DeckSelectDialog from './DeckSelectDialog';

const VALID_COD_XML =
  '<?xml version="1.0"?><cockatrice_deck version="1"><zone name="main"><card number="4" name="Island"/></zone></cockatrice_deck>';

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

function pickFile(contents: string, name = 'deck.cod') {
  const file = new File([contents], name, { type: 'application/xml' });
  const input = screen.getByLabelText('deck file') as HTMLInputElement;
  fireEvent.change(input, { target: { files: [file] } });
}

describe('DeckSelectDialog', () => {
  it('does not render content when closed', () => {
    renderWithProviders(
      <DeckSelectDialog isOpen={false} gameId={1} />,
      { preloadedState: stateWith() },
    );

    expect(screen.queryByRole('dialog')).not.toBeInTheDocument();
  });

  it('renders textarea, file picker, Submit Deck, and Ready controls when open', () => {
    renderWithProviders(
      <DeckSelectDialog isOpen gameId={1} />,
      { preloadedState: stateWith() },
    );

    expect(screen.getByLabelText('deck list')).toBeInTheDocument();
    expect(screen.getByLabelText('deck file')).toBeInTheDocument();
    expect(screen.getByRole('button', { name: /choose \.cod file/i })).toBeInTheDocument();
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
      target: { value: VALID_COD_XML },
    });
    expect(submit).not.toBeDisabled();
  });

  it('dispatches deckSelect with the pasted XML when Submit Deck is clicked', () => {
    const webClient = createMockWebClient();
    renderWithProviders(
      <DeckSelectDialog isOpen gameId={1} />,
      { preloadedState: stateWith(), webClient },
    );

    fireEvent.change(screen.getByLabelText('deck list'), {
      target: { value: VALID_COD_XML },
    });
    fireEvent.click(screen.getByRole('button', { name: /submit deck/i }));

    expect(webClient.request.game.deckSelect).toHaveBeenCalledWith(1, {
      deck: VALID_COD_XML,
    });
  });

  it('shows a validation error and does not dispatch when the textarea holds non-XML text', () => {
    const webClient = createMockWebClient();
    renderWithProviders(
      <DeckSelectDialog isOpen gameId={1} />,
      { preloadedState: stateWith(), webClient },
    );

    fireEvent.change(screen.getByLabelText('deck list'), {
      target: { value: '4 Lightning Bolt\n20 Mountain' },
    });
    fireEvent.click(screen.getByRole('button', { name: /submit deck/i }));

    expect(screen.getByRole('alert')).toHaveTextContent(/not a valid cockatrice deck/i);
    expect(webClient.request.game.deckSelect).not.toHaveBeenCalled();
  });

  it('shows a validation error when the XML has a wrong root element', () => {
    const webClient = createMockWebClient();
    renderWithProviders(
      <DeckSelectDialog isOpen gameId={1} />,
      { preloadedState: stateWith(), webClient },
    );

    fireEvent.change(screen.getByLabelText('deck list'), {
      target: { value: '<?xml version="1.0"?><not_a_deck/>' },
    });
    fireEvent.click(screen.getByRole('button', { name: /submit deck/i }));

    expect(screen.getByRole('alert')).toBeInTheDocument();
    expect(webClient.request.game.deckSelect).not.toHaveBeenCalled();
  });

  it('dispatches deckSelect with the file contents when a valid .cod file is picked', async () => {
    const webClient = createMockWebClient();
    renderWithProviders(
      <DeckSelectDialog isOpen gameId={1} />,
      { preloadedState: stateWith(), webClient },
    );

    pickFile(VALID_COD_XML, 'my-deck.cod');

    await waitFor(() => {
      expect(screen.getByText('my-deck.cod')).toBeInTheDocument();
    });

    fireEvent.click(screen.getByRole('button', { name: /submit deck/i }));
    expect(webClient.request.game.deckSelect).toHaveBeenCalledWith(1, {
      deck: VALID_COD_XML,
    });
  });

  it('rejects a picked file whose contents are not valid Cockatrice XML', async () => {
    const webClient = createMockWebClient();
    renderWithProviders(
      <DeckSelectDialog isOpen gameId={1} />,
      { preloadedState: stateWith(), webClient },
    );

    pickFile('4 Lightning Bolt\n20 Mountain', 'bogus.cod');

    await waitFor(() => {
      expect(screen.getByText('bogus.cod')).toBeInTheDocument();
    });

    fireEvent.click(screen.getByRole('button', { name: /submit deck/i }));
    expect(screen.getByRole('alert')).toHaveTextContent(/not a valid cockatrice deck/i);
    expect(webClient.request.game.deckSelect).not.toHaveBeenCalled();
  });

  it('clears the picked filename when the user starts typing in the textarea', async () => {
    renderWithProviders(
      <DeckSelectDialog isOpen gameId={1} />,
      { preloadedState: stateWith() },
    );

    pickFile(VALID_COD_XML, 'my-deck.cod');
    await waitFor(() => {
      expect(screen.getByText('my-deck.cod')).toBeInTheDocument();
    });

    fireEvent.change(screen.getByLabelText('deck list'), {
      target: { value: VALID_COD_XML },
    });

    expect(screen.queryByText('my-deck.cod')).not.toBeInTheDocument();
    expect(screen.getByText('No file selected')).toBeInTheDocument();
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
