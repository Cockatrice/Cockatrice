import { ModuleType } from 'i18next';

import { Language } from 'types';

class I18nBackend {
  static type: ModuleType = 'backend';

  read(language, namespace, callback) {
    language = Language[language] || Language.en;

    fetch(`/locales/${language}/${namespace}.json`)
      .then(resp => resp.json().then(json => callback(null, json)))
      .catch(error => callback(error, null));
  }
}

export default I18nBackend;
