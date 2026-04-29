import { ReactElement } from 'react';
import { render as rtlRender, screen, fireEvent } from '@testing-library/react';
import { create } from '@bufbuild/protobuf';
import { DndContext } from '@dnd-kit/core';
import { Data } from '@app/types';

import { makeCard } from '../../../store/game/__mocks__/fixtures';
import CardSlot from './CardSlot';

// useDraggable requires a DndContext ancestor; keep a lightweight wrapper
// for these leaf tests rather than paying for the full renderWithProviders.
const render = (ui: ReactElement) =>
  rtlRender(<DndContext>{ui}</DndContext>);

describe('CardSlot', () => {
  it('renders the Scryfall image for a normal card', () => {
    const card = makeCard({ name: 'Lightning Bolt', id: 1 });
    render(<CardSlot card={card} />);

    const img = screen.getByAltText('Lightning Bolt') as HTMLImageElement;
    expect(img.src).toContain('/cards/named');
    expect(img.src).toContain('Lightning%20Bolt');
    expect(img.src).toContain('version=small');
  });

  it('uses providerId over name when present', () => {
    const card = makeCard({ name: 'Anything', providerId: 'abc-123', id: 1 });
    render(<CardSlot card={card} />);

    const img = screen.getByAltText('Anything') as HTMLImageElement;
    expect(img.src).toContain('/cards/abc-123');
  });

  it('renders a face-down back and suppresses image/P-T/counters when faceDown', () => {
    const card = makeCard({
      name: 'Hidden',
      faceDown: true,
      pt: '3/3',
      counterList: [create(Data.ServerInfo_CardCounterSchema, { id: 1, value: 2 })],
    });
    render(<CardSlot card={card} />);

    expect(screen.getByLabelText('face-down card')).toBeInTheDocument();
    expect(screen.queryByAltText('Hidden')).not.toBeInTheDocument();
    expect(screen.queryByText('3/3')).not.toBeInTheDocument();
  });

  it('adds the tapped modifier when card.tapped is true', () => {
    const card = makeCard({ tapped: true });
    render(<CardSlot card={card} />);
    expect(screen.getByTestId('card-slot')).toHaveClass('card-slot--tapped');
  });

  it('adds the inverted modifier when prop inverted is true', () => {
    const card = makeCard();
    render(<CardSlot card={card} inverted />);
    expect(screen.getByTestId('card-slot')).toHaveClass('card-slot--inverted');
  });

  it('combines tapped and inverted classes so CSS can compose rotation', () => {
    const card = makeCard({ tapped: true });
    render(<CardSlot card={card} inverted />);
    const el = screen.getByTestId('card-slot');
    expect(el).toHaveClass('card-slot--tapped');
    expect(el).toHaveClass('card-slot--inverted');
  });

  it('renders P/T overlay when pt is set', () => {
    const card = makeCard({ pt: '5/5' });
    render(<CardSlot card={card} />);
    expect(screen.getByText('5/5')).toBeInTheDocument();
  });

  it('renders annotation overlay when annotation is set', () => {
    const card = makeCard({ annotation: 'note' });
    render(<CardSlot card={card} />);
    expect(screen.getByText('note')).toBeInTheDocument();
  });

  it('renders a counter badge per card counter', () => {
    const card = makeCard({
      counterList: [
        create(Data.ServerInfo_CardCounterSchema, { id: 1, value: 3 }),
        create(Data.ServerInfo_CardCounterSchema, { id: 2, value: 7 }),
      ],
    });
    render(<CardSlot card={card} />);
    expect(screen.getByText('3')).toBeInTheDocument();
    expect(screen.getByText('7')).toBeInTheDocument();
  });

  it('adds the attacking modifier when card.attacking is true', () => {
    const card = makeCard({ attacking: true });
    render(<CardSlot card={card} />);
    expect(screen.getByTestId('card-slot')).toHaveClass('card-slot--attacking');
  });

  it('invokes click handlers with the card payload', () => {
    const card = makeCard();
    const onClick = vi.fn();
    const onDoubleClick = vi.fn();
    const onContextMenu = vi.fn();
    const onMouseEnter = vi.fn();
    render(
      <CardSlot
        card={card}
        onClick={onClick}
        onDoubleClick={onDoubleClick}
        onContextMenu={onContextMenu}
        onMouseEnter={onMouseEnter}
      />,
    );

    const el = screen.getByTestId('card-slot');
    fireEvent.click(el);
    fireEvent.doubleClick(el);
    fireEvent.contextMenu(el);
    fireEvent.mouseEnter(el);

    expect(onClick).toHaveBeenCalledWith(card);
    expect(onDoubleClick).toHaveBeenCalledWith(card);
    expect(onContextMenu).toHaveBeenCalled();
    expect(onContextMenu.mock.calls[0][0]).toBe(card);
    expect(onMouseEnter).toHaveBeenCalledWith(card);
  });
});
