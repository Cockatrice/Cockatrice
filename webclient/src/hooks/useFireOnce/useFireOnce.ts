import { useCallback, useRef, useState } from 'react';

type FireOnceFn = (...args: never[]) => unknown;

export function useFireOnce<T extends FireOnceFn>(fn: T): [boolean, () => void, (...args: Parameters<T>) => void] {
  const [actionIsInFlight, setActionIsInFlight] = useState(false);
  const fnRef = useRef(fn);
  fnRef.current = fn;

  const handleFireOnce = useCallback((...args: Parameters<T>) => {
    setActionIsInFlight(true);
    fnRef.current(...args);
  }, []);

  const resetInFlightStatus = useCallback(() => {
    setActionIsInFlight(false);
  }, []);

  return [actionIsInFlight, resetInFlightStatus, handleFireOnce];
}
