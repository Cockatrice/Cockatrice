import { useCallback } from 'react';

type UseDebounceType = (...args: any) => any;
const DEBOUNCE_DELAY = 250;

export interface DebouncedFn<T extends UseDebounceType> {
  (...args: Parameters<T>): void;
  cancel(): void;
}

function debounce<T extends UseDebounceType>(fn: T, timeout: number): DebouncedFn<T> {
  let timer: ReturnType<typeof setTimeout> | undefined;
  const debounced = ((...args: Parameters<T>): void => {
    if (timer !== undefined) {
      clearTimeout(timer);
    }
    timer = setTimeout(() => fn(...args), timeout);
  }) as DebouncedFn<T>;
  debounced.cancel = (): void => {
    if (timer !== undefined) {
      clearTimeout(timer);
      timer = undefined;
    }
  };
  return debounced;
}

export function useDebounce<T extends UseDebounceType>(
  fn: T,
  deps: any[] = [],
  timeout: number = DEBOUNCE_DELAY
): DebouncedFn<T> {
  return useCallback(debounce(fn, timeout), deps);
}
