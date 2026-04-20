import { useTranslation } from 'react-i18next';

import type { ResetPasswordFormValues } from '@app/forms';
import { ResetPasswordForm } from '@app/forms';

import AuthDialogShell from '../AuthDialogShell/AuthDialogShell';

interface ResetPasswordDialogProps {
  isOpen: boolean;
  handleClose?: () => void;
  onSubmit: (values: ResetPasswordFormValues) => void;
  userName?: string;
}

const ResetPasswordDialog = ({ handleClose, isOpen, onSubmit, userName }: ResetPasswordDialogProps) => {
  const { t } = useTranslation();

  return (
    <AuthDialogShell
      isOpen={isOpen}
      handleClose={handleClose}
      title={t('ResetPasswordDialog.title')}
    >
      <ResetPasswordForm onSubmit={onSubmit} userName={userName} />
    </AuthDialogShell>
  );
};

export default ResetPasswordDialog;
