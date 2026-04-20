import { createContext, useCallback, useContext } from 'react';

export type CardKey = string;

export function makeCardKey(playerId: number, zone: string, cardId: number): CardKey {
  return `${playerId}-${zone}-${cardId}`;
}

export interface CardRegistry {
  register(key: CardKey, el: HTMLElement): void;
  unregister(key: CardKey): void;
  get(key: CardKey): HTMLElement | undefined;
  subscribe(listener: () => void): () => void;
}

export const CardRegistryContext = createContext<CardRegistry | null>(null);

export function useCardRegistry(): CardRegistry | null {
  return useContext(CardRegistryContext);
}

export function useRegisterCardRef(key: CardKey | null) {
  const registry = useCardRegistry();
  return useCallback(
    (el: HTMLElement | null) => {
      if (!registry || key == null) {
        return;
      }
      if (el) {
        registry.register(key, el);
      } else {
        registry.unregister(key);
      }
    },
    [registry, key],
  );
}

export function createCardRegistry(): CardRegistry {
  const map = new Map<CardKey, HTMLElement>();
  const listeners = new Set<() => void>();
  const notify = () => {
    listeners.forEach((l) => l());
  };
  return {
    register(key, el) {
      map.set(key, el);
      notify();
    },
    unregister(key) {
      map.delete(key);
      notify();
    },
    get(key) {
      return map.get(key);
    },
    subscribe(l) {
      listeners.add(l);
      return () => {
        listeners.delete(l);
      };
    },
  };
}
