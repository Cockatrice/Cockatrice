/**
File is adapted from https://github.com/Qeepsake/use-redux-effect under MIT License
 * @author Aspect Apps Limited
 * @description
 */

import { useEffect, useRef, DependencyList } from 'react'
import { useStore } from 'react-redux'
import { castArray } from 'lodash'

// Actions are identified by string `type` at runtime, so the callback
// receives an untyped action object to allow free property access.

export type ReduxEffect = (action: any) => void

/**
 * Subscribes to redux store events.
 *
 * On mount, synchronously inspects the current `state.action` so an action
 * dispatched between render and effect-commit is still observed — this is
 * what lets `<Server />` catch a `JOIN_ROOM` that auto-join fired while the
 * route was transitioning.
 */
export function useReduxEffect(
  effect: ReduxEffect,
  type: string | string[],
  deps: DependencyList = [],
): void {
  const store = useStore();
  const effectRef = useRef(effect);
  const typeRef = useRef(type);
  // Persists across StrictMode's mount → unmount → remount cycle so we
  // don't re-fire for an action we already handled on the first mount.
  const lastHandledCountRef = useRef<number>(-1);

  effectRef.current = effect;
  typeRef.current = type;

  useEffect(() => {
    const check = (): void => {
      const action = (store.getState() as any).action;
      if (!action || action.count === lastHandledCountRef.current) {
        return;
      }
      lastHandledCountRef.current = action.count;
      if (castArray(typeRef.current).includes(action.type)) {
        effectRef.current(action);
      }
    };

    check();

    const unsubscribe = store.subscribe(check);
    return (): void => unsubscribe();
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, deps);
}
