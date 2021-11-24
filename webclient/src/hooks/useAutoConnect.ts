import { useEffect, useState } from 'react';
import { debounce, DebouncedFunc } from 'lodash';

import { SettingDTO } from 'services';
import { APP_USER } from 'types';

export function useAutoConnect(callback: Function, deps: any[] = []) {
  const [state, setState] = useState(null);

  useEffect(() => {
    SettingDTO.get(APP_USER).then((setting: SettingDTO) => {
      setState(setting.autoConnect);
    });
  }, deps);

  useEffect(() => {
    callback(state);

    SettingDTO.get(APP_USER).then((setting: SettingDTO) => {
      setting.autoConnect = state;
      setting.save();
    });
  }, [state]);

  return [state, setState];
}
