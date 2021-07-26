// eslint-disable-next-line
import React, { useEffect, useMemo, useState } from 'react';

import { NavLink, generatePath } from "react-router-dom";

import { RouteEnum } from 'types';

import CardCallout from './CardCallout';
import './Message.css';

// eslint-disable-next-line
const urlRegex = /((?:https?:\/\/.)?(?:www\.)?[-a-zA-Z0-9@:%._\+~#=]{2,256}\.[a-z]{2,6}\b(?:[-a-zA-Z0-9@:%_\+.~#?&//=]*))/g;
const nameRegex = /(^[^: ]+):/;
const mentionRegex = /(@[^ ]+)/g;
const cardCalloutsRegex = /(\[\[[^\]]+\]\])/g;
const calloutsRegex = /(\[\[|\]\])/g;

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
    const name = message.match(nameRegex);

    const messageChunks = message.replace(nameRegex, '')
      .split(cardCalloutsRegex)
      .filter(chunk => !!chunk)
      .map(parseChunks);

    if (name) {
      setName(name[1]);
    }

    setMessageChunks(messageChunks);
  }, [message]);


  return (
    <div>
      {
        name && ( <strong><PlayerLink name={name} />:</strong> )
      }

      { messageChunks }
    </div>
  );
};

const PlayerLink = ({ name, label = name }) => (
  <NavLink className="link" to={generatePath(RouteEnum.PLAYER, { name })}>
    {label}
  </NavLink>
);

function parseChunks(chunk, index) {
  if (chunk.match(cardCalloutsRegex)) {
    return ( <CardCallout name={chunk.replace(calloutsRegex, '')} key={index}></CardCallout> );
  }

  if (chunk.match(urlRegex)) {
    return parseUrlChunk(chunk);
  }

  if (chunk.match(mentionRegex)) {
    return parseMentionChunk(chunk);
  }

  return chunk;
}

function parseUrlChunk(chunk) {
  return chunk.split(urlRegex)
    .filter(urlChunk => !!urlChunk)
    .map((urlChunk, index) => {
      if (urlChunk.match(urlRegex)) {
        return ( <a className='link' href={urlChunk} key={index} target='_blank'rel='noopener noreferrer'>{urlChunk}</a> );
      }

      return urlChunk;
    });
}

function parseMentionChunk(chunk) {
  return chunk.split(mentionRegex)
    .filter(mentionChunk => !!mentionChunk)
    .map((mentionChunk, index) => {
      const mention = mentionChunk.match(mentionRegex);

      if (mention) {
        const name = mention[0].slice(1, mention[0].length);

        return ( <PlayerLink name={name} label={mention} key={index} /> );
      }

      return mentionChunk;
    });
}

export default Message;
