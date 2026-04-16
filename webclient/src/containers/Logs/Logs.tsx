// eslint-disable-next-line
import React, { useEffect } from "react";

import { request } from '@app/api';
import { AuthGuard, ModGuard } from '@app/components';
import { SearchForm } from '@app/forms';
import { ServerDispatch, ServerSelectors } from '@app/store';
import { Data } from '@app/types';
import { useAppSelector } from '@app/store';

import LogResults from './LogResults';
import './Logs.css';

const Logs = () => {
  const logs = useAppSelector(state => ServerSelectors.getLogs(state));
  const MAXIMUM_RESULTS = 1000;

  useEffect(() => {
    return () => {
      ServerDispatch.clearLogs();
    };
  }, []);

  const trimFields = (fields) => {
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

  const flattenLogLocations = (logLocations) => Object.keys(logLocations);

  const onSubmit = (fields: Data.ViewLogHistoryParams) => {
    const trimmedFields: any = trimFields(fields);
    const { userName, ipAddress, gameName, gameId, message, logLocation } = trimmedFields;

    const required = [userName, ipAddress, gameName, gameId, message].filter(Boolean);

    if (logLocation) {
      trimmedFields.logLocation = flattenLogLocations(logLocation);
    }

    trimmedFields.maximumResults = MAXIMUM_RESULTS;

    if (required.length) {
      request.moderator.viewLogHistory(trimmedFields);
    } else {
      // @TODO use yet-to-be-implemented banner/alert
    }
  };

  return (
    <div className="moderator-logs overflow-scroll">
      <AuthGuard />
      <ModGuard />

      <div className="moderator-logs__form">
        <SearchForm onSubmit={onSubmit} />
      </div>

      <div className="moderator-logs__results">
        <LogResults logs={logs} />
      </div>
    </div>
  );
};

export default Logs;










