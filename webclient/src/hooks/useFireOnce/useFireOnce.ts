import { useCallback, useState } from 'react';

type UseFireOnceType = (...args: any) => any;

export function useFireOnce<T extends UseFireOnceType>(fn: T): [boolean, any, any] {
  const [actionIsInFlight, setActionIsInFlight] = useState(false)
  const handleFireOnce = useCallback((args) => {
    setActionIsInFlight(true);
    fn(args);
  }, [])
  function resetInFlightStatus() {
    setActionIsInFlight(false);
  }
  return [actionIsInFlight, resetInFlightStatus, handleFireOnce]
}
