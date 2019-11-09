// eslint-disable-next-line
import React, { Component } from 'react';
import { connect } from 'react-redux';
import { Field, InjectedFormProps, reduxForm } from 'redux-form'

import './ConnectForm.css';

class ConnectForm extends Component<InjectedFormProps> {
	render() {
		return (
			<form className="connectForm" onSubmit={this.props.handleSubmit}>
				<div className="connectForm-item">
					<label className="connectForm-item__label" htmlFor="host">Host</label>	
					<Field className="connectForm-item__field" name="host" component="input" type="text" />
				</div>
				<div className="connectForm-item">
					<label className="connectForm-item__label" htmlFor="port">Port</label>	
					<Field className="connectForm-item__field" name="port" component="input" type="text" />
				</div>
				<div className="connectForm-item">
					<label className="connectForm-item__label" htmlFor="user">User</label>	
					<Field className="connectForm-item__field" name="user" component="input" type="text" />
				</div>
				<div className="connectForm-item">
					<label className="connectForm-item__label" htmlFor="pass">Password</label>	
					<Field className="connectForm-item__field" name="pass" component="input" type="password" />
				</div>
				<button className="connectForm-submit" type="submit">Connect</button>
			</form>
		);
	}
}

const propsMap = {
	form: 'connect'
};

const mapStateToProps = () => ({
	initialValues: {
		host: 'server.cockatrice.us',
		port: '4748'
	}
});

export default connect(mapStateToProps)(reduxForm(propsMap)(ConnectForm));
