import React, { useEffect, useRef } from 'react';

const ScrollToBottomOnChanges = ({ content, changes }) => {
  const messagesEndRef = useRef(null);

  // @TODO improve this to not scroll down
  // when the user has scrolled up

  const scrollToBottom = () => {
    console.log('scrolling');
    messagesEndRef.current.scrollIntoView({ behavior: "smooth" })
  }

  useEffect(scrollToBottom, [changes]);

  return (
    <div>
      {content}
      <div ref={messagesEndRef} />
    </div>
  )
}

export default ScrollToBottomOnChanges;