// eslint-disable-next-line
import React, { useEffect, useState } from 'react';
import { Select, MenuItem } from '@material-ui/core';
import Button from '@material-ui/core/Button';
import FormControl from '@material-ui/core/FormControl';
import IconButton from '@material-ui/core/IconButton';
import InputLabel from '@material-ui/core/InputLabel';
import EditRoundedIcon from '@material-ui/icons/Edit';

import { HostDTO } from 'services';
import { DefaultHosts, getHostPort } from 'types';

import './KnownHosts.css';

const KnownHosts = ({ onChange }) => {
  const [state, setState] = useState({
    hosts: [],
    selectedHost: 0,
  });

  useEffect(() => {
    HostDTO.getAll().then(async hosts => {
      if (hosts?.length) {
        setState(s => ({ ...s, hosts }));
      } else {
        setState(s => ({ ...s, hosts: DefaultHosts }));
        await HostDTO.bulkAdd(DefaultHosts);
      }
    });
  }, []);

  useEffect(() => {
    if (state.hosts.length) {
      onChange(getHostPort(state.hosts[state.selectedHost]));
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

  return (
    <FormControl variant='outlined' className='KnownHosts'>
      <InputLabel id='KnownHosts-select'>Host</InputLabel>
      <Select
        id='KnownHosts-select'
        labelId='KnownHosts-label'
        label='Host'
        margin='dense'
        value={state.selectedHost}
        fullWidth={true}
        onChange={e => selectHost(e.target.value)}
      >
        <Button value={state.selectedHost} onClick={addKnownHost}>Add</Button>

        {
          state.hosts.map((host, index) => (
            <MenuItem className='KnownHosts-item' value={index} key={index}>
              <span>{host.name} ({ getHostPort(state.hosts[index]).host }:{getHostPort(state.hosts[index]).port})</span>
              <IconButton size='small' color='primary' disabled={!host.editable} onClick={() => editKnownHost(index)}>
                <EditRoundedIcon fontSize='small' />
              </IconButton>
            </MenuItem>
          ))
        }
      </Select>
    </FormControl>
  )
};

export default KnownHosts;
