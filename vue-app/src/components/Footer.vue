<template>
  <footer id="footer" class="border-box flex-cc">
    <span class="small">
      Hardware-ID: {{ settingsStore.settings.esp_id }} -
      RotorControl UI v0.9.0 - made by
      <a href="https://github.com/stefan-wr">Stefan Wraase</a>
    </span>
  </footer>
</template>

<script setup>
import { useSettingsStore } from '../stores/settings';
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

    &:hover {
      color: var(--footer-text-color-hover);
    }
  }
}
</style>
