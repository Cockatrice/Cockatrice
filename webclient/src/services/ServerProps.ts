import props from '../server-props.json';

class ServerProps {
  get REACT_APP_VERSION() {
    return props?.REACT_APP_VERSION;
  }
}

export const serverProps = new ServerProps();
