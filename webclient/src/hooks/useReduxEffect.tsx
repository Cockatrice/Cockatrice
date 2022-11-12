/**
File is adapted from https://github.com/Qeepsake/use-redux-effect under MIT License
 * @author Aspect Apps Limited
 * @description
 */

import { useRef, useEffect, DependencyList } from 'react'
import { useStore } from 'react-redux'
import { AnyAction } from 'redux'
import { castArray } from 'lodash'

export type ReduxEffect = (action: AnyAction) => void

/**
  * Subscribes to redux store events
  *
  * @param effect
  * @param type
  * @param deps
  */
export function useReduxEffect(
  effect: ReduxEffect,
  type: string | string[],
  deps: DependencyList = [],
): void {
  const currentValue = useRef(null);
  const store = useStore();

  const handleChange = (): void => {
    const state: any = store.getState();
    const action = state.action;
    const previousValue = currentValue.current;
    currentValue.current = action.count;

    if (
      previousValue !== action.count &&
      castArray(type).includes(action.type)
    ) {
      effect(action);
    }
  }

  useEffect(() => {
    const unsubscribe = store.subscribe(handleChange);
    return (): void => unsubscribe();
  }, deps)
}
