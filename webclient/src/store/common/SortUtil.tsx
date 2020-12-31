import { SortBy, SortDirection, User } from "types";

export class SortUtil {
  static sortByField(arr:  any[], sortBy: SortBy): void {
    if (arr.length) {
      const field = SortUtil.resolveFieldChain(arr[0], sortBy.field);
      const fieldType = typeof field;

      if (fieldType === "string") {
        SortUtil.sortByString(arr, sortBy);
        return;
      }

      if (fieldType === "number") {
        SortUtil.sortByNumber(arr, sortBy);
        return;
      }

      throw new Error("SortField must resolve to either a string or number");
    }
  }

  static sortByFields(arr: any[], sorts: SortBy[]) {
    if (arr.length) {
      arr.sort((a, b) => {
        for (let i = 0; i < sorts.length; i++) {
          const sortBy = sorts[i];
          const field = SortUtil.resolveFieldChain(arr[0], sortBy.field);
          
          const fieldType = typeof field;

          if (fieldType === "string") {
            const result = SortUtil.stringComparator(a, b, sortBy);

            if (result) {
              return result;
            }
          }

          if (fieldType === "number") {
            const result = SortUtil.numberComparator(a, b, sortBy);

            if (result) {
              return result;
            }
          }

          throw new Error("SortField must resolve to either a string or number");
        }

        return 0;
      })
    }
  }

  static sortUsersByField(users: User[], sortBy: SortBy) {
    if (users.length) {
      users.sort((a, b) => SortUtil.userComparator(a, b, sortBy))
    }
  }

  static toggleSortBy(field: string, sortBy: SortBy) {
    const sameField = field === sortBy.field;
    const isASC = sortBy.order === SortDirection.ASC;

    return {
      field,
      order: sameField && isASC ? SortDirection.DESC : SortDirection.ASC
    }
  }

  private static sortByNumber(arr: any[], sortBy: SortBy): void {
    arr.sort((a, b) => SortUtil.numberComparator(a, b, sortBy));
  }

  private static sortByString(arr: any[], sortBy: SortBy): void {
    arr.sort((a, b) => SortUtil.stringComparator(a, b, sortBy));
  }

  private static userComparator(a, b, sortBy, sortByUserLevel = true) {
    if (sortByUserLevel) {
      const adminSortBy = {
        field: "userLevel",
        order: SortDirection.DESC
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

  private static numberComparator(a, b, { field, order }: SortBy) {
    const aResolved = SortUtil.resolveFieldChain(a, field);
    const bResolved = SortUtil.resolveFieldChain(b, field);

    if (order === SortDirection.ASC) {
      return aResolved - bResolved;
    } else {
      return bResolved - aResolved;
    }
  }

  private static stringComparator(a, b, { field, order }: SortBy) {
    const aResolved = SortUtil.resolveFieldChain(a, field);
    const bResolved = SortUtil.resolveFieldChain(b, field);

    // Force empty strings to sort to bottom
    if (!aResolved && !bResolved) { return 0; }
    if (!aResolved) { return 1; }
    if (!bResolved) { return -1; }

    if (order === SortDirection.ASC) {
      return aResolved.localeCompare(bResolved);
    } else {
      return bResolved.localeCompare(aResolved);
    }
  }

  private static resolveFieldChain(obj: object, field: string) {
    const links = field.split(".");

    if (links.length > 1) {
      return links.reduce((obj, link) => {
        const parsed = parseInt(link, 10);

        if (parsed.toLocaleString() === "NaN") {
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
