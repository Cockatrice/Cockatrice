import { IndexableType } from 'dexie';
import { App } from '@app/types';

import { dexieService } from '../DexieService';

export class TokenDTO extends App.Token {
  save() {
    return dexieService.tokens.put(this);
  }

  static get(name: string) {
    return dexieService.tokens.where('name.value').equalsIgnoreCase(name).first();
  }

  static bulkAdd(tokens: TokenDTO[]): Promise<IndexableType> {
    return dexieService.tokens.bulkPut(tokens);
  }
};

dexieService.tokens.mapToClass(TokenDTO);
