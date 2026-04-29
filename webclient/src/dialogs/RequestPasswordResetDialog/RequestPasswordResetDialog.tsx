import { useTranslation } from 'react-i18next';

import type { RequestPasswordResetFormValues } from '@app/forms';
import { RequestPasswordResetForm } from '@app/forms';

import AuthDialogShell from '../AuthDialogShell/AuthDialogShell';

interface RequestPasswordResetDialogProps {
  isOpen: boolean;
  handleClose?: () => void;
  onSubmit: (values: RequestPasswordResetFormValues) => void;
  skipTokenRequest: (userName: string) => void;
}

const RequestPasswordResetDialog = ({
  handleClose,
  isOpen,
  onSubmit,
  skipTokenRequest,
}: RequestPasswordResetDialogProps) => {
  const { t } = useTranslation();

  return (
    <AuthDialogShell
      isOpen={isOpen}
      handleClose={handleClose}
      title={t('RequestPasswordResetDialog.title')}
    >
      <RequestPasswordResetForm onSubmit={onSubmit} skipTokenRequest={skipTokenRequest} />
    </AuthDialogShell>
  );
};

export default RequestPasswordResetDialog;
