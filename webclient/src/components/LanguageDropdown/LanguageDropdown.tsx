// eslint-disable-next-line
import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { Select, MenuItem } from '@mui/material';
import FormControl from '@mui/material/FormControl';
import InputLabel from '@mui/material/InputLabel';

import { Images } from 'images/Images';
import { Language, LanguageCountry, LanguageNative } from 'types';

import './LanguageDropdown.css';

const LanguageDropdown = () => {
  const { t, i18n } = useTranslation();
  const [language, setLanguage] = useState(i18n.resolvedLanguage);

  useEffect(() => {
    if (language !== i18n.resolvedLanguage) {
      i18n.changeLanguage(language);
    }
  }, [language]);

  return (
    <FormControl size='small' variant='outlined' className='LanguageDropdown'>
      <Select
        id='LanguageDropdown-select'
        margin='dense'
        value={language}
        fullWidth={true}
        onChange={e => setLanguage(e.target.value as Language)}
      >
        {
          Object.keys(Language).map((lang) => {
            const country = LanguageCountry[lang];

            return (
              <MenuItem value={lang} key={lang}>
                <div className="LanguageDropdown-item">
                  <img className="LanguageDropdown-item__image" src={Images.Countries[country]} />
                  <span className="LanguageDropdown-item__label">
                    {LanguageNative[lang]} {
                      LanguageNative[lang] !== t(`Common.languages.${lang}`) && (
                        <>({ t(`Common.languages.${lang}`) })</>
                      )
                    }
                  </span>
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
