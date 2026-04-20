import { useState } from 'react';

import { useReduxEffect } from '@app/hooks';
import { ServerTypes } from '@app/store';

export interface ResetPasswordForm {
  errorMessage: boolean;
}

export function useResetPasswordForm(): ResetPasswordForm {
  const [errorMessage, setErrorMessage] = useState(false);

  useReduxEffect(() => {
    setErrorMessage(true);
  }, ServerTypes.RESET_PASSWORD_FAILED, []);

  return { errorMessage };
}
