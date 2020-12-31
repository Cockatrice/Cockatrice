// eslint-disable-next-line
import React, { Component } from "react";
import { connect } from "react-redux";
import { Form, Field, InjectedFormProps, reduxForm } from "redux-form"

import Button from "@material-ui/core/Button";
import Divider from "@material-ui/core/Divider";
import Paper from "@material-ui/core/Paper";

import InputField from "AppShell/common/components/InputField/InputField";
import CheckboxField from "AppShell/common/components/CheckboxField/CheckboxField";

import "./SearchForm.css";

class SearchForm extends Component<InjectedFormProps> {
  render() {
    return (
      <Paper className="log-search">
        <Form className="log-search__form" onSubmit={this.props.handleSubmit}>
          <div className="log-search__form-item">
            <Field label="Username" name="userName" component={InputField} />
          </div>
          <div className="log-search__form-item">
            <Field label="IP Address" name="ipAddress" component={InputField} />
          </div>
          <div className="log-search__form-item">
            <Field label="Game Name" name="gameName" component={InputField} />
          </div>
          <div className="log-search__form-item">
            <Field label="GameID" name="gameId" component={InputField} />
          </div>
          <div className="log-search__form-item">
            <Field label="Message" name="message" component={InputField} />
          </div>
          <Divider />
          <div className="log-search__form-item log-location">
            <Field label="Rooms" name="logLocation.room" component={CheckboxField} />
            <Field label="Games" name="logLocation.game" component={CheckboxField} />
            <Field label="Chats" name="logLocation.chat" component={CheckboxField} />
         </div>
          <Divider />
          <div className="log-search__form-item">
            <span>Date Range: Coming Soon</span>
          </div>
          <Divider />
          <div className="log-search__form-item">
            <span>Maximum Results: 1000</span>
          </div>
          <Divider />
          <Button className="log-search__form-submit" color="primary" variant="contained" type="submit">
            Search Logs
          </Button>
        </Form>
      </Paper>  
    )
  }
}

const propsMap = {
  form: "logs"
};

const mapStateToProps = () => ({
  
});

export default connect(mapStateToProps)(reduxForm(propsMap)(SearchForm));

