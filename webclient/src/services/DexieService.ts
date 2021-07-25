import Dexie from 'dexie';

class DexieService {
  private db: Dexie = new Dexie('Webatrice');

  constructor() {
    this.db.version(1).stores({
      cards: "name",
      sets: "code",
      tokens: "name.value",
    });
  }

  get cards() {
    return (this.db as any).cards;
  }

  get sets() {
    return (this.db as any).sets;
  }

  get tokens() {
    return (this.db as any).tokens;
  }
}

export const dexieService = new DexieService();
