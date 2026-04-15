import { ModuleType } from 'i18next';

import { App } from '@app/types';

class I18nBackend {
  static type: ModuleType = 'backend';
  static BASE_URL = `${import.meta.env.BASE_URL}locales`;

  read(language, namespace, callback) {
    if (!language[App.Language as unknown as string]) {
      callback(true, null);
      return;
    }

    fetch(`${I18nBackend.BASE_URL}/${language[App.Language as unknown as string]}/${namespace}.json`)
      .then(resp => resp.json().then(json => callback(null, json)))
      .catch(error => callback(error, null));
  }
}

export default I18nBackend;
