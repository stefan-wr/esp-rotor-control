<script setup>
import { RouterLink, RouterView } from 'vue-router';
import Header from '@/components/Header.vue';
</script>

<template>
  <div class="content">
    <Header />
    <RouterView />
  </div>
</template>

<style scoped>
.content {
  background-color: var(--content-color-1);
  margin: 2em;
  border-radius: 2em;
  box-shadow: 0px 2em 3em #9e9e9e;
  width: 100vw;
  max-width: 1200px;
  overflow: hiden;
}
</style>
