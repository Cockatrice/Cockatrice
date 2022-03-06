// eslint-disable-next-line
import React, { useEffect, useState } from 'react';
import { Select, MenuItem } from '@material-ui/core';
import FormControl from '@material-ui/core/FormControl';
import InputLabel from '@material-ui/core/InputLabel';
import { useTranslation } from 'react-i18next';

import { Images } from 'images/Images';
import { CountryCode } from 'types';

import './CountryDropdown.css';

const CountryDropdown = ({ input: { onChange } }) => {
  const [state, setState] = useState('');
  const { t } = useTranslation();

  useEffect(() => onChange(state), [state]);

  return (
    <FormControl variant='outlined' className='CountryDropdown'>
      <InputLabel id='CountryDropdown-select'>Country</InputLabel>
      <Select
        id='CountryDropdown-select'
        labelId='CountryDropdown-label'
        label='Country'
        margin='dense'
        value={state}
        fullWidth={true}
        onChange={e => setState(e.target.value as string)}
      >
        <MenuItem value={''} key={-1}>
          <div className="CountryDropdown-item">
            <span className="CountryDropdown-item__label">None</span>
          </div>
        </MenuItem>

        {
          Object.keys(CountryCode).map((country, index:number) => (
            <MenuItem value={country} key={index}>
              <div className="CountryDropdown-item">
                <img className="CountryDropdown-item__image" src={Images.Countries[country.toLowerCase()]} />
                <span className="CountryDropdown-item__label">{t(`Common.countries.${country}`)}</span>
              </div>
            </MenuItem>
          ))
        }
      </Select>
    </FormControl>
  )
};

export default CountryDropdown;
