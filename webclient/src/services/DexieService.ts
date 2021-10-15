import Dexie from 'dexie';

enum Stores {
  CARDS = 'cards',
  SETS = 'sets',
  TOKENS = 'tokens',
}

const StoreKeyIndexes = {
  [Stores.CARDS]: "name",
  [Stores.SETS]: "code",
  [Stores.TOKENS]: "name.value",
};

class DexieService {
  private db: Dexie = new Dexie('Webatrice');

  constructor() {
    this.db.version(1).stores(StoreKeyIndexes);
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
}

export const dexieService = new DexieService();
