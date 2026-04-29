import { useEffect, useState } from 'react';

import type { CounterColor } from './CreateCounterDialog';

export interface CreateCounterDialogState {
  name: string;
  selectedIdx: number;
  error: string | null;
  handleNameChange: (value: string) => void;
  setSelectedIdx: (idx: number) => void;
  handleSubmit: (e?: React.FormEvent<HTMLFormElement>) => void;
}

export interface UseCreateCounterDialogArgs {
  isOpen: boolean;
  swatches: ReadonlyArray<{ color: CounterColor }>;
  onSubmit: (args: { name: string; color: CounterColor }) => void;
}

export function useCreateCounterDialog({
  isOpen,
  swatches,
  onSubmit,
}: UseCreateCounterDialogArgs): CreateCounterDialogState {
  const [name, setName] = useState('');
  const [selectedIdx, setSelectedIdx] = useState(0);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    if (isOpen) {
      setName('');
      setSelectedIdx(0);
      setError(null);
    }
  }, [isOpen]);

  const handleNameChange = (value: string) => {
    setName(value);
    if (error) {
      setError(null);
    }
  };

  const handleSubmit = (e?: React.FormEvent<HTMLFormElement>) => {
    e?.preventDefault();
    if (name.trim().length === 0) {
      setError('Name is required');
      return;
    }
    onSubmit({ name: name.trim(), color: swatches[selectedIdx].color });
  };

  return { name, selectedIdx, error, handleNameChange, setSelectedIdx, handleSubmit };
}
