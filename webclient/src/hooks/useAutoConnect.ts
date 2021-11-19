import { useEffect, useState } from "react";
import { debounce, DebouncedFunc } from "lodash";

import { SettingDTO } from 'services';
import { APP_USER } from 'types';

export function useAutoConnect(fn: Function, deps?: any[]): void {
  const [state, setState] = useState(null);

  useEffect(() => {
    SettingDTO.get(APP_USER).then((setting: SettingDTO) => {
      // setting.autoConnect = autoConnect;
      setting.save();
    });
  }, deps);

  useEffect(() => {
    fn(state);
  }, [state]);
}
