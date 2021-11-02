// eslint-disable-next-line
import React, { useEffect, useState } from 'react';
import { Select, MenuItem } from '@material-ui/core';
import Button from '@material-ui/core/Button';
import FormControl from '@material-ui/core/FormControl';
import IconButton from '@material-ui/core/IconButton';
import InputLabel from '@material-ui/core/InputLabel';
import { makeStyles } from '@material-ui/core/styles';
import Check from '@material-ui/icons/Check';
import EditRoundedIcon from '@material-ui/icons/Edit';
import ErrorOutlinedIcon from '@material-ui/icons/ErrorOutlined';

import { HostDTO } from 'services';
import { DefaultHosts, getHostPort } from 'types';

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

const KnownHosts = ({ input: { onChange }, meta: { touched, error, warning } }) => {
  // this feels messy
  const classes = useStyles();

  const [state, setState] = useState({
    hosts: [],
    selectedHost: 0,
  });

  useEffect(() => {
    console.log('mount');
    HostDTO.getAll().then(async hosts => {
      console.log('setState');
      if (hosts?.length) {
        let selectedHost = hosts.findIndex(({ lastSelected }) => lastSelected);
        selectedHost = selectedHost === -1 ? 0 : selectedHost;
        setState(s => ({ ...s, hosts, selectedHost }));
      } else {
        setState(s => ({ ...s, hosts: DefaultHosts }));
        await HostDTO.bulkAdd(DefaultHosts);
      }
    });
  }, []);

  useEffect(() => {
    console.log('state changed', state);
    if (state.hosts.length && state.hosts[state.selectedHost]) {
      updateLastSelectedHost(state.hosts[state.selectedHost].id).then(() => {
        console.log('updateLastSelectedHost');
        onChange(state.hosts[state.selectedHost]);
      });
    }
  }, [state, onChange]);

  const selectHost = (selectedHost) => {
    setState(s => ({ ...s, selectedHost }));
  };

  const addKnownHost = () => {
    console.log('KnownHosts->addKnownHost');
  };

  const editKnownHost = (hostIndex) => {
    console.log('KnownHosts->editKnownHost: ', state.hosts[hostIndex]);
  };

  const updateLastSelectedHost = (hostId): Promise<any[]> => {
    return HostDTO.getAll().then(hosts =>
      hosts.map(async host => {
        if (host.id === hostId) {
          host.lastSelected = true;
          return await host.save();
        } else if (host.lastSelected) {
          host.lastSelected = false;
          return await host.save();
        }

        return host;
      })
    );
  };

  return (
    <FormControl variant='outlined' className={'KnownHosts ' + classes.root}>
      { touched && (
        <div className="KnownHosts-validation">
          {
            ( error &&
              <div className="KnownHosts-error">
                {error}
                <ErrorOutlinedIcon style={{ fontSize: 'small', fontWeight: 'bold' }} />
              </div>
            ) ||

            ( warning && <div className="KnownHosts-warning">{warning}</div> )
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
        value={state.selectedHost}
        fullWidth={true}
        onChange={e => selectHost(e.target.value)}
      >
        <Button value={state.selectedHost} onClick={addKnownHost}>Add</Button>

        {
          state.hosts.map((host, index) => (
            <MenuItem className='KnownHosts-item' value={index} key={index}>
              <div className='KnownHosts-item__label'>
                <Check />
                <span>{host.name} ({ getHostPort(state.hosts[index]).host }:{getHostPort(state.hosts[index]).port})</span>
              </div>

              { host.editable && (
                <IconButton size='small' color='primary' onClick={() => editKnownHost(index)}>
                  <EditRoundedIcon fontSize='small' />
                </IconButton>
              ) }
            </MenuItem>
          ))
        }
      </Select>
    </FormControl>
  )
};

export default KnownHosts;
