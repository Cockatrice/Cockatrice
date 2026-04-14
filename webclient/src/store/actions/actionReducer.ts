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
  * Calculates the application state.
  *
  * @param state
  * @param action
  * @return {*}
  */
export const actionReducer = (
  state = initialState,
  action: UnknownAction,
): InitialState => {
  return {
    ...state,
    ...action,
    count: state.count + 1,
  }
}
