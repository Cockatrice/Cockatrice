import React from 'react';

export function SignIn(props) {
	return (
		<div className="Header-connect">
			<button onClick={props.connect}>Connect</button>
		</div>
	);
}