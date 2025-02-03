<template>
  <footer id="footer" class="border-box flex-cc">
    <span class="small">
      Hardware-ID: {{ settingsStore.settings.espID }} - RotorControl UI v{{ version }} - made by
      <a href="https://github.com/stefan-wr" target="_blank" rel="noopener noreferrer">Stefan&nbsp;Wraase</a>
      -
      <a href="https://github.com/stefan-wr/esp-rotor-control" target="_blank" rel="noopener noreferrer">GitHub</a>
    </span>
  </footer>
</template>

<script setup>
import { useSettingsStore } from '../stores/settings';
import { version } from '@root/package.json';

const settingsStore = useSettingsStore();
</script>

<style lang="scss" scoped>
#footer {
  padding: 0.5em 1.25em !important;
  min-height: calc($border-box-bradius * 2);
  border-radius: 100em;
  background-color: var(--footer-color);
  color: var(--footer-text-color);

  span {
    filter: brightness(120%);
  }

  a {
    text-decoration: underline;

    &:hover, &:focus-visible {
      color: var(--footer-text-color-hover);
    }
  }
}
</style>
