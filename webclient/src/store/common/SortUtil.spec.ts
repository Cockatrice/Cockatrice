import { create } from '@bufbuild/protobuf';
import { SortDirection } from 'types';
import { ServerInfo_UserSchema } from 'generated/proto/serverinfo_user_pb';
import SortUtil from './SortUtil';

// ── sortByField ───────────────────────────────────────────────────────────────

describe('sortByField', () => {
  it('sorts string field ASC alphabetically', () => {
    const arr = [{ name: 'Zane' }, { name: 'Alice' }, { name: 'Bob' }];
    SortUtil.sortByField(arr, { field: 'name', order: SortDirection.ASC });
    expect(arr.map(x => x.name)).toEqual(['Alice', 'Bob', 'Zane']);
  });

  it('sorts string field DESC reverse-alphabetically', () => {
    const arr = [{ name: 'Alice' }, { name: 'Zane' }, { name: 'Bob' }];
    SortUtil.sortByField(arr, { field: 'name', order: SortDirection.DESC });
    expect(arr.map(x => x.name)).toEqual(['Zane', 'Bob', 'Alice']);
  });

  it('sorts number field ASC', () => {
    const arr = [{ score: 30 }, { score: 10 }, { score: 20 }];
    SortUtil.sortByField(arr, { field: 'score', order: SortDirection.ASC });
    expect(arr.map(x => x.score)).toEqual([10, 20, 30]);
  });

  it('sorts number field DESC', () => {
    const arr = [{ score: 10 }, { score: 30 }, { score: 20 }];
    SortUtil.sortByField(arr, { field: 'score', order: SortDirection.DESC });
    expect(arr.map(x => x.score)).toEqual([30, 20, 10]);
  });

  it('no-ops on empty array without error', () => {
    expect(() => SortUtil.sortByField([], { field: 'name', order: SortDirection.ASC })).not.toThrow();
  });

  it('sorts with nested dot-notation field', () => {
    const arr = [{ meta: { rank: 3 } }, { meta: { rank: 1 } }, { meta: { rank: 2 } }];
    SortUtil.sortByField(arr, { field: 'meta.rank', order: SortDirection.ASC });
    expect(arr.map(x => x.meta.rank)).toEqual([1, 2, 3]);
  });

  it('throws when field resolves to a non-string, non-number value', () => {
    const arr = [{ data: {} }, { data: {} }];
    expect(() => SortUtil.sortByField(arr, { field: 'data', order: SortDirection.ASC })).toThrow(
      'SortField must resolve to either a string or number'
    );
  });

  it('sorts empty-string values to the bottom when sorting ASC', () => {
    const arr = [{ name: '' }, { name: 'Alice' }, { name: '' }];
    SortUtil.sortByField(arr, { field: 'name', order: SortDirection.ASC });
    expect(arr[0].name).toBe('Alice');
    expect(arr[1].name).toBe('');
    expect(arr[2].name).toBe('');
  });
});

// ── sortByFields ──────────────────────────────────────────────────────────────

