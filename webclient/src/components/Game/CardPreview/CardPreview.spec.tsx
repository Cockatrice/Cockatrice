import { render, screen, fireEvent } from '@testing-library/react';

import { makeCard } from '../../../store/game/__mocks__/fixtures';
import CardPreview from './CardPreview';

describe('CardPreview', () => {
  it('shows an empty hint when no card is hovered', () => {
    render(<CardPreview card={null} />);
    expect(screen.getByText(/hover a card/i)).toBeInTheDocument();
  });

  it('renders the small image immediately on hover', () => {
    const card = makeCard({ name: 'Lightning Bolt' });
    render(<CardPreview card={card} />);

    const small = document.querySelector('.card-preview__image--small') as HTMLImageElement;
    expect(small).not.toBeNull();
    expect(small.src).toContain('version=small');
    expect(small.src).toContain('Lightning%20Bolt');
  });

  it('renders a normal image that stays transparent until it loads', () => {
    const card = makeCard({ name: 'Lightning Bolt' });
    render(<CardPreview card={card} />);

    const normal = screen.getByTestId('card-preview-normal') as HTMLImageElement;
    expect(normal.src).toContain('version=normal');
    expect(normal).not.toHaveClass('card-preview__image--loaded');
  });

  it('reveals the normal image once onLoad fires', () => {
    const card = makeCard({ name: 'Lightning Bolt' });
    render(<CardPreview card={card} />);

    const normal = screen.getByTestId('card-preview-normal');
    fireEvent.load(normal);
    expect(normal).toHaveClass('card-preview__image--loaded');
  });

  it('resets the loaded flag when the card changes', () => {
    const a = makeCard({ id: 1, name: 'A' });
    const b = makeCard({ id: 2, name: 'B' });
    const { rerender } = render(<CardPreview card={a} />);

    fireEvent.load(screen.getByTestId('card-preview-normal'));
    expect(screen.getByTestId('card-preview-normal')).toHaveClass(
      'card-preview__image--loaded',
    );

    rerender(<CardPreview card={b} />);
    expect(screen.getByTestId('card-preview-normal')).not.toHaveClass(
      'card-preview__image--loaded',
    );
  });
});
