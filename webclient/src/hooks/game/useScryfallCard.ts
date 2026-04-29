import { useMemo } from 'react';
import { App } from '@app/types';
import { getScryfallUrl } from '@app/services';

export interface ScryfallCard {
  smallUrl: string | null;
  normalUrl: string | null;
  ready: boolean;
}

interface CardLike {
  providerId?: string;
  name?: string;
}

export function useScryfallCard(card: CardLike | null | undefined): ScryfallCard {
  return useMemo<ScryfallCard>(() => {
    if (!card) {
      return { smallUrl: null, normalUrl: null, ready: false };
    }
    const smallUrl = getScryfallUrl(card, App.ScryfallImageSize.Small);
    const normalUrl = getScryfallUrl(card, App.ScryfallImageSize.Normal);
    return {
      smallUrl,
      normalUrl,
      ready: smallUrl != null,
    };
  }, [card?.providerId, card?.name]);
}
