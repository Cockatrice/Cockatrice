
import LeftNav from './LeftNav';

import './Layout.css'

function Layout(props:LayoutProps) {
  const { children, className, showNav = true } = props;
  return (
    <div className="layout">
      {showNav && <LeftNav />}
      <section className="page">
        <div className="page__body">
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
}

export default Layout;
