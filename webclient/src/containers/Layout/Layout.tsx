import { ReactNode } from 'react';

import LeftNav from './LeftNav';

import './Layout.css'

interface LayoutProps {
  showNav?: boolean;
  children: ReactNode;
  className?: string;
  noHeightLimit?: boolean;
}

function Layout(props: LayoutProps) {
  const { children, className, showNav = true, noHeightLimit = false } = props;
  const containerClasses = ['layout'];
  if (noHeightLimit) {
    containerClasses.push('layout--no-height-limit');
  }

  return (
    <div className={containerClasses.join(' ')}>
      {showNav && <LeftNav />}
      <section className="page">
        <div className={`page__body ${className}`}>
          {children}
        </div>
        {showNav && <BottomBar />}
      </section>
    </div>
  )
}

function BottomBar() {
  return (
    <div className="bottom-bar__container">
    </div>
  )
}

export default Layout;
