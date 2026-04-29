import { Message } from '@app/components';
import type { Enriched } from '@app/types';

import './Messages.css';

interface MessagesProps {
  messages?: Enriched.Message[];
}

const Messages = ({ messages }: MessagesProps) => (
  <div className="messages">
    {
      messages && messages.map((message, idx) => (
        <div className="message-wrapper" key={`${message.timeReceived}-${idx}`}>
          <Message message={message} />
        </div>
      ))
    }
  </div>
);

export default Messages;
