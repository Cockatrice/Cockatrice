export const ACTIONS = {
  ADD_TOAST: 'ADD_TOAST',
  OPEN_TOAST: 'OPEN_TOAST',
  CLOSE_TOAST: 'CLOSE_TOAST',
  REMOVE_TOAST: 'REMOVE_TOAST',
}

export const initialState = {
  toasts: {}
}

export function reducer(state, { type, payload }) {
  const { key, children } = payload;

  switch (type) {
    case ACTIONS.ADD_TOAST: {
      return {
        ...state,
        toasts: {
          ...state.toasts,
          [key]: {
            isOpen: false,
            children,
          },
        },
      };
    }
    case ACTIONS.OPEN_TOAST: {
      return {
        ...state,
        toasts: {
          ...state.toasts,
          [key]: {
            ...state.toasts[key],
            isOpen: true,
          },
        },
      };
    }
    case ACTIONS.CLOSE_TOAST: {
      return {
        ...state,
        toasts: {
          ...state.toasts,
          [key]: {
            ...state.toasts[key],
            isOpen: false,
          },
        },
      };
    }
    case ACTIONS.REMOVE_TOAST: {
      const newState = { ...state };
      delete newState.toasts[key];

      return newState;
    }
    default:
      throw Error('Please pick an available action')
  }
}
