import { createContext, FC, PropsWithChildren, ReactNode, useContext, useEffect, useReducer } from 'react';

import { ACTIONS, initialState, reducer, ToastEntry } from './reducer';
import Toast from './Toast';

interface ToastContextValue {
  toasts: Record<string, ToastEntry>;
  addToast: (key: string, children: ReactNode) => void;
  openToast: (key: string) => void;
  closeToast: (key: string) => void;
  removeToast: (key: string) => void;
}

const ToastContext = createContext<ToastContextValue>({
  toasts: {},
  addToast: () => {},
  openToast: () => {},
  closeToast: () => {},
  removeToast: () => {},
});

export const ToastProvider: FC<PropsWithChildren> = ({ children }) => {
  const [state, dispatch] = useReducer(reducer, initialState);
  const providerState: ToastContextValue = {
    toasts: state.toasts,
    addToast: (key, toastChildren) => dispatch({ type: ACTIONS.ADD_TOAST, payload: { key, children: toastChildren } }),
    openToast: (key) => dispatch({ type: ACTIONS.OPEN_TOAST, payload: { key } }),
    closeToast: (key) => dispatch({ type: ACTIONS.CLOSE_TOAST, payload: { key } }),
    removeToast: (key) => dispatch({ type: ACTIONS.REMOVE_TOAST, payload: { key } }),
  };
  return (
    <ToastContext.Provider value={providerState}>
      {children}
      <div>
        {Object.entries(state.toasts).map(([key, entry]) => (
          <Toast
            key={key}
            open={entry.isOpen}
            onClose={() => dispatch({ type: ACTIONS.CLOSE_TOAST, payload: { key } })}
          >
            {entry.children}
          </Toast>
        ))}
      </div>
    </ToastContext.Provider>
  );
};

export interface ToastHookOptions {
  key: string;
  children: ReactNode;
}

export interface ToastHandle {
  openToast: () => void;
  closeToast: () => void;
  removeToast: () => void;
}

export function useToast({ key, children }: ToastHookOptions): ToastHandle {
  const { addToast, openToast, closeToast, removeToast } = useContext(ToastContext);

  // Toast children are captured at registration; re-registering every render
  // would churn provider state. Intentional mount/unmount-only effect keyed on `key`.
  useEffect(() => {
    addToast(key, children);
    return () => {
      removeToast(key);
    };
  }, [key]);

  return {
    openToast: () => openToast(key),
    closeToast: () => closeToast(key),
    removeToast: () => removeToast(key),
  };
}
