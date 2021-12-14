import { useCallback, useState } from 'react';

type UseFireOnceType = (...args: any) => any;

export function useFireOnce<T extends UseFireOnceType>(fn: T): [boolean, any] {
    const [isActionPrevented, setIsActionPrevented] = useState(false)
    const handleFireOnce = useCallback((args) => {
        setIsActionPrevented(true);
        fn(args);
    }, [])
    return [isActionPrevented, handleFireOnce]
}