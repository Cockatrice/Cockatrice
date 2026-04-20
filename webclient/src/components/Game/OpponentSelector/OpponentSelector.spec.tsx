import { render, screen, fireEvent, within } from '@testing-library/react';

import OpponentSelector from './OpponentSelector';

describe('OpponentSelector', () => {
  it('does not render with fewer than 2 opponents (2-player game)', () => {
    const { container } = render(
      <OpponentSelector
        opponents={[{ playerId: 2, name: 'Solo' }]}
        selectedPlayerId={2}
        onSelect={vi.fn()}
      />,
    );

    expect(container.firstChild).toBeNull();
  });

  it('renders with 2+ opponents', () => {
    render(
      <OpponentSelector
        opponents={[
          { playerId: 2, name: 'Alice' },
          { playerId: 3, name: 'Bob' },
        ]}
        selectedPlayerId={2}
        onSelect={vi.fn()}
      />,
    );

    expect(screen.getByTestId('opponent-selector')).toBeInTheDocument();
    expect(screen.getByText('Opponent:')).toBeInTheDocument();
    expect(screen.getByText('Alice')).toBeInTheDocument();
  });

  it('fires onSelect with the chosen opponent playerId', () => {
    const onSelect = vi.fn();
    render(
      <OpponentSelector
        opponents={[
          { playerId: 2, name: 'Alice' },
          { playerId: 3, name: 'Bob' },
          { playerId: 4, name: 'Carol' },
        ]}
        selectedPlayerId={2}
        onSelect={onSelect}
      />,
    );

    fireEvent.mouseDown(screen.getByRole('combobox'));
    const listbox = within(screen.getByRole('listbox'));
    fireEvent.click(listbox.getByText('Carol'));

    expect(onSelect).toHaveBeenCalledWith(4);
  });
});
