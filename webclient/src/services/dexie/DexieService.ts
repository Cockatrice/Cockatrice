import Dexie from 'dexie';

import { Stores, schemaV1 } from './DexieSchemas/v1.schema';

class DexieService {
  private db: Dexie = new Dexie('Webatrice');

  constructor() {
    schemaV1(this.db);
  }

  get settings() {
    return this.db.table(Stores.SETTINGS);
  }

  get cards() {
    return this.db.table(Stores.CARDS);
  }

  get sets() {
    return this.db.table(Stores.SETS);
  }

  get tokens() {
    return this.db.table(Stores.TOKENS);
  }

  get hosts() {
    return this.db.table(Stores.HOSTS);
  }

  testConnection() {
    return this.db.open();
  }
}

export const dexieService = new DexieService();
