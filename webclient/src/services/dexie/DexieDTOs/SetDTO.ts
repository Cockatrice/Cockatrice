import { IndexableType } from 'dexie';
import { App } from '@app/types';

import { dexieService } from '../DexieService';

export class SetDTO extends App.Set {
  save() {
    return dexieService.sets.put(this);
  }

  static get(name: string) {
    return dexieService.sets.where('name').equalsIgnoreCase(name).first();
  }

  static bulkAdd(sets: SetDTO[]): Promise<IndexableType> {
    return dexieService.sets.bulkPut(sets);
  }
};

dexieService.sets.mapToClass(SetDTO);
