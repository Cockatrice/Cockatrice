import i18n from 'i18next';
import LanguageDetector from 'i18next-browser-languagedetector';
import { initReactI18next } from 'react-i18next';

import I18nBackend from './i18n-backend';

i18n
  .use(I18nBackend)
  .use(LanguageDetector)
  .use(initReactI18next)
  // for all options read: https://www.i18next.com/overview/configuration-options
  .init({
    fallbackLng: 'en-US',

    interpolation: {
      // not needed for react as it escapes by default
      escapeValue: false,
    },
  });

export default i18n;
