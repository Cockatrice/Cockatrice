import { App } from '@app/types';

import { dexieService } from '../DexieService';

export class SettingDTO extends App.Setting {
  constructor(user) {
    super();

    this.user = user;
    this.autoConnect = false;
  }

  save() {
    return dexieService.settings.put(this);
  }

  static get(user) {
    return dexieService.settings.where('user').equalsIgnoreCase(user).first();
  }
};

dexieService.settings.mapToClass(SettingDTO);
