<template>
  <q-item clickable tag="a" target="_blank" :href="link">
    <q-item-section avatar>
      <q-icon name="people" color="purple" />
    </q-item-section>

    <q-item-section>
      <q-item-label>{{ name }}</q-item-label>
      <q-item-label caption class="justify-start items-start row text-justify"
        ><q-icon name="circle" class="q-pr-sm" :color="online ? 'green' : 'grey'" />{{
          online ? 'Online' : 'Offline'
        }}</q-item-label
      >
    </q-item-section>
  </q-item>
  <q-separator />
</template>

<script setup lang="ts">
export interface BuddyListProps {
  name: string;
  link: string;
  online: boolean;
}
withDefaults(defineProps<BuddyListProps>(), {});
</script>
