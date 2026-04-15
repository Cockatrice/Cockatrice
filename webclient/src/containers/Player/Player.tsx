import Layout from '../Layout/Layout';

import { AuthGuard } from '@app/components';

function Player() {
  return (
    <Layout>
      <AuthGuard />
      <span>"Player"</span>
    </Layout>
  );
}

export default Player;
