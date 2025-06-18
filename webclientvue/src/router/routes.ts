import type { RouteRecordRaw } from 'vue-router';

const routes: RouteRecordRaw[] = [
  {
    path: '/',
    component: () => import('layouts/MainLayout.vue'),
    children: [{ path: '', component: () => import('pages/IndexPage.vue') }],
  },
  {
    path: '/',
    name: '',
    component: () => import('layouts/MainLayout.vue'),
    children: [{ path: 'home', name: 'home', component: () => import('pages/HomePage.vue') }],
  },
  {
    path: '/',
    name: '',
    component: () => import('layouts/MainLayout.vue'),
    children: [{ path: 'game', name: 'game', component: () => import('pages/GamePage.vue') }],
  },

  // Always leave this as last one,
  // but you can also remove it
  {
    path: '/:catchAll(.*)*',
    component: () => import('pages/ErrorNotFound.vue'),
  },
];

export default routes;
