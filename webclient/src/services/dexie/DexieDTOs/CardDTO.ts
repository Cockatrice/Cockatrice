import { IndexableType } from 'dexie';
import { App } from '@app/types';

import { dexieService } from '../DexieService';

export class CardDTO extends App.Card {
  save() {
    return dexieService.cards.put(this);
  }

  static get(name: string) {
    return dexieService.cards.where('name').equalsIgnoreCase(name).first();
  }

  static bulkAdd(cards: CardDTO[]): Promise<IndexableType> {
    return dexieService.cards.bulkPut(cards);
  }
};

dexieService.cards.mapToClass(CardDTO);
