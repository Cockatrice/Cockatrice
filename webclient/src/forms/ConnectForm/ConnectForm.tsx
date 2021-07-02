// eslint-disable-next-line
import React, { useState } from "react";
import { connect } from "react-redux";
import { Form, Field, reduxForm, change} from "redux-form"

import Button from "@material-ui/core/Button";

import { InputField } from "components";
import { FormKey, KnownHost, KnownHosts } from 'types';

import "./ConnectForm.css";
import { Select, MenuItem } from "@material-ui/core";

const ConnectForm = (props) => {
  const { handleSubmit, dispatch } = props;
  const [knownHosts, setKnownHosts] = useState(KnownHost.ROOSTER);

  const handleChange = (event) => {
    setKnownHosts(event.target.value);
    dispatch(change(FormKey.CONNECT,'host', KnownHosts[event.target.value].host));
    dispatch(change(FormKey.CONNECT,'port', KnownHosts[event.target.value].port))
  };

  return (
    <Form className="connectForm" onSubmit={handleSubmit}>
      <div className="connectForm-item">
        <Select
              labelId="selectedKnownHosts-label"
              id="selectedKnownHosts-label"
              className="selectKnownHosts-items"
              value={knownHosts}
              fullWidth={true}
              onChange={handleChange}
            >
            <MenuItem value={KnownHost.ROOSTER}>{KnownHost.ROOSTER}</MenuItem>
            <MenuItem value={KnownHost.TETRARCH}>{KnownHost.TETRARCH}</MenuItem>
        </Select>
      </div>
      <div className="connectForm-item">
        <Field label="Host" name="host" component={InputField} />
      </div>
      <div className="connectForm-item">
        <Field label="Port" name="port" component={InputField} />
      </div>
      <div className="connectForm-item">
        <Field label="User" name="user" component={InputField} autoComplete="username" />
      </div>
      <div className="connectForm-item">
        <Field label="Pass" name="pass" type="password" component={InputField} autoComplete="current-password" />
      </div>
      <Button className="connectForm-submit" color="primary" variant="contained" type="submit">
        Connect
      </Button>
    </Form>
  );
}

const propsMap = {
  form: FormKey.CONNECT
};

const mapStateToProps = () => ({
  initialValues: {
    // host: "mtg.tetrarch.co/servatrice",
    // port: "443"
    host: "server.cockatrice.us",
    port: "4748"
  }
});

export default connect(mapStateToProps)(reduxForm(propsMap)(ConnectForm));
