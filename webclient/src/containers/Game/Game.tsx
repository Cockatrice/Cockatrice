import { AuthGuard } from '@app/components';
import Layout from '../Layout/Layout';

import './Game.css';

function Game() {
  return (
    <Layout>
      <AuthGuard />
      <span>"Game"</span>
    </Layout>
  );
}

export default Game;
