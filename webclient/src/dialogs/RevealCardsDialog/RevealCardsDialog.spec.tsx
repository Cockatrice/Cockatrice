import { screen, fireEvent } from '@testing-library/react';

import { renderWithProviders } from '../../__test-utils__';
import RevealCardsDialog from './RevealCardsDialog';

const PLAYERS = [
  { playerId: 1, name: 'Alice' },
  { playerId: 2, name: 'Bob' },
];

describe('RevealCardsDialog', () => {
  it('defaults to All players and reports -1 + topCards=-1 for a zone-wide reveal', () => {
    const onSubmit = vi.fn();
    renderWithProviders(
      <RevealCardsDialog
        isOpen
        title="Reveal hand"
        zoneLabel="Hand"
        players={PLAYERS}
        onSubmit={onSubmit}
        onCancel={() => {}}
      />,
    );

    fireEvent.click(screen.getByRole('button', { name: /reveal/i }));

    expect(onSubmit).toHaveBeenCalledWith({ targetPlayerId: -1, topCards: -1 });
  });

  it('emits the selected target playerId', () => {
    const onSubmit = vi.fn();
    renderWithProviders(
      <RevealCardsDialog
        isOpen
        title="Reveal hand"
        zoneLabel="Hand"
        players={PLAYERS}
        onSubmit={onSubmit}
        onCancel={() => {}}
      />,
    );

    // MUI Select opens a popover; clicking the role=button for the Select.
    fireEvent.mouseDown(screen.getByRole('combobox'));
    fireEvent.click(screen.getByRole('option', { name: 'Bob' }));
    fireEvent.click(screen.getByRole('button', { name: /reveal/i }));

    expect(onSubmit).toHaveBeenCalledWith({ targetPlayerId: 2, topCards: -1 });
  });

  it('validates the count input when showCountInput is true', () => {
    const onSubmit = vi.fn();
    renderWithProviders(
      <RevealCardsDialog
        isOpen
        title="Reveal top N"
        zoneLabel="Library"
        showCountInput
        defaultCount={3}
        players={PLAYERS}
        onSubmit={onSubmit}
        onCancel={() => {}}
      />,
    );

    const input = screen.getByRole('spinbutton') as HTMLInputElement;
    expect(input.value).toBe('3');
    fireEvent.change(input, { target: { value: '5' } });
    fireEvent.click(screen.getByRole('button', { name: /reveal/i }));

    expect(onSubmit).toHaveBeenCalledWith({ targetPlayerId: -1, topCards: 5 });
  });

  it('rejects a non-positive count', () => {
    const onSubmit = vi.fn();
    renderWithProviders(
      <RevealCardsDialog
        isOpen
        title="Reveal top N"
        zoneLabel="Library"
        showCountInput
        players={PLAYERS}
        onSubmit={onSubmit}
        onCancel={() => {}}
      />,
    );

    fireEvent.change(screen.getByRole('spinbutton'), {
      target: { value: '0' },
    });
    fireEvent.click(screen.getByRole('button', { name: /reveal/i }));

    expect(onSubmit).not.toHaveBeenCalled();
    expect(screen.getByText(/enter a positive integer/i)).toBeInTheDocument();
  });

  it('calls onCancel when Cancel is clicked', () => {
    const onCancel = vi.fn();
    renderWithProviders(
      <RevealCardsDialog
        isOpen
        title="Reveal"
        zoneLabel="Hand"
        players={PLAYERS}
        onSubmit={() => {}}
        onCancel={onCancel}
      />,
    );

    fireEvent.click(screen.getByRole('button', { name: /cancel/i }));

    expect(onCancel).toHaveBeenCalled();
  });
});
