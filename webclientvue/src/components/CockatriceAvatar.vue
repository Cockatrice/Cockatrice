<template>
  <q-avatar :size="size" :square="square"
    ><img :src="CockatriceLogo" alt="Cockatrice Logo"
  /></q-avatar>
</template>
<script setup lang="ts">
import CockatriceLogo from 'assets/cockatrice_logo.png';
interface Props {
  size?: string;
  square?: boolean;
}
withDefaults(defineProps<Props>(), {
  size: '',
  square: false,
});
</script>
