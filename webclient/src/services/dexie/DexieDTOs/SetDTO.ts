import { Set } from 'types';

import { dexieService } from '../DexieService';

export class SetDTO extends Set {
  save() {
    return dexieService.sets.put(this);
  }

  static get(name) {
    return dexieService.sets.where('name').equalsIgnoreCase(name).first();
  }

  static bulkAdd(sets: SetDTO[]): Promise<any> {
    return dexieService.sets.bulkPut(sets);
  }
};

dexieService.sets.mapToClass(SetDTO);
