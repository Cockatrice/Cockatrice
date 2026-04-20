import Typography from '@mui/material/Typography';
import { useTranslation } from 'react-i18next';

import type { KnownHostFormValues } from '@app/forms';
import { KnownHostForm } from '@app/forms';
import type { HostDTO } from '@app/services';

import AuthDialogShell from '../AuthDialogShell/AuthDialogShell';

import './KnownHostDialog.css';

interface KnownHostDialogProps {
  isOpen: boolean;
  handleClose?: () => void;
  onRemove: (host: HostDTO) => void;
  onSubmit: (values: KnownHostFormValues) => void;
  host?: HostDTO;
}

const KnownHostDialog = ({ handleClose, onRemove, onSubmit, isOpen, host }: KnownHostDialogProps) => {
  const { t } = useTranslation();

  const mode = host ? 'edit' : 'add';

  return (
    <AuthDialogShell
      className="KnownHostDialog"
      contentClassName="dialog-content"
      isOpen={isOpen}
      handleClose={handleClose}
      title={t('KnownHostDialog.title', { mode })}
    >
      <Typography className="dialog-content__subtitle" variant="subtitle1">
        {t('KnownHostDialog.subtitle')}
      </Typography>
      <KnownHostForm onRemove={onRemove} onSubmit={onSubmit} host={host} />
    </AuthDialogShell>
  );
};

export default KnownHostDialog;
