import { useEffect, useState } from 'react';

import type { RevealCardsSubmit } from './RevealCardsDialog';

export interface RevealCardsDialogState {
  targetPlayerId: number;
  countDraft: string;
  error: string | null;
  setTargetPlayerId: (id: number) => void;
  handleCountChange: (value: string) => void;
  handleSubmit: (e?: React.FormEvent<HTMLFormElement>) => void;
}

export interface UseRevealCardsDialogArgs {
  isOpen: boolean;
  showCountInput: boolean;
  defaultCount: number;
  onSubmit: (args: RevealCardsSubmit) => void;
}

const ALL_PLAYERS = -1;

export function useRevealCardsDialog({
  isOpen,
  showCountInput,
  defaultCount,
  onSubmit,
}: UseRevealCardsDialogArgs): RevealCardsDialogState {
  const [targetPlayerId, setTargetPlayerId] = useState<number>(ALL_PLAYERS);
  const [countDraft, setCountDraft] = useState(String(defaultCount));
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    if (isOpen) {
      setTargetPlayerId(ALL_PLAYERS);
      setCountDraft(String(defaultCount));
      setError(null);
    }
  }, [isOpen, defaultCount]);

  const handleCountChange = (value: string) => {
    setCountDraft(value);
    if (error) {
      setError(null);
    }
  };

  const handleSubmit = (e?: React.FormEvent<HTMLFormElement>) => {
    e?.preventDefault();
    let topCards = -1;
    if (showCountInput) {
      const n = Number(countDraft);
      if (!Number.isInteger(n) || n < 1) {
        setError('Enter a positive integer');
        return;
      }
      topCards = n;
    }
    onSubmit({ targetPlayerId, topCards });
  };

  return {
    targetPlayerId,
    countDraft,
    error,
    setTargetPlayerId,
    handleCountChange,
    handleSubmit,
  };
}
