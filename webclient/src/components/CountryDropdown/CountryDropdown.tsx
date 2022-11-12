import { useEffect, useState } from 'react';
import { Select, MenuItem } from '@mui/material';
import FormControl from '@mui/material/FormControl';
import InputLabel from '@mui/material/InputLabel';
import { useTranslation } from 'react-i18next';

import { useLocaleSort } from 'hooks';
import { Images } from 'images/Images';
import { countryCodes } from 'types';


import './CountryDropdown.css';

const CountryDropdown = ({ input: { onChange } }) => {
  const [value, setValue] = useState('');
  const { t } = useTranslation();

  useEffect(() => onChange(value), [value]);

  const translateCountry = country => t(`Common.countries.${country}`);
  const sortedCountries = useLocaleSort(countryCodes, translateCountry);

  return (
    <FormControl size='small' variant='outlined' className='CountryDropdown'>
      <InputLabel id='CountryDropdown-select'>Country</InputLabel>
      <Select
        id='CountryDropdown-select'
        labelId='CountryDropdown-label'
        label='Country'
        margin='dense'
        value={value}
        fullWidth={true}
        onChange={e => setValue(e.target.value as string)}
      >
        <MenuItem value={''} key={-1}>
          <div className="CountryDropdown-item">
            <span className="CountryDropdown-item__label">None</span>
          </div>
        </MenuItem>

        {
          sortedCountries.map((country, index:number) => (
            <MenuItem value={country} key={index}>
              <div className="CountryDropdown-item">
                <img className="CountryDropdown-item__image" src={Images.Countries[country.toLowerCase()]} />
                <span className="CountryDropdown-item__label">{translateCountry(country)}</span>
              </div>
            </MenuItem>
          ))
        }
      </Select>
    </FormControl>
  )
};

export default CountryDropdown;
