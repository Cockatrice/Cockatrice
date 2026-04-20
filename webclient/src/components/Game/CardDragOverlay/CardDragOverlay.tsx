import { useScryfallCard } from '@app/hooks';
import type { Data } from '@app/types';

import './CardDragOverlay.css';

export interface CardDragOverlayProps {
  card: Data.ServerInfo_Card;
}

function CardDragOverlay({ card }: CardDragOverlayProps) {
  const { smallUrl } = useScryfallCard(card);

  return (
    <div className="card-drag-overlay" data-testid="card-drag-overlay">
      {card.faceDown || !smallUrl ? (
        <div className="card-drag-overlay__back" aria-label="face-down card" />
      ) : (
        <img className="card-drag-overlay__image" src={smallUrl} alt={card.name} />
      )}
    </div>
  );
}

export default CardDragOverlay;
