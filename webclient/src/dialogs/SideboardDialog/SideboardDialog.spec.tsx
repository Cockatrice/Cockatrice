import { screen, fireEvent, within } from '@testing-library/react';
import { App } from '@app/types';

import { renderWithProviders } from '../../__test-utils__';
import SideboardDialog, { applyMoves } from './SideboardDialog';

const DEFAULT_PROPS = {
  isOpen: true,
  playerName: 'P1',
  deckCards: [
    { id: 1, name: 'Island' },
    { id: 2, name: 'Mountain' },
  ],
  sideboardCards: [
    { id: 10, name: 'Counterspell' },
  ],
  isLocked: false,
  onSubmit: () => {},
  onCancel: () => {},
  onToggleLock: () => {},
};

describe('SideboardDialog', () => {
  it('renders deck and sideboard columns with counts', () => {
    renderWithProviders(<SideboardDialog {...DEFAULT_PROPS} />);

    expect(screen.getByText(/main deck \(2\)/i)).toBeInTheDocument();
    expect(screen.getByText(/^sideboard \(1\)/i)).toBeInTheDocument();
    expect(screen.getByText('Island')).toBeInTheDocument();
    expect(screen.getByText('Counterspell')).toBeInTheDocument();
  });

  it('moves a card from deck to sideboard when → is clicked (local draft only)', () => {
    const onSubmit = vi.fn();
    renderWithProviders(
      <SideboardDialog {...DEFAULT_PROPS} onSubmit={onSubmit} />,
    );

    fireEvent.click(screen.getByRole('button', { name: /move Island to sideboard/i }));

    // Island now shown in the sideboard list; counts updated.
    expect(screen.getByText(/main deck \(1\)/i)).toBeInTheDocument();
    expect(screen.getByText(/^sideboard \(2\)/i)).toBeInTheDocument();
    // No dispatch yet — Apply hasn't been clicked.
    expect(onSubmit).not.toHaveBeenCalled();
  });

  it('submits the accumulated draft as a moveList when Apply is clicked', () => {
    const onSubmit = vi.fn();
    renderWithProviders(
      <SideboardDialog {...DEFAULT_PROPS} onSubmit={onSubmit} />,
    );

    fireEvent.click(screen.getByRole('button', { name: /move Island to sideboard/i }));
    fireEvent.click(screen.getByRole('button', { name: /move Counterspell to main deck/i }));
    fireEvent.click(screen.getByRole('button', { name: /apply plan/i }));

    expect(onSubmit).toHaveBeenCalledWith([
      { cardName: 'Island', startZone: App.ZoneName.DECK, targetZone: App.ZoneName.SIDEBOARD },
      { cardName: 'Counterspell', startZone: App.ZoneName.SIDEBOARD, targetZone: App.ZoneName.DECK },
    ]);
  });

  it('disables move buttons and Apply when isLocked is true', () => {
    renderWithProviders(<SideboardDialog {...DEFAULT_PROPS} isLocked />);

    expect(screen.getByRole('button', { name: /move Island to sideboard/i })).toBeDisabled();
    expect(screen.getByRole('button', { name: /move Counterspell to main deck/i })).toBeDisabled();
    expect(screen.getByRole('button', { name: /apply plan/i })).toBeDisabled();
    expect(screen.getByRole('note')).toHaveTextContent(/sideboard is locked/i);
  });

  it('dispatches onToggleLock when the Lock checkbox changes', () => {
    const onToggleLock = vi.fn();
    renderWithProviders(
      <SideboardDialog {...DEFAULT_PROPS} onToggleLock={onToggleLock} />,
    );

    fireEvent.click(screen.getByLabelText('Lock sideboard'));
    expect(onToggleLock).toHaveBeenCalledWith(true);
  });

  it('resets the draft when the sideboard is locked mid-edit (desktop resetSideboardPlan parity)', () => {
    const onSubmit = vi.fn();
    const { rerender } = renderWithProviders(
      <SideboardDialog {...DEFAULT_PROPS} onSubmit={onSubmit} />,
    );

    fireEvent.click(screen.getByRole('button', { name: /move Island to sideboard/i }));
    expect(screen.getByRole('button', { name: /apply plan \(1\)/i })).toBeInTheDocument();

    rerender(<SideboardDialog {...DEFAULT_PROPS} isLocked onSubmit={onSubmit} />);

    // Draft cleared — Apply label no longer carries a count and the lists
    // reflect the original wire snapshot.
    expect(screen.queryByRole('button', { name: /apply plan \(/i })).not.toBeInTheDocument();
    expect(screen.getByText(/main deck \(2\)/i)).toBeInTheDocument();
  });

  it('renders (empty) when a column has no cards', () => {
    renderWithProviders(
      <SideboardDialog {...DEFAULT_PROPS} sideboardCards={[]} />,
    );

    const sbList = screen.getByTestId('sideboard-dialog-sb');
    expect(within(sbList).getByText(/\(empty\)/)).toBeInTheDocument();
  });

  it('fires onCancel when the Cancel button is clicked', () => {
    const onCancel = vi.fn();
    renderWithProviders(
      <SideboardDialog {...DEFAULT_PROPS} onCancel={onCancel} />,
    );

    fireEvent.click(screen.getByRole('button', { name: /cancel/i }));
    expect(onCancel).toHaveBeenCalled();
  });
});

describe('applyMoves', () => {
  it('processes moves in order, identifying cards by name', () => {
    const deck = [{ id: 1, name: 'A' }, { id: 2, name: 'B' }];
    const sb = [{ id: 3, name: 'C' }];

    const result = applyMoves(deck, sb, [
      { cardName: 'A', startZone: App.ZoneName.DECK, targetZone: App.ZoneName.SIDEBOARD },
      { cardName: 'C', startZone: App.ZoneName.SIDEBOARD, targetZone: App.ZoneName.DECK },
    ]);

    expect(result.deck.map((c) => c.name).sort()).toEqual(['B', 'C']);
    expect(result.sideboard.map((c) => c.name)).toEqual(['A']);
  });

  it('drops moves that reference cards not present in the source zone', () => {
    const deck = [{ id: 1, name: 'A' }];
    const result = applyMoves(deck, [], [
      { cardName: 'Missing', startZone: App.ZoneName.DECK, targetZone: App.ZoneName.SIDEBOARD },
    ]);

    expect(result.deck).toHaveLength(1);
    expect(result.sideboard).toHaveLength(0);
  });
});
