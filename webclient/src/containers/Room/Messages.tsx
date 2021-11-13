// eslint-disable-next-line
import React from "react";

import { Message } from 'components';

import './Messages.css';

const Messages = ({ messages }) => (
  <div className="messages">
    {
      messages && messages.map((message, index) => (
        <div className="message-wrapper" key={message.timeReceived}>
          <Message message={message} />
        </div>
      ))
    }
  </div>
);

export default Messages;
