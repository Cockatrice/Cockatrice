// eslint-disable-next-line
import React from "react";

import "./Messages.css";

const Messages = ({ messages }) => (
  <div className="messages">
    {
      messages && messages.map(({ message, messageType, timeOf, timeReceived }) => (
        <div className="message" key={timeReceived}>
          <div className="message__detail">{ParsedMessage(message)}</div>
        </div>
      ) )
    }
  </div>
);

const ParsedMessage = (message) => {
  const name = message.match("^[^:]+:");

  if (name && name.length) {
    message = message.slice(name[0].length, message.length);
  }

  return <div>
    <strong>{name}</strong>
    {message}
  </div>
};

export default Messages;