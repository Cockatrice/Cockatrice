import { ReactNode, useEffect, useRef } from 'react';

interface ScrollToBottomOnChangesProps {
  content: ReactNode;
  changes: unknown;
}

const ScrollToBottomOnChanges = ({ content, changes }: ScrollToBottomOnChangesProps) => {
  const messagesEndRef = useRef<HTMLDivElement | null>(null);

  useEffect(() => {
    messagesEndRef.current?.scrollIntoView({ behavior: 'smooth' });
  }, [changes]);

  return (
    <div style={{ height: '100%' }}>
      {content}
      <div ref={messagesEndRef} />
    </div>
  );
};

export default ScrollToBottomOnChanges;
