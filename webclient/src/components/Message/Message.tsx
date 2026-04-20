import { NavLink, generatePath } from 'react-router-dom';
import type { ReactNode } from 'react';

import { App } from '@app/types';

import CardCallout from './CardCallout';
import { useParsedMessage } from './useMessage';
import './Message.css';

const Message = ({ message: { message } }) => (
  <div className='message'>
    <div className='message__detail'>
      <ParsedMessage message={message} />
    </div>
  </div>
);

const ParsedMessage = ({ message }) => {
  const { name, chunks } = useParsedMessage(message, parseChunks);

  return (
    <div>
      {name && (<strong><PlayerLink name={name} />:</strong>)}
      {chunks}
    </div>
  );
};

const PlayerLink = ({ name, label = name }) => (
  <NavLink className="link" to={generatePath(App.RouteEnum.PLAYER, { name })}>
    {label}
  </NavLink>
);

function parseChunks(chunk: string, index: number): ReactNode {
  if (chunk.match(App.CARD_CALLOUT_REGEX)) {
    const name = chunk.replace(App.CALLOUT_BOUNDARY_REGEX, '').trim();
    return (<CardCallout name={name} key={index}></CardCallout>);
  }

  if (chunk.match(App.URL_REGEX)) {
    return parseUrlChunk(chunk);
  }

  if (chunk.match(App.MENTION_REGEX)) {
    return parseMentionChunk(chunk);
  }

  return chunk;
}

function parseUrlChunk(chunk: string): ReactNode {
  return chunk.split(App.URL_REGEX)
    .filter((urlChunk) => !!urlChunk)
    .map((urlChunk, index) => {
      if (urlChunk.match(App.URL_REGEX)) {
        return (<a className='link' href={urlChunk} key={index} target='_blank' rel='noopener noreferrer'>{urlChunk}</a>);
      }

      return urlChunk;
    });
}

function parseMentionChunk(chunk: string): ReactNode {
  return chunk.split(App.MENTION_REGEX)
    .filter((mentionChunk) => !!mentionChunk)
    .map((mentionChunk, index) => {
      const mention = mentionChunk.match(App.MENTION_REGEX);

      if (mention) {
        const name = mention[0].substr(1);
        return (<PlayerLink name={name} label={mention} key={index} />);
      }

      return mentionChunk;
    });
}

export default Message;
