import { useTranslation } from 'react-i18next';

import type { RegisterFormValues } from '@app/forms';
import { RegisterForm } from '@app/forms';

import AuthDialogShell from '../AuthDialogShell/AuthDialogShell';

import './RegistrationDialog.css';

interface RegistrationDialogProps {
  isOpen: boolean;
  handleClose?: () => void;
  onSubmit: (values: RegisterFormValues) => void;
}

const RegistrationDialog = ({ handleClose, isOpen, onSubmit }: RegistrationDialogProps) => {
  const { t } = useTranslation();

  return (
    <AuthDialogShell
      className="RegistrationDialog"
      contentClassName="dialog-content"
      isOpen={isOpen}
      handleClose={handleClose}
      title={t('RegistrationDialog.title')}
      maxWidth="xl"
    >
      <RegisterForm onSubmit={onSubmit} />
    </AuthDialogShell>
  );
};

export default RegistrationDialog;
