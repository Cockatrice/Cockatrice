export enum Stores {
  SETTINGS = 'settings',
  CARDS = 'cards',
  SETS = 'sets',
  TOKENS = 'tokens',
  HOSTS = 'hosts',
}

export const schemaV1 = (db) => {
  db.version(1).stores({
    [Stores.CARDS]: 'name',
    [Stores.SETS]: 'code',
    [Stores.SETTINGS]: 'user',
    [Stores.TOKENS]: 'name.value',
    [Stores.HOSTS]: '++id',
  });
}
