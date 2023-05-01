<template>
  <div id="settings" class="flex-vst gap-one l-align">
    <Disconnect></Disconnect>
    <Reboot></Reboot>
    <FontSize></FontSize>
    <KeyboardShortcuts></KeyboardShortcuts>
    <ADC></ADC>
    
  </div>
</template>

<script setup>
import KeyboardShortcuts from '@/components/settings/KeyboardShortcuts.vue';
import Disconnect from '@/components/settings/Disconnect.vue';
import Reboot from '@/components/settings/Reboot.vue';
import ADC from '@/components/settings/ADC.vue';
import FontSize from '@/components/settings/FontSize.vue';

import { ref } from 'vue';

import { useUmbrellaStore } from '@/stores/umbrella';
import { useUIStore } from '@/stores/ui';

const umbrellaStore = useUmbrellaStore();
const uiStore = useUIStore();

const toggle = ref(true);
</script>

<style lang="scss">
#settings {
  width: 100%;
}

.setting {
  display: flex;
  flex-direction: column;
  gap: 1em;
}

.setting-dscr {
  color: var(--text-color-darker);
}
</style>