describe('sortByFields', () => {
  it('sorts by the first key when all items have distinct first-key values', () => {
    const arr = [
      { group: 'C', name: 'Zane' },
      { group: 'A', name: 'Bob' },
      { group: 'B', name: 'Alice' },
    ];
    SortUtil.sortByFields(arr, [
      { field: 'group', order: SortDirection.ASC },
      { field: 'name', order: SortDirection.ASC },
    ]);
    expect(arr.map(x => x.group)).toEqual(['A', 'B', 'C']);
  });

  it('breaks ties on primary key using secondary key', () => {
    const arr = [
      { group: 'A', name: 'Zane' },
      { group: 'A', name: 'Alice' },
      { group: 'B', name: 'Bob' },
    ];
    SortUtil.sortByFields(arr, [
      { field: 'group', order: SortDirection.ASC },
      { field: 'name', order: SortDirection.ASC },
    ]);
    expect(arr[0]).toEqual({ group: 'A', name: 'Alice' });
    expect(arr[1]).toEqual({ group: 'A', name: 'Zane' });
    expect(arr[2]).toEqual({ group: 'B', name: 'Bob' });
  });

  it('no-ops on empty array', () => {
    expect(() =>
      SortUtil.sortByFields([], [{ field: 'name', order: SortDirection.ASC }])
    ).not.toThrow();
  });

  it('sorts by number field', () => {
    const arr = [{ score: 3 }, { score: 1 }, { score: 2 }];
    SortUtil.sortByFields(arr, [{ field: 'score', order: SortDirection.ASC }]);
    expect(arr.map(x => x.score)).toEqual([1, 2, 3]);
  });

  it('returns 0 when all items tie on every sort key', () => {
    const arr = [{ score: 5 }, { score: 5 }];
    expect(() =>
      SortUtil.sortByFields(arr, [{ field: 'score', order: SortDirection.ASC }])
    ).not.toThrow();
    expect(arr).toHaveLength(2);
  });

  it('throws when field resolves to a non-string, non-number value', () => {
    const arr = [{ data: {} }, { data: {} }];
    expect(() =>
      SortUtil.sortByFields(arr, [{ field: 'data', order: SortDirection.ASC }])
    ).toThrow('SortField must resolve to either a string or number');
  });
});

// ── sortUsersByField ──────────────────────────────────────────────────────────

describe('sortUsersByField', () => {
  it('sorts by userLevel DESC first, then name ASC', () => {
    const users = [
      create(ServerInfo_UserSchema, { name: 'Alice', userLevel: 1, accountageSecs: 0n, privlevel: '' }),
      create(ServerInfo_UserSchema, { name: 'Bob', userLevel: 8, accountageSecs: 0n, privlevel: '' }),
      create(ServerInfo_UserSchema, { name: 'Carol', userLevel: 1, accountageSecs: 0n, privlevel: '' }),
    ];
    SortUtil.sortUsersByField(users, { field: 'name', order: SortDirection.ASC });
    expect(users[0].name).toBe('Bob');
    expect(users[1].name).toBe('Alice');
    expect(users[2].name).toBe('Carol');
  });

  it('no-ops on empty array', () => {
    expect(() =>
      SortUtil.sortUsersByField([], { field: 'name', order: SortDirection.ASC })
    ).not.toThrow();
  });

  it('returns 0 (stable) when two users tie on both userLevel and name', () => {
    const users = [
      create(ServerInfo_UserSchema, { name: 'Alice', userLevel: 1, accountageSecs: 0n, privlevel: '' }),
      create(ServerInfo_UserSchema, { name: 'Alice', userLevel: 1, accountageSecs: 0n, privlevel: '' }),
    ];
    expect(() =>
      SortUtil.sortUsersByField(users, { field: 'name', order: SortDirection.ASC })
    ).not.toThrow();
    expect(users).toHaveLength(2);
  });
});

// ── toggleSortBy ──────────────────────────────────────────────────────────────

describe('toggleSortBy', () => {
  it('same field + ASC → returns DESC', () => {
    const result = SortUtil.toggleSortBy('name', { field: 'name', order: SortDirection.ASC });
    expect(result).toEqual({ field: 'name', order: SortDirection.DESC });
  });

  it('same field + DESC → returns ASC', () => {
    const result = SortUtil.toggleSortBy('name', { field: 'name', order: SortDirection.DESC });
    expect(result).toEqual({ field: 'name', order: SortDirection.ASC });
  });

  it('different field → returns ASC regardless of current order', () => {
    const result = SortUtil.toggleSortBy('score', { field: 'name', order: SortDirection.DESC });
    expect(result).toEqual({ field: 'score', order: SortDirection.ASC });
  });
});

// ── resolveFieldChain with numeric index ─────────────────────────────────────

describe('resolveFieldChain via sortByField (numeric index)', () => {
  it('resolves numeric index in dot-notation chain', () => {
    const arr = [{ items: ['c', 'b', 'a'] }, { items: ['z', 'y', 'x'] }];
    // Sort by items.0 which is the first element of the items array
    SortUtil.sortByField(arr, { field: 'items.0', order: SortDirection.ASC });
    expect(arr[0].items[0]).toBe('c');
    expect(arr[1].items[0]).toBe('z');
  });
});
