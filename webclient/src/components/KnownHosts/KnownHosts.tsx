import { useCallback, useEffect, useState } from 'react';
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

import { AuthenticationService } from 'api';
import { KnownHostDialog } from 'dialogs';
import { useReduxEffect } from 'hooks';
import { HostDTO } from 'services';
import { ServerTypes } from 'store';
import { DefaultHosts, Host, getHostPort } from 'types';
import Toast from 'components/Toast/Toast';

import './KnownHosts.css';

enum TestConnection {
  TESTING = 'testing',
  FAILED = 'failed',
  SUCCESS = 'success',
}

const PREFIX = 'KnownHosts';

const classes = {
  root: `${PREFIX}-root`
};

const Root = styled('div')(({ theme }) => ({
  [`&.${classes.root}`]: {
    '& .KnownHosts-error': {
      color: theme.palette.error.main
    },

    '& .KnownHosts-warning': {
      color: theme.palette.warning.main
    },

    '& .KnownHosts-item': {
      [`& .${TestConnection.TESTING}`]: {
        color: theme.palette.warning.main
      },
      [`& .${TestConnection.FAILED}`]: {
        color: theme.palette.error.main
      },
      [`& .${TestConnection.SUCCESS}`]: {
        color: theme.palette.success.main
      }
    }
  }
}));

