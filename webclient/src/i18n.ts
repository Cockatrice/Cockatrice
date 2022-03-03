import i18n from 'i18next';
import LanguageDetector from 'i18next-browser-languagedetector';
import ICU from 'i18next-icu';
import { initReactI18next } from 'react-i18next';

import { Language } from 'types';

import I18nBackend from './i18n-backend';

// Bundle default translation with application
import translation from './i18n-default.json';

i18n
  .use(ICU)
  .use(I18nBackend)
  .use(LanguageDetector)
  .use(initReactI18next)
  // for all options read: https://www.i18next.com/overview/configuration-options
  .init({
    fallbackLng: Language['en-US'],
    resources: {
      [Language['en-US']]: { translation },
    },
    partialBundledLanguages: true,

    interpolation: {
      // not needed for react as it escapes by default
      escapeValue: false,
    }
  });

export default i18n;
