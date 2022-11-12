
import { Header } from 'components';

function Layout(props:LayoutProps) {
    const { children, className, showNav = true } = props;
    return (
        <div className={className}>
            <Header showNav={showNav} />
            {children}
        </div>
    )
}

interface LayoutProps {
    showNav?: boolean;
    children: any;
    className?: string;
}

export default Layout;
