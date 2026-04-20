import Typography from '@mui/material/Typography';
import { useTranslation } from 'react-i18next';

import type { AccountActivationFormValues } from '@app/forms';
import { AccountActivationForm } from '@app/forms';

import AuthDialogShell from '../AuthDialogShell/AuthDialogShell';

import './AccountActivationDialog.css';

interface AccountActivationDialogProps {
  isOpen: boolean;
  handleClose?: () => void;
  onSubmit: (values: AccountActivationFormValues) => void;
}

const AccountActivationDialog = ({ handleClose, isOpen, onSubmit }: AccountActivationDialogProps) => {
  const { t } = useTranslation();

  return (
    <AuthDialogShell
      isOpen={isOpen}
      handleClose={handleClose}
      title={t('AccountActivationDialog.title')}
    >
      <div className="content">
        <Typography variant='subtitle1'>{ t('AccountActivationDialog.subtitle1') }</Typography>
        <Typography variant='subtitle1'>{ t('AccountActivationDialog.subtitle2') }</Typography>
      </div>

      <AccountActivationForm onSubmit={onSubmit} />
    </AuthDialogShell>
  );
};

export default AccountActivationDialog;
