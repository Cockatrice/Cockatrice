import { useEffect, useState } from 'react';

import type { CreateTokenSubmit } from './CreateTokenDialog';

export interface CreateTokenDialogState {
  name: string;
  color: string;
  pt: string;
  annotation: string;
  destroyOnZoneChange: boolean;
  faceDown: boolean;
  error: string | null;
  handleNameChange: (value: string) => void;
  setColor: (value: string) => void;
  setPT: (value: string) => void;
  setAnnotation: (value: string) => void;
  setDestroyOnZoneChange: (value: boolean) => void;
  setFaceDown: (value: boolean) => void;
  handleSubmit: (e?: React.FormEvent<HTMLFormElement>) => void;
}

export const CREATE_TOKEN_DEFAULT_COLOR = 'w';

// Desktop server-side MAX_NAME_LENGTH is 0xff (255). Client-side caps on
// the other free-text fields mirror that to prevent oversize-payload
// round-trips that the server would reject anyway.
export const MAX_NAME_LEN = 255;
export const MAX_PT_LEN = 255;
export const MAX_ANNOTATION_LEN = 255;

export interface UseCreateTokenDialogArgs {
  isOpen: boolean;
  onSubmit: (args: CreateTokenSubmit) => void;
}

export function useCreateTokenDialog({
  isOpen,
  onSubmit,
}: UseCreateTokenDialogArgs): CreateTokenDialogState {
  const [name, setName] = useState('');
  const [color, setColor] = useState(CREATE_TOKEN_DEFAULT_COLOR);
  const [pt, setPT] = useState('');
  const [annotation, setAnnotation] = useState('');
  const [destroyOnZoneChange, setDestroyOnZoneChange] = useState(true);
  const [faceDown, setFaceDown] = useState(false);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    if (isOpen) {
      setName('');
      setColor(CREATE_TOKEN_DEFAULT_COLOR);
      setPT('');
      setAnnotation('');
      setDestroyOnZoneChange(true);
      setFaceDown(false);
      setError(null);
    }
  }, [isOpen]);

  const handleNameChange = (value: string) => {
    setName(value.slice(0, MAX_NAME_LEN));
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
    onSubmit({
      name: name.trim(),
      color,
      pt: pt.trim(),
      annotation: annotation.trim(),
      destroyOnZoneChange,
      faceDown,
    });
  };

  return {
    name,
    color,
    pt,
    annotation,
    destroyOnZoneChange,
    faceDown,
    error,
    handleNameChange,
    setColor,
    setPT,
    setAnnotation,
    setDestroyOnZoneChange,
    setFaceDown,
    handleSubmit,
  };
}
