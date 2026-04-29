import { useEffect, useState } from 'react';

export interface PromptDialogHandle {
  value: string;
  error: string | null;
  handleChange: (v: string) => void;
  handleSubmit: (e?: React.FormEvent<HTMLFormElement>) => void;
}

export interface UsePromptDialogArgs {
  isOpen: boolean;
  initialValue: string;
  validate?: (value: string) => string | null;
  onSubmit: (value: string) => void;
}

export function usePromptDialog({
  isOpen,
  initialValue,
  validate,
  onSubmit,
}: UsePromptDialogArgs): PromptDialogHandle {
  const [value, setValue] = useState(initialValue);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    if (isOpen) {
      setValue(initialValue);
      setError(null);
    }
  }, [isOpen, initialValue]);

  const handleChange = (v: string) => {
    setValue(v);
    if (error) {
      setError(null);
    }
  };

  const handleSubmit = (e?: React.FormEvent<HTMLFormElement>) => {
    e?.preventDefault();
    if (validate) {
      const message = validate(value);
      if (message) {
        setError(message);
        return;
      }
    }
    onSubmit(value);
  };

  return { value, error, handleChange, handleSubmit };
}
