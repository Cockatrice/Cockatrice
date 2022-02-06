// eslint-disable-next-line
import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { Select, MenuItem } from '@material-ui/core';
import FormControl from '@material-ui/core/FormControl';
import InputLabel from '@material-ui/core/InputLabel';

import { Images } from 'images/Images';
import { CountryLabel, Language, LanguageCountry } from 'types';

import './LanguageDropdown.css';

const LanguageDropdown = () => {
  const { i18n } = useTranslation();
  const [language, setLanguage] = useState(i18n.resolvedLanguage);

  useEffect(() => {
    i18n.changeLanguage(language);
  }, [language]);

  return (
    <FormControl variant='outlined' className='LanguageDropdown'>
      <Select
        id='LanguageDropdown-select'
        margin='dense'
        value={language}
        fullWidth={true}
        onChange={e => setLanguage(e.target.value as Language)}
      >
        {
          Object.keys(LanguageCountry).map((_language, index: number) => {
            const country = LanguageCountry[_language];

            return (
              <MenuItem value={_language} key={index}>
                <div className="LanguageDropdown-item">
                  <img className="LanguageDropdown-item__image" src={Images.Countries[country]} alt={CountryLabel[country]} />
                  <span className="LanguageDropdown-item__label">{_language}</span>
                </div>
              </MenuItem>
            );
          })
        }
      </Select>
    </FormControl>
  )
};

export default LanguageDropdown;
