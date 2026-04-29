/**
 * Joins truthy class names into a space-separated string.
 * Matches the widely-used `clsx`/`classnames` contract but stays a tiny
 * in-tree helper so we don't pull another dependency.
 *
 * Accepts strings, arrays of strings, and objects mapping class names to
 * booleans. Falsy values are dropped.
 *
 * @example
 *   cx('card-slot', tapped && 'card-slot--tapped', {
 *     'card-slot--face-down': card.faceDown,
 *   });
 */
export type CxArg =
  | string
  | number
  | false
  | null
  | undefined
  | CxArg[]
  | { [className: string]: unknown };

export function cx(...args: CxArg[]): string {
  const out: string[] = [];
  for (const arg of args) {
    if (!arg) {
      continue;
    }
    if (typeof arg === 'string' || typeof arg === 'number') {
      out.push(String(arg));
    } else if (Array.isArray(arg)) {
      const nested = cx(...arg);
      if (nested) {
        out.push(nested);
      }
    } else if (typeof arg === 'object') {
      for (const key of Object.keys(arg)) {
        if (arg[key]) {
          out.push(key);
        }
      }
    }
  }
  return out.join(' ');
}
