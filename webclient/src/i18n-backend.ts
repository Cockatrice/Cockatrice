import { ModuleType } from 'i18next';

import { Language } from 'types';

class I18nBackend {
  static type: ModuleType = 'backend';
  static BASE_URL = `${process.env.PUBLIC_URL}/locales`;

  read(language, namespace, callback) {
    if (!Language[language]) {
      callback(true, null);
      return;
    }

    fetch(`${I18nBackend.BASE_URL}/${Language[language]}/${namespace}.json`)
      .then(resp => resp.json().then(json => callback(null, json)))
      .catch(error => callback(error, null));
  }
}

export default I18nBackend;
