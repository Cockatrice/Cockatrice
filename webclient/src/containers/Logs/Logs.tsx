// eslint-disable-next-line
import React, { useEffect } from "react";
import * as _ from 'lodash';

import { ModeratorService } from 'api';
import { AuthGuard, ModGuard } from 'components';
import { SearchForm } from 'forms';
import { ServerDispatch, ServerSelectors } from 'store';
import { LogFilters } from 'types';
import { useAppSelector } from 'store/store';

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
    return _.reduce(fields, (obj: any, field, key) => {
      if (typeof field === 'string') {
        const trimmed = _.trim(field);
        if (!!trimmed) {
          obj[key] = trimmed;
        }
      } else {
        obj[key] = field;
      }
      return obj;
    }, {});
  };

  const flattenLogLocations = (logLocations) => {
    return _.reduce(logLocations, (arr: any[], loc, key) => {
      arr.push(key);
      return arr;
    }, []);
  };

  const onSubmit = (fields: LogFilters) => {
    const trimmedFields: any = trimFields(fields);
    const { userName, ipAddress, gameName, gameId, message, logLocation } = trimmedFields;

    const required = _.filter({
      userName, ipAddress, gameName, gameId, message
    }, field => field);

    if (logLocation) {
      trimmedFields.logLocation = flattenLogLocations(logLocation);
    }

    trimmedFields.maximumResults = MAXIMUM_RESULTS;

    if (_.size(required)) {
      ModeratorService.viewLogHistory(trimmedFields);
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










