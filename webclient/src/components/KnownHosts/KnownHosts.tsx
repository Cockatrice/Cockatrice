// eslint-disable-next-line
import React, { useCallback, useEffect, useState } from 'react';
import { Select, MenuItem } from '@material-ui/core';
import Button from '@material-ui/core/Button';
import FormControl from '@material-ui/core/FormControl';
import IconButton from '@material-ui/core/IconButton';
import InputLabel from '@material-ui/core/InputLabel';
import { makeStyles } from '@material-ui/core/styles';
import Check from '@material-ui/icons/Check';
import AddIcon from '@material-ui/icons/Add';
import EditRoundedIcon from '@material-ui/icons/Edit';
import ErrorOutlinedIcon from '@material-ui/icons/ErrorOutlined';

import { KnownHostDialog } from 'dialogs';
import { HostDTO } from 'services';
import { DefaultHosts, Host, getHostPort } from 'types';

import './KnownHosts.css';

const useStyles = makeStyles(theme => ({
  root: {
    '& .KnownHosts-error': {
      color: theme.palette.error.main
    },

    '& .KnownHosts-warning': {
      color: theme.palette.warning.main
    }
  },
}));

const KnownHosts = (props) => {
  const { input: { onChange }, meta, disabled } = props;
  const { touched, error, warning } = meta;
  const classes = useStyles();

  const [hostsState, setHostsState] = useState({
    hosts: [],
    selectedHost: {} as any,
  });

  const [dialogState, setDialogState] = useState({
    open: false,
    edit: null,
  });

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
    } else {
      const newHost: Host = { name, host, port, editable: true };
      newHost.id = await HostDTO.add(newHost) as number;

      setHostsState(s => ({
        ...s,
        hosts: [...s.hosts, newHost],
        selectedHost: newHost,
      }));
    }

    closeKnownHostDialog();
  };

  const updateLastSelectedHost = (hostId): Promise<any[]> => {
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

  return (
    <div>
      <FormControl variant='outlined' className={'KnownHosts ' + classes.root}>
        { touched && (
          <div className="KnownHosts-validation">
            {
              (error &&
                <div className="KnownHosts-error">
                  {error}
                  <ErrorOutlinedIcon style={{ fontSize: 'small', fontWeight: 'bold' }} />
                </div>
              ) ||

              (warning && <div className="KnownHosts-warning">{warning}</div>)
            }
          </div>
        ) }

        <InputLabel id='KnownHosts-select'>Host</InputLabel>
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
            <span>Add new host</span>
            <AddIcon fontSize='small' color='primary' />
          </Button>

          {
            hostsState.hosts.map((host, index) => (
              <MenuItem className='KnownHosts-item' value={host} key={index}>
                <div className='KnownHosts-item__label'>
                  <Check />
                  <span>{host.name} ({ getHostPort(hostsState.hosts[index]).host }:{getHostPort(hostsState.hosts[index]).port})</span>
                </div>

                { host.editable && (
                  <IconButton className='KnownHosts-item__edit' size='small' color='primary' onClick={(e) => {
                    openEditKnownHostDialog(hostsState.hosts[index]);
                  }}>
                    <EditRoundedIcon fontSize='small' />
                  </IconButton>
                ) }
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
    </div>
  )
};

export default KnownHosts;
