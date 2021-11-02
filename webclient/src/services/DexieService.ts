import Dexie from 'dexie';

enum Stores {
  SETTINGS = 'settings',
  CARDS = 'cards',
  SETS = 'sets',
  TOKENS = 'tokens',
  HOSTS = 'hosts',
}

const StoreKeyIndexes = {
  [Stores.CARDS]: 'name',
  [Stores.SETS]: 'code',
  [Stores.SETTINGS]: 'user',
  [Stores.TOKENS]: 'name.value',
  [Stores.HOSTS]: '++id,name',
};

class DexieService {
  private db: Dexie = new Dexie('Webatrice');

  constructor() {
    this.db.version(2).stores(StoreKeyIndexes);
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
}

export const dexieService = new DexieService();
