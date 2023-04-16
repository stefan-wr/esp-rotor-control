<template>
  <header class="flex-vc">
    <h1 class="larger hide-s">RotorControl</h1>
    <nav class="flex-cst">
      <RouterLink to="/" class="tab">Controller</RouterLink>
      <RouterLink to="/settings" class="flex-cc tab">Einstellungen</RouterLink>
    </nav>
  </header>
</template>

<script setup>
import { ref, computed } from 'vue';
import { useRotorStore } from '@/stores/rotor';
const store = useRotorStore();
</script>

<style lang="scss" scoped>
header {
  width: 100%;
  border-bottom: 2px solid var(--content-color-0);
  padding: 0.5em 2em;
  justify-content: space-between;
  background-color: var(--content-color-0);
  border-radius: 2em 2em 0 0;
}

h1 {
  margin-right: 1em;
}

nav {
    gap: 1em;
}

.tab {
    position: relative;
    padding: 0.5em 0.05em;
    transition: opacity 0.1s;
    opacity: 0.8;
}
 .tab:hover {
  opacity: 1;
 }

 .tab.router-link-active {
  opacity: 1;
 }

.tab.router-link-active::after {
    content: "";
    position: absolute;
    bottom: 0;
    left: 0;
    width: 100%;
    border-bottom: 0.1em solid var(--text-color);
    border-radius: 0.1em;
    opacity: 0.8;
    animation: widen 0.15s ease-out;
}

@keyframes widen {
  0% { transform: scaleX(0%);}
  100% { transform: scaleX(100%);}
}
</style>
