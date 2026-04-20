import { Select, MenuItem } from '@mui/material';
import FormControl from '@mui/material/FormControl';
import InputLabel from '@mui/material/InputLabel';
import { useTranslation } from 'react-i18next';

import { useLocaleSort } from '@app/hooks';
import { Images } from '@app/images';
import { App } from '@app/types';

import type { FinalFormFieldProps } from '../fieldTypes';

import './CountryDropdown.css';

type CountryDropdownProps = FinalFormFieldProps<string, HTMLElement>;

const CountryDropdown = ({ input }: CountryDropdownProps) => {
  const { t } = useTranslation();
  const currentValue = (input.value as string | undefined) ?? '';

  const translateCountry = (country: string) => t(`Common.countries.${country}`);
  const sortedCountries = useLocaleSort(App.countryCodes, translateCountry);

  return (
    <FormControl size="small" variant="outlined" className="CountryDropdown">
      <InputLabel id="CountryDropdown-label">Country</InputLabel>
      <Select
        id="CountryDropdown-select"
        labelId="CountryDropdown-label"
        label="Country"
        margin="dense"
        fullWidth
        {...input}
        value={currentValue}
      >
        <MenuItem value="" key="none">
          <div className="CountryDropdown-item">
            <span className="CountryDropdown-item__label">None</span>
          </div>
        </MenuItem>

        {sortedCountries.map(country => (
          <MenuItem value={country} key={country}>
            <div className="CountryDropdown-item">
              <img className="CountryDropdown-item__image" src={Images.Countries[country.toLowerCase()]} />
              <span className="CountryDropdown-item__label">{translateCountry(country)}</span>
            </div>
          </MenuItem>
        ))}
      </Select>
    </FormControl>
  );
};

export default CountryDropdown;
