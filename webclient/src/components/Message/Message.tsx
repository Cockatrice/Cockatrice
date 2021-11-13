// eslint-disable-next-line
import React, { useEffect, useMemo, useState } from 'react';

import { NavLink, generatePath } from 'react-router-dom';

import {
  RouteEnum,
  URL_REGEX,
  MESSAGE_SENDER_REGEX,
  MENTION_REGEX,
  CARD_CALLOUT_REGEX,
  CALLOUT_BOUNDARY_REGEX,
} from 'types';

import CardCallout from './CardCallout';
import './Message.css';

const Message = ({ message: { message, messageType, timeOf, timeReceived } }) => (
  <div className='message'>
    <div className='message__detail'>
      <ParsedMessage message={message} />
    </div>
  </div>
);

const ParsedMessage = ({ message }) => {
  const [messageChunks, setMessageChunks] = useState(null);
  const [name, setName] = useState(null);

  useMemo(() => {
    const name = message.match(MESSAGE_SENDER_REGEX);

    if (name) {
      setName(name[1]);
    }

    setMessageChunks(parseMessage(message));
  }, [message]);

  return (
    <div>
      { name && (<strong><PlayerLink name={name} />:</strong>) }
      { messageChunks }
    </div>
  );
};

const PlayerLink = ({ name, label = name }) => (
  <NavLink className="link" to={generatePath(RouteEnum.PLAYER, { name })}>
    {label}
  </NavLink>
);

function parseMessage(message) {
  return message.replace(MESSAGE_SENDER_REGEX, '')
    .split(CARD_CALLOUT_REGEX)
    .filter(chunk => !!chunk)
    .map(parseChunks);
}

function parseChunks(chunk, index) {
  if (chunk.match(CARD_CALLOUT_REGEX)) {
    const name = chunk.replace(CALLOUT_BOUNDARY_REGEX, '').trim();
    return (<CardCallout name={name} key={index}></CardCallout>);
  }

  if (chunk.match(URL_REGEX)) {
    return parseUrlChunk(chunk);
  }

  if (chunk.match(MENTION_REGEX)) {
    return parseMentionChunk(chunk);
  }

  return chunk;
}

function parseUrlChunk(chunk) {
  return chunk.split(URL_REGEX)
    .filter(urlChunk => !!urlChunk)
    .map((urlChunk, index) => {
      if (urlChunk.match(URL_REGEX)) {
        return (<a className='link' href={urlChunk} key={index} target='_blank' rel='noopener noreferrer'>{urlChunk}</a>);
      }

      return urlChunk;
    });
}

function parseMentionChunk(chunk) {
  return chunk.split(MENTION_REGEX)
    .filter(mentionChunk => !!mentionChunk)
    .map((mentionChunk, index) => {
      const mention = mentionChunk.match(MENTION_REGEX);

      if (mention) {
        const name = mention[0].substr(1);
        return (<PlayerLink name={name} label={mention} key={index} />);
      }

      return mentionChunk;
    });
}

export default Message;
