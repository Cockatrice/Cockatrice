import { useTranslation } from 'react-i18next';
import { Select, MenuItem, SelectChangeEvent } from '@mui/material';
import FormControl from '@mui/material/FormControl';

import { Images } from '@app/images';
import { App } from '@app/types';

import './LanguageDropdown.css';

const LanguageDropdown = () => {
  const { t, i18n } = useTranslation();
  const currentLanguage = i18n.resolvedLanguage ?? i18n.language ?? '';

  const onLanguageChange = (event: SelectChangeEvent) => {
    const next = event.target.value as App.Language;
    if (next !== currentLanguage) {
      void i18n.changeLanguage(next);
    }
  };

  return (
    <FormControl size="small" variant="outlined" className="LanguageDropdown">
      <Select
        id="LanguageDropdown-select"
        margin="dense"
        value={currentLanguage}
        fullWidth
        onChange={onLanguageChange}
      >
        {Object.keys(App.Language).map((lang) => {
          const country = App.LanguageCountry[lang];
          const nativeName = App.LanguageNative[lang];
          const translatedName = t(`Common.languages.${lang}`);

          return (
            <MenuItem value={lang} key={lang}>
              <div className="LanguageDropdown-item">
                <img className="LanguageDropdown-item__image" src={Images.Countries[country]} />
                <span className="LanguageDropdown-item__label">
                  {nativeName} {nativeName !== translatedName && <>({translatedName})</>}
                </span>
              </div>
            </MenuItem>
          );
        })}
      </Select>
    </FormControl>
  );
};

export default LanguageDropdown;
