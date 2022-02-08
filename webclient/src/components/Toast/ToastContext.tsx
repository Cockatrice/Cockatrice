import * as React from 'react'

import { ACTIONS, initialState, reducer } from './reducer';
import Toast from './Toast'

interface Toast {
  isOpen: boolean,
  children: React.ReactChild,
}

interface ToastContext {
    toasts: Toast[],
    addToast: (key, children) => void,
    openToast: (key) => void,
    closeToast: (key) => void,
    removeToast: (key) => void,
}

const ToastContext: any = React.createContext({
  toasts: new Map(),
  addToast: (key, children) => {},
  openToast: (key) => {},
  closeToast: (key) => {},
  removeToast: (key) => {},
});

const ToastProvider: React.FC<React.ReactNode> = (props) => {
  const { children } = props
  const [state, dispatch] = React.useReducer(reducer, initialState)
  const providerState = {
    toasts: state.toasts,
    addToast: (key, children) => dispatch({ type: ACTIONS.ADD_TOAST, payload: { key, children } }),
    openToast: key => dispatch({ type: ACTIONS.OPEN_TOAST, payload: key }),
    closeToast: key => dispatch({ type: ACTIONS.CLOSE_TOAST, payload: key }),
    removeToast: key => dispatch({ type: ACTIONS.REMOVE_TOAST, payload: key }),
  }
  return (
    <ToastContext.Provider value={providerState}>
      {children}
      <div>
        {Array.from(state.toasts).map(([key, value]) => {
          const { isOpen, children } = value;
          return (
            <Toast key={key} open={isOpen} onClose={() => dispatch({ type: ACTIONS.CLOSE_TOAST, payload: key })}>
              {children}
            </Toast>
          )
        })}
      </div>
    </ToastContext.Provider>
  )
}

function useToast({ key, children }) {
  const { addToast, openToast, closeToast, removeToast, toasts } = React.useContext(ToastContext)

  React.useEffect(() => {
    addToast(key, children)
  }, [])

  return {
    openToast: () => openToast(key),
    closeToast: () => closeToast(key),
    removeToast: () => removeToast(key),
  }
}

export {
  ToastProvider,
  useToast
}
