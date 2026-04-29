import type { ReactNode } from 'react';

export const ACTIONS = {
  ADD_TOAST: 'ADD_TOAST',
  OPEN_TOAST: 'OPEN_TOAST',
  CLOSE_TOAST: 'CLOSE_TOAST',
  REMOVE_TOAST: 'REMOVE_TOAST',
} as const;

export interface ToastEntry {
  isOpen: boolean;
  children: ReactNode;
  // Refcount of active registrants for this key. Incremented on ADD, decremented on REMOVE.
  // Prevents two mounted callers sharing a key from stomping each other's registration.
  refs: number;
}

export interface ToastState {
  toasts: Record<string, ToastEntry>;
}

export const initialState: ToastState = {
  toasts: {},
};

export type ToastAction =
  | { type: typeof ACTIONS.ADD_TOAST; payload: { key: string; children: ReactNode } }
  | { type: typeof ACTIONS.OPEN_TOAST; payload: { key: string } }
  | { type: typeof ACTIONS.CLOSE_TOAST; payload: { key: string } }
  | { type: typeof ACTIONS.REMOVE_TOAST; payload: { key: string } };

export function reducer(state: ToastState, action: ToastAction): ToastState {
  switch (action.type) {
    case ACTIONS.ADD_TOAST: {
      const { key, children } = action.payload;
      const existing = state.toasts[key];
      return {
        ...state,
        toasts: {
          ...state.toasts,
          [key]: existing
            ? { ...existing, refs: existing.refs + 1 }
            : { isOpen: false, children, refs: 1 },
        },
      };
    }
    case ACTIONS.OPEN_TOAST: {
      const { key } = action.payload;
      const existing = state.toasts[key];
      if (!existing) {
        return state;
      }
      return {
        ...state,
        toasts: { ...state.toasts, [key]: { ...existing, isOpen: true } },
      };
    }
    case ACTIONS.CLOSE_TOAST: {
      const { key } = action.payload;
      const existing = state.toasts[key];
      if (!existing) {
        return state;
      }
      return {
        ...state,
        toasts: { ...state.toasts, [key]: { ...existing, isOpen: false } },
      };
    }
    case ACTIONS.REMOVE_TOAST: {
      const { key } = action.payload;
      const existing = state.toasts[key];
      if (!existing) {
        return state;
      }
      if (existing.refs > 1) {
        return {
          ...state,
          toasts: { ...state.toasts, [key]: { ...existing, refs: existing.refs - 1 } },
        };
      }
      const nextToasts = { ...state.toasts };
      delete nextToasts[key];
      return { ...state, toasts: nextToasts };
    }
    default:
      return state;
  }
}
