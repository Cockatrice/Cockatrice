import { useEffect, useState } from 'react';
import { styled } from '@mui/material/styles';
import { useTranslation } from 'react-i18next';
import { Select, MenuItem } from '@mui/material';
import Button from '@mui/material/Button';
import FormControl from '@mui/material/FormControl';
import IconButton from '@mui/material/IconButton';
import WifiTetheringIcon from '@mui/icons-material/WifiTethering';
import PortableWifiOffIcon from '@mui/icons-material/PortableWifiOff';
import InputLabel from '@mui/material/InputLabel';
import Check from '@mui/icons-material/Check';
import AddIcon from '@mui/icons-material/Add';
import EditRoundedIcon from '@mui/icons-material/Edit';
import ErrorOutlinedIcon from '@mui/icons-material/ErrorOutlined';

import { LoadingState, useKnownHosts, useReduxEffect, useWebClient } from '@app/hooks';
import { KnownHostDialog } from '@app/dialogs';
import { getHostPort, HostDTO } from '@app/services';
import { ServerTypes } from '@app/store';
import { App } from '@app/types';
import Toast from '../Toast/Toast';

import './KnownHosts.css';

enum TestConnection {
  TESTING = 'testing',
  FAILED = 'failed',
  SUCCESS = 'success',
}

const PREFIX = 'KnownHosts';

const classes = {
  root: `${PREFIX}-root`,
};

