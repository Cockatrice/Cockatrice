import { createContext, FC, PropsWithChildren, ReactChild, ReactNode, useContext, useEffect, useReducer, Context } from 'react'

import { ACTIONS, initialState, reducer } from './reducer';
import Toast from './Toast'

interface ToastEntry {
  isOpen: boolean,
  children: ReactChild,
}

interface ToastState {
    toasts: Record<string, ToastEntry>,
    addToast: (key, children) => void,
    openToast: (key) => void,
    closeToast: (key) => void,
    removeToast: (key) => void,
}

const ToastContext: Context<any> = createContext<ToastState>({
  toasts: {},
  addToast: (_key, _children) => {},
  openToast: (_key) => {},
  closeToast: (_key) => {},
  removeToast: (_key) => {},
});

export const ToastProvider: FC<PropsWithChildren> = (props) => {
  const { children } = props
  const [state, dispatch] = useReducer(reducer, initialState)
  const providerState = {
    toasts: state.toasts,
    addToast: (key, children) => dispatch({ type: ACTIONS.ADD_TOAST, payload: { key, children } }),
    openToast: key => dispatch({ type: ACTIONS.OPEN_TOAST, payload: { key } }),
    closeToast: key => dispatch({ type: ACTIONS.CLOSE_TOAST, payload: { key } }),
    removeToast: key => dispatch({ type: ACTIONS.REMOVE_TOAST, payload: { key } }),
  }
  return (
    <ToastContext.Provider value={providerState}>
      {children}
      <div>
        {Object.entries(state.toasts).map(([key, value]: [string, ToastEntry]) => {
          const { isOpen, children } = value;
          return (
            <Toast key={key} open={isOpen} onClose={() => dispatch({ type: ACTIONS.CLOSE_TOAST, payload: { key } })}>
              {children}
            </Toast>
          )
        })}
      </div>
    </ToastContext.Provider>
  )
}

export interface ToastHookOptions {
  key: string,
  children: ReactNode
}

export function useToast({ key, children }) {
  const { addToast, openToast, closeToast, removeToast } = useContext(ToastContext)

  useEffect(() => {
    addToast(key, children)
  }, [])

  return {
    openToast: () => openToast(key),
    closeToast: () => closeToast(key),
    removeToast: () => removeToast(key),
  }
}
