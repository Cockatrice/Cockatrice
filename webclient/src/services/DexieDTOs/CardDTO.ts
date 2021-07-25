import { Card } from 'types';

import { dexieService } from '../DexieService';

export class CardDTO extends Card {
  save() {
    return dexieService.cards.put(this);
  }

  static get(name) {
    return dexieService.cards.where('name').equals(name).first();
  }

  static bulkAdd(cards: CardDTO[]): Promise<any> {
    return dexieService.cards.bulkPut(cards);
  }
};

dexieService.cards.mapToClass(CardDTO);
