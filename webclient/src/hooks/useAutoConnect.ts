import { useEffect, useState } from 'react';
import { debounce, DebouncedFunc } from 'lodash';

import { SettingDTO } from 'services';
import { APP_USER } from 'types';

type OnChange = () => void;

export function useAutoConnect(onChange: OnChange) {
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
  }, []);

  useEffect(() => {
    if (setting) {
      setAutoConnect(setting.autoConnect);
    }
  }, [setting]);

  useEffect(() => {
    if (setting) {
      setting.autoConnect = autoConnect;
      setting.save();

      onChange();
    }
  }, [setting, autoConnect]);

  return [autoConnect, setAutoConnect];
}