const KnownHosts = (props) => {
  const { input: { onChange }, meta, disabled } = props;
  const { touched, error, warning } = meta;

  const { t } = useTranslation();

  const [hostsState, setHostsState] = useState({
    hosts: [],
    selectedHost: {} as any,
  });

  const [dialogState, setDialogState] = useState({
    open: false,
    edit: null,
  });

  const [testingConnection, setTestingConnection] = useState<TestConnection>(null);

  const [showCreateToast, setShowCreateToast] = useState(false);
  const [showDeleteToast, setShowDeleteToast] = useState(false);
  const [showEditToast, setShowEditToast] = useState(false);

  const loadKnownHosts = useCallback(async () => {
    const hosts = await HostDTO.getAll();

    if (!hosts?.length) {
      // @TODO: find a better pattern to seeding default data in indexedDB
      await HostDTO.bulkAdd(DefaultHosts);
      loadKnownHosts();
    } else {
      const selectedHost = hosts.find(({ lastSelected }) => lastSelected) || hosts[0];
      setHostsState(s => ({ ...s, hosts, selectedHost }));
    }
  }, []);

  useEffect(() => {
    loadKnownHosts();
  }, [loadKnownHosts]);

  useEffect(() => {
    const { hosts, selectedHost } = hostsState;

    if (selectedHost?.id) {
      updateLastSelectedHost(selectedHost.id).then(() => {
        onChange(selectedHost);
      });
    }
  }, [hostsState, onChange]);

  useReduxEffect(() => {
    setTestingConnection(TestConnection.SUCCESS);
  }, ServerTypes.TEST_CONNECTION_SUCCESSFUL, []);

  useReduxEffect(() => {
    setTestingConnection(TestConnection.FAILED);
  }, ServerTypes.TEST_CONNECTION_FAILED, []);

  const selectHost = (selectedHost) => {
    setHostsState(s => ({ ...s, selectedHost }));
  };

  const openAddKnownHostDialog = () => {
    setDialogState(s => ({ ...s, open: true, edit: null }));
  };

  const openEditKnownHostDialog = (host: HostDTO) => {
    setDialogState(s => ({ ...s, open: true, edit: host }));
  };

  const closeKnownHostDialog = () => {
    setDialogState(s => ({ ...s, open: false }));
  }

  const handleDialogRemove = async ({ id }) => {
    setHostsState(s => ({
      ...s,
      hosts: s.hosts.filter(host => host.id !== id),
      selectedHost: s.selectedHost.id === id ? s.hosts[0] : s.selectedHost,
    }));

    closeKnownHostDialog();
    HostDTO.delete(id);
    setShowDeleteToast(true)
  };

  const handleDialogSubmit = async ({ id, name, host, port }) => {
    if (id) {
      const hostDTO = await HostDTO.get(id);
      hostDTO.name = name;
      hostDTO.host = host;
      hostDTO.port = port;
      await hostDTO.save();

      setHostsState(s => ({
        ...s,
        hosts: s.hosts.map(h => h.id === id ? hostDTO : h),
        selectedHost: hostDTO
      }));
      setShowEditToast(true)
    } else {
      const newHost: Host = { name, host, port, editable: true };
      newHost.id = await HostDTO.add(newHost) as number;

      setHostsState(s => ({
        ...s,
        hosts: [...s.hosts, newHost],
        selectedHost: newHost,
      }));
      setShowCreateToast(true)
    }

    closeKnownHostDialog();
  };

  const updateLastSelectedHost = (hostId): Promise<any[]> => {
    testConnection();

    return HostDTO.getAll().then(hosts =>
      hosts.map(async host => {
        if (host.id === hostId) {
          host.lastSelected = true;
          return await host.save();
        }

        if (host.lastSelected) {
          host.lastSelected = false;
          return await host.save();
        }

        return host;
      })
    );
  };

  const testConnection = () => {
    setTestingConnection(TestConnection.TESTING);

    const options = { ...getHostPort(hostsState.selectedHost) };
    AuthenticationService.testConnection(options);
  }

  return (
    <Root className={'KnownHosts ' + classes.root}>
      <FormControl className='KnownHosts-form' size='small' variant='outlined'>
        { touched && (
          <div className='KnownHosts-validation'>
            {
              (error &&
                <div className='KnownHosts-error'>
                  {error}
                  <ErrorOutlinedIcon style={{ fontSize: 'small', fontWeight: 'bold' }} />
                </div>
              ) ||

              (warning && <div className='KnownHosts-warning'>{warning}</div>)
            }
          </div>
        ) }

        <InputLabel id='KnownHosts-select'>{ t('KnownHosts.label') }</InputLabel>
        <Select
          id='KnownHosts-select'
          labelId='KnownHosts-label'
          label='Host'
          margin='dense'
          name='host'
          value={hostsState.selectedHost}
          fullWidth={true}
          onChange={e => selectHost(e.target.value)}
          disabled={disabled}
        >
          <Button value={hostsState.selectedHost} onClick={openAddKnownHostDialog}>
            <span>{ t('KnownHosts.add') }</span>
            <AddIcon fontSize='small' color='primary' />
          </Button>

          {
            hostsState.hosts.map((host, index) => (
              <MenuItem value={host} key={index}>
                <div className='KnownHosts-item'>
                  <div className='KnownHosts-item__wrapper'>
                    <div className={'KnownHosts-item__status ' + testingConnection}>
                      {
                        testingConnection === TestConnection.FAILED
                          ? <PortableWifiOffIcon fontSize="small" />
                          : <WifiTetheringIcon fontSize="small" />
                      }
                    </div>

                    <div className='KnownHosts-item__label'>
                      <Check />
                      <span>{host.name} ({ getHostPort(hostsState.hosts[index]).host }:{getHostPort(hostsState.hosts[index]).port})</span>
                    </div>
                  </div>

                  { host.editable && (
                    <IconButton className='KnownHosts-item__edit' size='small' color='primary' onClick={(e) => {
                      openEditKnownHostDialog(hostsState.hosts[index]);
                    }}>
                      <EditRoundedIcon fontSize='small' />
                    </IconButton>
                  ) }
                </div>
              </MenuItem>
            ))
          }
        </Select>
      </FormControl>

      <KnownHostDialog
        isOpen={dialogState.open}
        host={dialogState.edit}
        onRemove={handleDialogRemove}
        onSubmit={handleDialogSubmit}
        handleClose={closeKnownHostDialog}
      />
      <Toast open={showCreateToast} onClose={() => setShowCreateToast(false)}>{ t('KnownHosts.toast', { mode: 'created' }) }</Toast>
      <Toast open={showDeleteToast} onClose={() => setShowDeleteToast(false)}>{ t('KnownHosts.toast', { mode: 'deleted' }) }</Toast>
      <Toast open={showEditToast} onClose={() => setShowEditToast(false)}>{ t('KnownHosts.toast', { mode: 'edited' }) }</Toast>
    </Root>
  );
};

export default KnownHosts;
