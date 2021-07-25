import { Token } from 'types';

import { dexieService } from '../DexieService';

export class TokenDTO extends Token {
  save() {
    return dexieService.tokens.put(this);
  }

  static get(name) {
    return dexieService.tokens.where('name.value').equals(name).first();
  }

  static bulkAdd(tokens: TokenDTO[]): Promise<any> {
    return dexieService.tokens.bulkPut(tokens);
  }
};

dexieService.tokens.mapToClass(TokenDTO);
