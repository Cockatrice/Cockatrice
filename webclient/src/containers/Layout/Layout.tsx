import LeftNav from './LeftNav';

import './Layout.css'

function Layout(props:LayoutProps) {
  const { children, className, showNav = true, noHeightLimit = false } = props;
  const containerClasses = ['layout']
  if (noHeightLimit === true) {
    containerClasses.push('layout--no-height-limit')
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

function BottomBar(props) {
  return (
    <div className="bottom-bar__container">
    </div>
  )
}

interface LayoutProps {
    showNav?: boolean;
    children: any;
    className?: string;
    noHeightLimit?: boolean
}

export default Layout;
