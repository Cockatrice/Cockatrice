import { useEffect, useState } from 'react';
import { debounce, DebouncedFunc } from 'lodash';

import { SettingDTO } from 'services';
import { APP_USER } from 'types';

export function useAutoConnect(callback: Function, deps: any[] = []) {
  const [setting, setSetting] = useState(undefined);
  const [autoConnect, setAutoConnect] = useState(undefined);

  useEffect(() => {
    SettingDTO.get(APP_USER).then((setting: SettingDTO) => {
      if (!setting) {
        setting = new SettingDTO(APP_USER);
        setting.save();
      }

      setSetting(setting);
    });
  }, deps);

  useEffect(() => {
    if (setting) {
      setAutoConnect(setting.autoConnect);
    }
  }, [setting]);

  useEffect(() => {
    if (setting) {
      setting.autoConnect = autoConnect;
      setting.save();

      callback(autoConnect);
    }
  }, [setting, autoConnect]);

  return [autoConnect, setAutoConnect];
}
