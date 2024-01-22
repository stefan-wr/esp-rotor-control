<template>
  <div id="settings" class="flex-vst gap-one">
    <h2 class="medium">System</h2>
    <Disconnect></Disconnect>
    <Reboot></Reboot>
    <Firmware></Firmware>
    <FirmwareUpdate></FirmwareUpdate>
    <h2 class="medium">Benutzeroberfläche</h2>
    <FontSize></FontSize>
    <ColorTheme></ColorTheme>
    <CompassSettings></CompassSettings>
    <KeyboardShortcuts></KeyboardShortcuts>
    <h2 class="medium">Funktionen</h2>
    <UseOverlap></UseOverlap>
    <ResetLock></ResetLock>
  </div>
</template>

<script setup>
import KeyboardShortcuts from '@/components/settings/KeyboardShortcuts.vue';
import UseOverlap from '@/components/settings/UseOverlap.vue';
import Disconnect from '@/components/settings/Disconnect.vue';
import Reboot from '@/components/settings/Reboot.vue';
import Firmware from '@/components/settings/Firmware.vue';
import FirmwareUpdate from '@/components/settings/FirmwareUpdate.vue';
import FontSize from '@/components/settings/FontSize.vue';
import ColorTheme from '@/components/settings/ColorTheme.vue';
import ResetLock from '@/components/settings/ResetLock.vue';
import CompassSettings from '@/components/settings/CompassSettings.vue';

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

  h2:not(:first-child) {
    margin-top: 1em;
  }
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
