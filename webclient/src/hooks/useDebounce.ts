import { useCallback } from 'react';
import { debounce, DebouncedFunc } from 'lodash';

type UseDebounceType = (...args: any) => any;
const DEBOUNCE_DELAY = 250;

export function useDebounce<T extends UseDebounceType>(
  fn: T,
  deps: any[] = [],
  timeout: number = DEBOUNCE_DELAY
): DebouncedFunc<T> {
  return useCallback(debounce(fn, timeout), deps);
}
