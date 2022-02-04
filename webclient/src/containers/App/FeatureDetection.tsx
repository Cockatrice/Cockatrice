import { useEffect, useState } from 'react';
import { Redirect } from 'react-router-dom';
import { dexieService } from 'services';
import { RouteEnum } from 'types';

const FeatureDetection = () => {
  const [unsupported, setUnsupported] = useState(false);

  useEffect(() => {
    const features: Promise<any>[] = [
      detectIndexedDB(),
    ];

    Promise.all(features).catch((e) => setUnsupported(true));
  }, []);

  return unsupported
    ? <Redirect from="*" to={RouteEnum.UNSUPPORTED} />
    : <></>;

  function detectIndexedDB() {
    return dexieService.testConnection();
  }
};

export default FeatureDetection;
