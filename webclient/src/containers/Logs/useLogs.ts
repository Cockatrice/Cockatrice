import { useEffect } from 'react';

import { useWebClient } from '@app/hooks';
import { ServerDispatch, ServerSelectors, useAppSelector } from '@app/store';
import { Data } from '@app/types';

const MAXIMUM_RESULTS = 1000;

export interface Logs {
  logs: any;
  onSubmit: (fields: Data.ViewLogHistoryParams) => void;
}

export function useLogs(): Logs {
  const logs = useAppSelector((state) => ServerSelectors.getLogs(state));
  const webClient = useWebClient();

  useEffect(() => {
    return () => {
      ServerDispatch.clearLogs();
    };
  }, []);

  const trimFields = (fields: any) => {
    const result: any = {};
    for (const [key, field] of Object.entries(fields)) {
      if (typeof field === 'string') {
        const trimmed = field.trim();
        if (trimmed) {
          result[key] = trimmed;
        }
      } else {
        result[key] = field;
      }
    }
    return result;
  };

  const flattenLogLocations = (logLocations: any) => Object.keys(logLocations);

  const onSubmit = (fields: Data.ViewLogHistoryParams) => {
    const trimmedFields: any = trimFields(fields);
    const { userName, ipAddress, gameName, gameId, message, logLocation } = trimmedFields;

    const required = [userName, ipAddress, gameName, gameId, message].filter(Boolean);

    if (logLocation) {
      trimmedFields.logLocation = flattenLogLocations(logLocation);
    }

    trimmedFields.maximumResults = MAXIMUM_RESULTS;

    if (required.length) {
      webClient.request.moderator.viewLogHistory(trimmedFields);
    } else {
      // @TODO use yet-to-be-implemented banner/alert
    }
  };

  return { logs, onSubmit };
}
