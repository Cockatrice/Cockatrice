import { useEffect, useState } from 'react';
import { debounce, DebouncedFunc } from 'lodash';

import { SettingDTO } from 'services';
import { APP_USER } from 'types';

export function useAutoConnect(callback: Function, deps: any[] = []) {
  const [autoConnect, setAutoConnect] = useState(null);

  useEffect(() => {
    SettingDTO.get(APP_USER).then((setting: SettingDTO) => {
      setAutoConnect(setting.autoConnect);
    });
  }, deps);

  useEffect(() => {
    callback(autoConnect);

    SettingDTO.get(APP_USER).then((setting: SettingDTO) => {
      setting.autoConnect = autoConnect;
      setting.save();
    });
  }, [autoConnect]);

  return [autoConnect, setAutoConnect];
}
