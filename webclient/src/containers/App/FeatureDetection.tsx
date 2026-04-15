import { useEffect, useState } from 'react';
import { Navigate } from 'react-router-dom';
import { dexieService } from '@app/services';
import { App } from '@app/types';

const FeatureDetection = () => {
  const [unsupported, setUnsupported] = useState(false);

  useEffect(() => {
    const features: Promise<any>[] = [
      detectIndexedDB(),
    ];

    Promise.all(features).catch(() => setUnsupported(true));
  }, []);

  return unsupported
    ? <Navigate to={App.RouteEnum.UNSUPPORTED} />
    : <></>;

  function detectIndexedDB() {
    return dexieService.testConnection();
  }
};

export default FeatureDetection;
