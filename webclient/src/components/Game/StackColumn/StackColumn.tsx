import { useScryfallCard } from '@app/hooks';
import { GameSelectors, useAppSelector } from '@app/store';
import { App, Data } from '@app/types';
import { cx } from '@app/utils';

import './StackColumn.css';

export interface StackColumnProps {
  gameId: number;
  playerId: number;
  mirrored?: boolean;
  onCardHover?: (card: Data.ServerInfo_Card) => void;
}

interface StackThumbProps {
  card: Data.ServerInfo_Card;
  onHover?: (card: Data.ServerInfo_Card) => void;
}

function StackThumb({ card, onHover }: StackThumbProps) {
  const { smallUrl } = useScryfallCard(card);
  return (
    <div
      className="stack-column__thumb"
      onMouseEnter={() => onHover?.(card)}
      title={card.name}
    >
      {smallUrl && !card.faceDown ? (
        <img className="stack-column__image" src={smallUrl} alt={card.name} />
      ) : (
        <div className="stack-column__placeholder" />
      )}
    </div>
  );
}

function StackColumn({ gameId, playerId, mirrored = false, onCardHover }: StackColumnProps) {
  const zone = useAppSelector((state) =>
    GameSelectors.getZone(state, gameId, playerId, App.ZoneName.STACK),
  );
  const cards = zone ? zone.order.map((id) => zone.byId[id]).filter(Boolean) : [];

  return (
    <div
      className={cx('stack-column', { 'stack-column--mirrored': mirrored })}
      data-testid={`stack-column-${playerId}`}
    >
      <div className="stack-column__cards" data-testid={`stack-column-cards-${playerId}`}>
        {cards.map((c) => (
          <StackThumb key={c.id} card={c} onHover={onCardHover} />
        ))}
      </div>
    </div>
  );
}

export default StackColumn;
