<template>
  <q-layout view="lHh Lpr lFf">
    <q-header elevated>
      <q-toolbar class="toolbar">
        <CockatriceAvatar square />
        <q-toolbar-title> Webatrice </q-toolbar-title>
        <q-btn flat dense round icon="people" aria-label="Menu" @click="toggleDrawer" />
      </q-toolbar>
    </q-header>

    <q-drawer v-model="drawerOpen" side="right" bordered elevated>
      <q-list>
        <q-item-label header class="text-h6"> Buddies </q-item-label>
        <q-separator />
        <Buddy v-for="buddy in buddyList" :key="buddy.name" v-bind="buddy" />
      </q-list>
    </q-drawer>

    <q-page-container>
      <router-view />
    </q-page-container>
  </q-layout>
</template>

<style scoped>
  .toolbar{
    min-height: 0;
    height: 5vh;
  }
</style>

<script setup lang="ts">
import { ref } from 'vue';
import Buddy, { type BuddyListProps } from 'components/BuddyList.vue';
import CockatriceAvatar from 'components/CockatriceAvatar.vue';

const buddyList: BuddyListProps[] = [
  {
    name: 'Zach',
    link: '#',
    online: true,
  },
  {
    name: 'Dustin',
    link: '#',
    online: true,
  },
  {
    name: 'Alex',
    link: '#',
    online: false,
  },
  {
    name: 'Matt',
    link: '#',
    online: false,
  },
];

const drawerOpen = ref(false);

function toggleDrawer() {
  drawerOpen.value = !drawerOpen.value;
}
</script>
