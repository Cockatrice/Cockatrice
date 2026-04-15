import { App, Data } from '@app/types';

export default class SortUtil {
  static sortByField<T extends object>(arr: T[], sortBy: App.SortBy): void {
    if (arr.length) {
      const field = SortUtil.resolveFieldChain(arr[0], sortBy.field);
      const fieldType = typeof field;

      if (fieldType === 'string') {
        SortUtil.sortByString(arr, sortBy);
        return;
      }

      if (fieldType === 'number') {
        SortUtil.sortByNumber(arr, sortBy);
        return;
      }

      throw new Error('SortField must resolve to either a string or number');
    }
  }

  static sortByFields<T extends object>(arr: T[], sorts: App.SortBy[]) {
    if (arr.length) {
      arr.sort((a, b) => {
        for (let i = 0; i < sorts.length; i++) {
          const sortBy = sorts[i];
          const field = SortUtil.resolveFieldChain(arr[0], sortBy.field);

          const fieldType = typeof field;

          if (fieldType === 'string') {
            const result = SortUtil.stringComparator(a, b, sortBy);

            if (result) {
              return result;
            }
          } else if (fieldType === 'number') {
            const result = SortUtil.numberComparator(a, b, sortBy);

            if (result) {
              return result;
            }
          } else {
            throw new Error('SortField must resolve to either a string or number');
          }
        }

        return 0;
      })
    }
  }

  static sortUsersByField(users: Data.ServerInfo_User[], sortBy: App.SortBy) {
    if (users.length) {
      users.sort((a, b) => SortUtil.userComparator(a, b, sortBy))
    }
  }

  static toggleSortBy<F extends string>(field: F, sortBy: App.SortBy): { field: F; order: App.SortDirection } {
    const sameField = field === sortBy.field;
    const isASC = sortBy.order === App.SortDirection.ASC;

    return {
      field,
      order: sameField && isASC ? App.SortDirection.DESC : App.SortDirection.ASC
    }
  }

  private static sortByNumber<T extends object>(arr: T[], sortBy: App.SortBy): void {
    arr.sort((a, b) => SortUtil.numberComparator(a, b, sortBy));
  }

  private static sortByString<T extends object>(arr: T[], sortBy: App.SortBy): void {
    arr.sort((a, b) => SortUtil.stringComparator(a, b, sortBy));
  }

  private static userComparator(a: Data.ServerInfo_User, b: Data.ServerInfo_User, sortBy: App.SortBy, sortByUserLevel = true) {
    if (sortByUserLevel) {
      const adminSortBy = {
        field: 'userLevel',
        order: App.SortDirection.DESC
      };

      const adminSorted = SortUtil.numberComparator(a, b, adminSortBy);

      if (adminSorted) {
        return adminSorted;
      }
    }

    const sorted = SortUtil.stringComparator(a, b, sortBy);

    if (sorted) {
      return sorted;
    }

    return 0;
  }

  private static numberComparator<T extends object>(a: T, b: T, { field, order }: App.SortBy) {
    const aResolved = SortUtil.resolveFieldChain(a, field);
    const bResolved = SortUtil.resolveFieldChain(b, field);

    if (order === App.SortDirection.ASC) {
      return aResolved - bResolved;
    } else {
      return bResolved - aResolved;
    }
  }

  private static stringComparator<T extends object>(a: T, b: T, { field, order }: App.SortBy) {
    const aResolved = SortUtil.resolveFieldChain(a, field);
    const bResolved = SortUtil.resolveFieldChain(b, field);

    // Force empty strings to sort to bottom
    if (!aResolved && !bResolved) {
      return 0;
    }
    if (!aResolved) {
      return 1;
    }
    if (!bResolved) {
      return -1;
    }

    if (order === App.SortDirection.ASC) {
      return aResolved.localeCompare(bResolved);
    } else {
      return bResolved.localeCompare(aResolved);
    }
  }

  private static resolveFieldChain(obj: object, field: string) {
    const links = field.split('.');

    if (links.length > 1) {
      return links.reduce((obj, link) => {
        const parsed = parseInt(link, 10);

        if (parsed.toLocaleString() === 'NaN') {
          return obj[link];
        } else {
          return obj[parsed];
        }
      }, obj) || null;
    } else {
      return obj[field];
    }
  }
}
