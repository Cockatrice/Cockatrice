import { useState } from 'react';

import { useReduxEffect } from '@app/hooks';
import { ServerTypes } from '@app/store';

export interface RequestPasswordResetForm {
  errorMessage: boolean;
  setErrorMessage: (v: boolean) => void;
  isMFA: boolean;
  setIsMFA: (v: boolean) => void;
}

export function useRequestPasswordResetForm(): RequestPasswordResetForm {
  const [errorMessage, setErrorMessage] = useState(false);
  const [isMFA, setIsMFA] = useState(false);

  useReduxEffect(() => {
    setErrorMessage(true);
  }, ServerTypes.RESET_PASSWORD_FAILED, []);

  useReduxEffect(() => {
    setIsMFA(true);
  }, ServerTypes.RESET_PASSWORD_CHALLENGE, []);

  return { errorMessage, setErrorMessage, isMFA, setIsMFA };
}
