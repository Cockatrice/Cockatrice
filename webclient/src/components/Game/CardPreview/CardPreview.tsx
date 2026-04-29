import { useEffect, useState } from 'react';
import type { Data } from '@app/types';
import { useScryfallCard } from '@app/hooks';

import './CardPreview.css';

export interface CardPreviewProps {
  card: Data.ServerInfo_Card | null | undefined;
}

function CardPreview({ card }: CardPreviewProps) {
  const { smallUrl, normalUrl, ready } = useScryfallCard(card ?? null);
  const [normalLoaded, setNormalLoaded] = useState(false);

  useEffect(() => {
    setNormalLoaded(false);
  }, [normalUrl]);

  return (
    <div className="card-preview" data-testid="card-preview">
      {!ready && (
        <div className="card-preview__empty">Hover a card to preview</div>
      )}
      {ready && smallUrl && (
        <div className="card-preview__frame">
          <img
            className="card-preview__image card-preview__image--small"
            src={smallUrl}
            alt={card?.name ?? ''}
          />
          {normalUrl && (
            <img
              className={
                'card-preview__image card-preview__image--normal' +
                (normalLoaded ? ' card-preview__image--loaded' : '')
              }
              src={normalUrl}
              alt={card?.name ?? ''}
              onLoad={() => setNormalLoaded(true)}
              data-testid="card-preview-normal"
            />
          )}
        </div>
      )}
    </div>
  );
}

export default CardPreview;
