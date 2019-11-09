// eslint-disable-next-line
import React from 'react';

import './Messages.css';

const Messages = ({ messages }) => (
	<div className="messages">
		{
			messages.map(({ message, messageType, timeOf }, i) => {
		    return (
		    	<div className="message" key={i}>
			      <div className="message__detail">{message}</div>
			    </div>
			  );
		  })
		}
	</div>
);

export default Messages;