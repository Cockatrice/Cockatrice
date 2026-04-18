import { useCallback, useRef, useState } from 'react';

type UseFireOnceType = (...args: any) => any;

export function useFireOnce<T extends UseFireOnceType>(fn: T): [boolean, () => void, (...args: Parameters<T>) => void] {
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
