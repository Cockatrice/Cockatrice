// eslint-disable-next-line
import React from "react";

import VirtualList from "AppShell/common/components/VirtualList/VirtualList";

import "./Messages.css";

const Messages = ({ messages }) => (
  <div className="messages">
    {
      messages && (
        <VirtualList
          size={25}
          itemKey={(index, data) => `${index} - ${messages[index].message}` }
          items={ messages.map(({ message, messageType, timeOf, timeReceived }) => (
            <div className="message" key={timeReceived}>
              <div className="message__detail">{ParsedMessage(message)}</div>
            </div>
          ) ) }
        />
      )
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