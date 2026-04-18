/**
 * @author Luke Brandon Farrell
 * @description Application reducer.
 */

import { UnknownAction } from '@reduxjs/toolkit'

 interface InitialState {
   type: string | null
   payload: unknown
   meta: unknown
   error: boolean
   count: number
 }

/**
  * Initial data.
  */
const initialState: InitialState = {
  type: null,
  payload: null,
  meta: null,
  error: false,
  count: 0,
}

/**
  * Stores the most recent action so `useReduxEffect` can react to dispatches.
  *
  * Payloads are deep-cloned to prevent shared object references between this
  * slice and the slice that owns the action. Without the clone, Immer mutations
  * in the target slice are detected as mutations of the stale payload stored here.
  */
export const actionReducer = (
  state = initialState,
  action: UnknownAction,
): InitialState => {
  return {
    type: action.type ?? null,
    payload: 'payload' in action ? structuredClone(action.payload) : null,
    meta: 'meta' in action ? structuredClone(action.meta) : null,
    error: !!action.error,
    count: state.count + 1,
  }
}
