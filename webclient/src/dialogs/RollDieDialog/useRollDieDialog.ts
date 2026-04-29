import { useEffect, useState } from 'react';

export interface RollDieDialogState {
  sides: string;
  count: string;
  error: { field: 'sides' | 'count'; message: string } | null;
  handleSidesChange: (value: string) => void;
  handleCountChange: (value: string) => void;
  handleSubmit: (e?: React.FormEvent<HTMLFormElement>) => void;
}

export interface UseRollDieDialogArgs {
  isOpen: boolean;
  lastSides: number;
  lastCount: number;
  onSubmit: (args: { sides: number; count: number }) => void;
}

export function useRollDieDialog({
  isOpen,
  lastSides,
  lastCount,
  onSubmit,
}: UseRollDieDialogArgs): RollDieDialogState {
  const [sides, setSides] = useState(String(lastSides));
  const [count, setCount] = useState(String(lastCount));
  const [error, setError] = useState<{ field: 'sides' | 'count'; message: string } | null>(null);

  useEffect(() => {
    if (isOpen) {
      setSides(String(lastSides));
      setCount(String(lastCount));
      setError(null);
    }
  }, [isOpen, lastSides, lastCount]);

  const handleSidesChange = (value: string) => {
    setSides(value);
    if (error) {
      setError(null);
    }
  };

  const handleCountChange = (value: string) => {
    setCount(value);
    if (error) {
      setError(null);
    }
  };

  const handleSubmit = (e?: React.FormEvent<HTMLFormElement>) => {
    e?.preventDefault();
    const s = Number(sides);
    if (!Number.isInteger(s) || s < 1) {
      setError({ field: 'sides', message: 'Enter an integer ≥ 1' });
      return;
    }
    const c = Number(count);
    if (!Number.isInteger(c) || c < 1) {
      setError({ field: 'count', message: 'Enter an integer ≥ 1' });
      return;
    }
    onSubmit({ sides: s, count: c });
  };

  return { sides, count, error, handleSidesChange, handleCountChange, handleSubmit };
}