const Root = styled('div')(({ theme }) => ({
  [`&.${classes.root}`]: {
    '& .KnownHosts-error': {
      color: theme.palette.error.main,
    },

    '& .KnownHosts-warning': {
      color: theme.palette.warning.main,
    },

    '& .KnownHosts-item': {
      [`& .${TestConnection.TESTING}`]: {
        color: theme.palette.warning.main,
      },
      [`& .${TestConnection.FAILED}`]: {
        color: theme.palette.error.main,
      },
      [`& .${TestConnection.SUCCESS}`]: {
        color: theme.palette.success.main,
      },
    },
  },
}));
const KnownHosts = (props: any) => {
  const { input, meta, disabled } = props;
  const onChange: (value: HostDTO) => void = input.onChange;
  const { touched, error, warning } = meta;

  const { t } = useTranslation();
  const webClient = useWebClient();
  const knownHosts = useKnownHosts();

  const [dialogState, setDialogState] = useState<{ open: boolean; edit: HostDTO | null }>({
    open: false,
    edit: null,
  });

  const [testingConnection, setTestingConnection] = useState<TestConnection | null>(null);

  const [showCreateToast, setShowCreateToast] = useState(false);
  const [showDeleteToast, setShowDeleteToast] = useState(false);
  const [showEditToast, setShowEditToast] = useState(false);

  const selectedHost =
    knownHosts.status === LoadingState.READY ? knownHosts.value?.selectedHost : undefined;
  const hosts = knownHosts.status === LoadingState.READY ? knownHosts.value?.hosts ?? [] : [];

  const testConnection = (host: HostDTO) => {
    setTestingConnection(TestConnection.TESTING);
    webClient.request.authentication.testConnection({ ...getHostPort(host) });
  };

  // Mirror the store's selectedHost into the form field. Also kick off a
  // connection test so the user sees the green/red indicator on mount.
  useEffect(() => {
    if (!selectedHost) {
      return;
    }
    onChange(selectedHost);
    testConnection(selectedHost);
  }, [selectedHost]);

  useReduxEffect(
    () => {
      setTestingConnection(TestConnection.SUCCESS);
    },
    ServerTypes.TEST_CONNECTION_SUCCESSFUL,
    []
  );

  useReduxEffect(
    () => {
      setTestingConnection(TestConnection.FAILED);
    },
    ServerTypes.TEST_CONNECTION_FAILED,
    []
  );

  const onPick = async (host: HostDTO) => {
    if (knownHosts.status !== LoadingState.READY) {
      return;
    }
    onChange(host);
    await knownHosts.select(host.id!);
    testConnection(host);
  };

  const openAddKnownHostDialog = () => {
    setDialogState((s) => ({ ...s, open: true, edit: null }));
  };

  const openEditKnownHostDialog = (host: HostDTO) => {
    setDialogState((s) => ({ ...s, open: true, edit: host }));
  };

  const closeKnownHostDialog = () => {
    setDialogState((s) => ({ ...s, open: false }));
  };

  const handleDialogRemove = async ({ id }: { id: number }) => {
    if (knownHosts.status !== LoadingState.READY) {
      return;
    }
    await knownHosts.remove(id);
    closeKnownHostDialog();
    setShowDeleteToast(true);
  };

  const handleDialogSubmit = async ({
    id,
    name,
    host,
    port,
  }: {
    id?: number;
    name: string;
    host: string;
    port: string;
  }) => {
    if (knownHosts.status !== LoadingState.READY) {
      return;
    }

    if (id) {
      await knownHosts.update(id, { name, host, port });
      setShowEditToast(true);
    } else {
      const newHost: App.Host = { name, host, port, editable: true };
      await knownHosts.add(newHost);
      setShowCreateToast(true);
    }

    closeKnownHostDialog();
  };

  return (
    <Root className={'KnownHosts ' + classes.root}>
      <FormControl className="KnownHosts-form" size="small" variant="outlined">
        {touched && (
          <div className="KnownHosts-validation">
            {(error && (
              <div className="KnownHosts-error">
                {error}
                <ErrorOutlinedIcon style={{ fontSize: 'small', fontWeight: 'bold' }} />
              </div>
            )) ||
              (warning && <div className="KnownHosts-warning">{warning}</div>)}
          </div>
        )}

        <InputLabel id="KnownHosts-select">{t('KnownHosts.label')}</InputLabel>
        <Select
          id="KnownHosts-select"
          labelId="KnownHosts-label"
          label="Host"
          margin="dense"
          name="host"
          value={selectedHost ?? ''}
          fullWidth={true}
          onChange={(e) => onPick(e.target.value as unknown as HostDTO)}
          disabled={disabled}
        >
          <Button value={selectedHost} onClick={openAddKnownHostDialog}>
            <span>{t('KnownHosts.add')}</span>
            <AddIcon fontSize="small" color="primary" />
          </Button>

          {hosts.map((host, index) => {
            const hostPort = getHostPort(host);

            return (
              <MenuItem value={host as any} key={host.id ?? index}>
                <div className="KnownHosts-item">
                  <div className="KnownHosts-item__wrapper">
                    <div className={'KnownHosts-item__status ' + testingConnection}>
                      {testingConnection === TestConnection.FAILED ? (
                        <PortableWifiOffIcon fontSize="small" />
                      ) : (
                        <WifiTetheringIcon fontSize="small" />
                      )}
                    </div>

                    <div className="KnownHosts-item__label">
                      <Check />
                      <span>
                        {host.name} ({hostPort.host}:{hostPort.port})
                      </span>
                    </div>
                  </div>

                  {host.editable && (
                    <IconButton
                      className="KnownHosts-item__edit"
                      size="small"
                      color="primary"
                      onClick={() => {
                        openEditKnownHostDialog(host);
                      }}
                    >
                      <EditRoundedIcon fontSize="small" />
                    </IconButton>
                  )}
                </div>
              </MenuItem>
            );
          })}
        </Select>
      </FormControl>

      <KnownHostDialog
        isOpen={dialogState.open}
        host={dialogState.edit}
        onRemove={handleDialogRemove}
        onSubmit={handleDialogSubmit}
        handleClose={closeKnownHostDialog}
      />
      <Toast open={showCreateToast} onClose={() => setShowCreateToast(false)}>
        {t('KnownHosts.toast', { mode: 'created' })}
      </Toast>
      <Toast open={showDeleteToast} onClose={() => setShowDeleteToast(false)}>
        {t('KnownHosts.toast', { mode: 'deleted' })}
      </Toast>
      <Toast open={showEditToast} onClose={() => setShowEditToast(false)}>
        {t('KnownHosts.toast', { mode: 'edited' })}
      </Toast>
    </Root>
  );
};

export default KnownHosts;
