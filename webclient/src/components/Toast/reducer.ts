export const ACTIONS = {
  ADD_TOAST: 'ADD_TOAST',
  OPEN_TOAST: 'OPEN_TOAST',
  CLOSE_TOAST: 'CLOSE_TOAST',
  REMOVE_TOAST: 'REMOVE_TOAST',
}

export const initialState = {
  toasts: new Map()
}

export function reducer(state, action) {
  const { type, payload } = action
  switch (type) {
    case ACTIONS.ADD_TOAST: {
      const newState = { ...state }
      newState.toasts = new Map(Array.from(state.toasts))
      const { toasts } = newState;
      const { key, children } = payload
      toasts.set(key, { isOpen: false, children })
      return newState
    }
    case ACTIONS.OPEN_TOAST: {
      const newState = { ...state }
      newState.toasts = new Map(Array.from(state.toasts))
      const { toasts } = newState;
      const toast = toasts.get(payload)
      toasts.set(payload, { isOpen: true, children: toast.children })
      return newState
    }
    case ACTIONS.CLOSE_TOAST: {
      const newState = { ...state }
      newState.toasts = new Map(Array.from(state.toasts))
      const { toasts } = newState;
      const toast = toasts.get(payload)
      toasts.set(payload, { isOpen: false, children: toast.children })
      return newState
    }
    case ACTIONS.REMOVE_TOAST: {
      const newState = { ...state }
      newState.toasts = new Map(Array.from(state.toasts))
      newState.toasts.delete(payload)
      return newState
    }
    default:
      throw Error('Please pick an available action')
  }
}
