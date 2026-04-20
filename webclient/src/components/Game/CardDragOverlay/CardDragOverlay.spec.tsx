import { render, screen } from '@testing-library/react';

import { makeCard } from '../../../store/game/__mocks__/fixtures';
import CardDragOverlay from './CardDragOverlay';

describe('CardDragOverlay', () => {
  it('renders the Scryfall image for a face-up card', () => {
    render(<CardDragOverlay card={makeCard({ name: 'Lightning Bolt' })} />);

    const img = screen.getByAltText('Lightning Bolt') as HTMLImageElement;
    expect(img.src).toContain('Lightning%20Bolt');
    expect(img.src).toContain('version=small');
  });

  it('renders the face-down placeholder for hidden cards', () => {
    render(<CardDragOverlay card={makeCard({ faceDown: true })} />);

    expect(screen.getByLabelText('face-down card')).toBeInTheDocument();
  });
});
