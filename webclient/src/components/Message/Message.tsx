// eslint-disable-next-line
import React, { useEffect, useMemo, useState } from 'react';

import CardCallout from './CardCallout';
import './Message.css';

// eslint-disable-next-line
const urlRegex = /((?:https?:\/\/.)?(?:www\.)?[-a-zA-Z0-9@:%._\+~#=]{2,256}\.[a-z]{2,6}\b(?:[-a-zA-Z0-9@:%_\+.~#?&//=]*))/g;
const nameRegex = /^[^:]+:/;
const cardCalloutsRegex = /(\[\[[^\]]+\]\])/g;
const calloutsRegex = /(\[\[|\]\])/g;

const Message = (({ message: { message, messageType, timeOf, timeReceived } }) => (
  <div className='message'>
    <div className='message__detail'>
      <ParsedMessage message={message} />
    </div>
  </div>
) )


const ParsedMessage = ({ message }) => {
  const name = message.match(nameRegex);
  message = message.replace(nameRegex, '');

  const messageChunks = message
    .split(cardCalloutsRegex)
    .filter(chunk => !!chunk)
    .map(parseChunks);

  return <div>
    <strong>{name}</strong>
    { messageChunks }
  </div>

  function parseChunks(chunk, index) {
    if (chunk.match(cardCalloutsRegex)) {
      return ( <CardCallout name={chunk.replace(calloutsRegex, '')} key={index}></CardCallout> );
    }

    if (chunk.match(urlRegex)) {
      return parseUrlChunk(chunk);
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
};

export default Message;
