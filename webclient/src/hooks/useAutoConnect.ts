import { useEffect, useState } from 'react';

import { SettingDTO } from '@app/services';
import { App } from '@app/types';

export function useAutoConnect() {
  const [setting, setSetting] = useState(undefined);
  const [autoConnect, setAutoConnect] = useState(undefined);

  useEffect(() => {
    SettingDTO.get(App.APP_USER).then((setting: SettingDTO) => {
      if (!setting) {
        setting = new SettingDTO(App.APP_USER);
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
    }
  }, [setting, autoConnect]);

  return [autoConnect, setAutoConnect];
}
