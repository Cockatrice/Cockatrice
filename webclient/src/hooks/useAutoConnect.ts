import { Dispatch, SetStateAction, useEffect, useRef, useState } from 'react';

import { SettingDTO } from '@app/services';
import { App } from '@app/types';

export function useAutoConnect(): [boolean | undefined, Dispatch<SetStateAction<boolean | undefined>>] {
  const [setting, setSetting] = useState<SettingDTO | undefined>(undefined);
  const [autoConnect, setAutoConnect] = useState<boolean | undefined>(undefined);
  const prevAutoConnectRef = useRef<boolean | undefined>(undefined);

  useEffect(() => {
    SettingDTO.get(App.APP_USER).then((loaded: SettingDTO) => {
      if (!loaded) {
        loaded = new SettingDTO(App.APP_USER);
        loaded.save();
      }

      setSetting(loaded);
      setAutoConnect(loaded.autoConnect);
      prevAutoConnectRef.current = loaded.autoConnect;
    });
  }, []);

  useEffect(() => {
    if (setting && autoConnect !== prevAutoConnectRef.current) {
      prevAutoConnectRef.current = autoConnect;
      setting.autoConnect = autoConnect;
      setting.save();
    }
  }, [autoConnect]);

  return [autoConnect, setAutoConnect];
}
