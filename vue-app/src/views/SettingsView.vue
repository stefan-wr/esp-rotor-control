<template>
  <main class="gap-one">
    <div id="settings-left" class="flex-vst gap-one" style="height: 100%">
      <!-- Menu -->

      <ul id="settings-menu" class="flex-vst gap-half">
        <li v-for="item in settingsMenu">
          <button
            class="menu-item flex-cst gap-half btn-std-resp txt-dark"
            :class="{ 'menu-item-active': uiStore.ui.activeSettings === item.id }"
            @click="setActiveSetting(item.id)"
          >
            <Icon :icon="'fa-solid ' + item.icon" class="menu-icon" />
            <span class="no-wrap-ellip">{{ $t(item.label) }}</span>
          </button>
        </li>
      </ul>

      <div class="flex-vst gap-one">
        <div class="small txt-dark flex-cc gap-half hide-m">
          <RouterLink to="/licenses" class="a-hover-dark">Licenses</RouterLink>
          <span>|</span>
          <a class="a-hover-dark" href="#">Support</a>
        </div>
        <Compass :full="false" class="hide-m" v-if="uiStore.ui.activeSettings === 1"></Compass>
      </div>
    </div>

    <!-- Settings -->

    <div id="settings-right">
      <div class="flex-vst gap-one" v-if="uiStore.ui.activeSettings === 0">
        <h2 class="medium hide-m">{{ $t('settings.system') }}</h2>
        <Disconnect></Disconnect>
        <Reboot></Reboot>
        <FirmwareVersion></FirmwareVersion>
        <FirmwareUpdate></FirmwareUpdate>
      </div>

      <div class="flex-vst gap-one" v-else-if="uiStore.ui.activeSettings === 1">
        <h2 class="medium hide-m">{{ $t('settings.interface') }}</h2>
        <LanguageSelector></LanguageSelector>
        <FontSize></FontSize>
        <ColorTheme></ColorTheme>
        <CompassSettings></CompassSettings>
      </div>

      <div class="flex-vst gap-one" v-else-if="uiStore.ui.activeSettings === 2">
        <h2 class="medium hide-m">{{ $t('settings.functions') }}</h2>
        <KeyboardShortcuts></KeyboardShortcuts>
        <UseOverlap></UseOverlap>
        <UseSmoothSpeed></UseSmoothSpeed>
        <UseScreen v-if="settingsStore.settings.hasScreen"></UseScreen>
        <ResetLock></ResetLock>
      </div>

      <div class="flex-vst gap-one" v-else-if="uiStore.ui.activeSettings === 3">
        <h2 class="medium hide-m">{{ $t('settings.calibration') }}</h2>
        <ADC></ADC>
        <CalibrationCurrent></CalibrationCurrent>
        <Calibration></Calibration>
      </div>

      <div class="flex-cc gap-half small txt-dark show-m-smaller" style="width: 100%; margin-top: 1rem;">
        <RouterLink to="/licenses" class="a-hover-dark">Licenses</RouterLink>
        <span>|</span>
        <a class="a-hover-dark" href="#">Support</a>
      </div>
    </div>

    <!-- Footer -->
    <Footer id="footer"></Footer>
  </main>
</template>

<script setup>
import Compass from '@/components/Compass.vue';
import KeyboardShortcuts from '@/components/settings/KeyboardShortcuts.vue';
import UseOverlap from '@/components/settings/UseOverlap.vue';
import UseSmoothSpeed from '@/components/settings/UseSmoothSpeed.vue';
import UseScreen from '@/components/settings/UseScreen.vue';
import Disconnect from '@/components/settings/Disconnect.vue';
import Reboot from '@/components/settings/Reboot.vue';
import FirmwareVersion from '@/components/settings/FirmwareVersion.vue';
import FirmwareUpdate from '@/components/settings/FirmwareUpdate.vue';
import LanguageSelector from '@/components/settings/LanguageSelector.vue';
import FontSize from '@/components/settings/FontSize.vue';
import ColorTheme from '@/components/settings/ColorTheme.vue';
import ResetLock from '@/components/settings/ResetLock.vue';
import CompassSettings from '@/components/settings/CompassSettings.vue';
import CalibrationCurrent from '@/components/settings/CalibrationCurrent.vue';
import ADC from '@/components/settings/ADC.vue';
import Calibration from '@/components/settings/Calibration.vue';
import Footer from '@/components/Footer.vue';

import { onBeforeMount } from 'vue';
import { useSettingsStore } from '@/stores/settings';
import { useUIStore } from '@/stores/ui';

const settingsStore = useSettingsStore();
const uiStore = useUIStore();

const settingsMenu = [
  {
    id: 0,
    icon: 'fa-microchip',
    label: 'settings.system'
  },
  {
    id: 1,
    icon: 'fa-display',
    label: 'settings.interface'
  },
  {
    id: 2,
    icon: 'fa-arrows-spin',
    label: 'settings.functions'
  },
  {
    id: 3,
    icon: 'fa-compass',
    label: 'settings.calibration'
  }
];

function setActiveSetting(id) {
  uiStore.ui.activeSettings = id;
}

// Re-enable header links when mounting SettingsView
onBeforeMount(() => {
  uiStore.enableHeaderLinks();
});
</script>

<style lang="scss" scoped>
main {
  display: grid;
  align-items: start;
  justify-items: stretch;
  grid-template-columns:
    [grid-start menu-start] minmax(14em, 1.25fr) [menu-end settings-start] minmax(0, 3fr)
    [settings-end grid-end];

  @include medium {
    display: flex;
    flex-direction: column;
    align-items: start;
  }

  h2 {
    align-self: flex-start;
  }
}

#settings-left {
  grid-column: menu-start / menu-end;
  justify-content: space-between;
  width: 100%;
}

#settings-right {
  grid-column: settings-start / settings-end;
  width: 100%;
}

#footer {
  grid-column: grid-start / grid-end;
  width: 100%;
}

#settings-menu {
  @include medium {
    flex-direction: row;
    align-items: center;
    justify-content: center;
    flex-wrap: wrap;
  }
}

.menu-icon {
  min-width: 1.3em;
}

.menu-item {
  width: 100%;
  text-align: left;
  padding-top: 0.7em;
  padding-bottom: 0.7em;
  background: none;
  transition: background-color 0.1s, color 0.1s;  

  @include medium {
    background-color: var(--content-color-0);
    padding: 0.25em 0.6em;
  }

  &:hover {
    background-color: var(--content-color-2);
    filter: unset;
  }

  &:active {
    transform: unset;
  }

  &:disabled {
    filter: unset;
  }

  &:hover {
    color: var(--text-color);
  }
}

.menu-item-active {
  background-color: var(--content-color-2);
  color: var(--text-color);
  cursor: default;
}
</style>
