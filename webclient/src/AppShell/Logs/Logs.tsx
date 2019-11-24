// eslint-disable-next-line
import React, { Component } from "react";
import { connect } from "react-redux";
import { withRouter } from "react-router-dom";
import * as _ from "lodash";

import { Dispatch, Selectors, ServerStateLogs } from "store/server"

import { ModeratorService } from "AppShell/common/services";
import AuthGuard from "AppShell/common/guards/AuthGuard";
import ModGuard from "AppShell/common/guards/ModGuard";

import LogResults from "./LogResults/LogResults";
import SearchForm from "./SearchForm/SearchForm";

import "./Logs.css";

class Logs extends Component<LogsTypes> {
  MAXIMUM_RESULTS = 1000;

  constructor(props) {
    super(props);

    this.onSubmit = this.onSubmit.bind(this);
  }

  componentWillUnmount() {
    Dispatch.clearLogs();
  }

  onSubmit(fields) {
    const trimmedFields: any = this.trimFields(fields);
    
    const { userName, ipAddress, gameName, gameId, message, logLocation } = trimmedFields;

    const required = _.filter({
      userName, ipAddress, gameName, gameId, message
    }, field => field);

    if (logLocation) {
      trimmedFields.logLocation = this.flattenLogLocations(logLocation);
    }

    trimmedFields.maximumResults = this.MAXIMUM_RESULTS;

    if (_.size(required)) {
      ModeratorService.viewLogHistory(trimmedFields);
    } else {
      // @TODO use yet-to-be-implemented banner/alert
    }
  }

  private trimFields(fields) {
    return _.reduce(fields, (obj, field, key) => {
      if (typeof field === "string") {
        const trimmed = _.trim(field);

        if (!!trimmed) {
          obj[key] = trimmed;
        }
      } else {
        obj[key] = field;
      }

      return obj;
    }, {});
  }

  private flattenLogLocations(logLocations) {
    return _.reduce(logLocations, (arr, loc, key) => {
      arr.push(key);
      return arr;
    }, [])
  }

  render() {
    return (
      <div className="moderator-logs overflow-scroll">
        <AuthGuard />
        <ModGuard />
        
        <div className="moderator-logs__form">
          <SearchForm onSubmit={this.onSubmit} />
        </div>

        <div className="moderator-logs__results">
          <LogResults logs={this.props.logs} />
        </div>
      </div>  
    )
  }
}

interface LogsTypes {
  logs: ServerStateLogs
}

const mapStateToProps = state => ({
  logs: Selectors.getLogs(state)
});

export default withRouter(connect(mapStateToProps)(Logs));
