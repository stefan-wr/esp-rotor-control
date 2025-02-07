<template>
  <main class="gap-one">
    <!-- Menu -->
    <div id="settings-left" class="flex-vst gap-one" style="height: 100%">
      <ul id="settings-menu" class="flex-vst gap-half">
        <li v-for="item in settingsMenu">
          <RouterLink
            class="button btn-std-resp menu-item flex-cst gap-half "
            :to="item.name"
            activeClass="menu-item-active"
          >
            <Icon :icon="'fa-solid ' + item.icon" class="menu-icon" />
            <span class="no-wrap-ellip">{{ $t(item.label) }}</span>
          </RouterLink>
        </li>
      </ul>

      <div class="flex-vst gap-one">
        <!--div class="small txt-dark flex-cc gap-half hide-m">
          <a class="a-hover-dark" href="#">Support</a>
        </div!-->
        <Compass
          v-if="uiStore.ui.activeSetting === 'ui'"
          :isInteractive="false"
          :hasFavorites="uiStore.ui.hasFavoritesDots"
          :hasCardinals="uiStore.ui.hasCardinalLabels"
          :hasDegrees="uiStore.ui.hasDegreeLabels"
          :hasCorners="false"
          :hasBox="false"
          class="hide-m"
        ></Compass>
        <Compass
          v-if="uiStore.ui.activeSetting === 'calibration'"
          :isInteractive="false"
          :hasFavorites="false"
          :hasCardinals="true"
          :hasDegrees="true"
          :hasCorners="false"
          :hasBox="false"
          class="hide-m"
        ></Compass>
      </div>
    </div>

    <!-- Settings -->
    <div id="settings-right">
      <RouterView></RouterView>
    </div>

    <!-- Footer -->
    <Footer id="footer"></Footer>
  </main>
</template>

<script setup>
import Compass from '@/components/Compass.vue';
import Footer from '@/components/Footer.vue';

import { onBeforeMount, onBeforeUpdate } from 'vue';
import { useUIStore } from '@/stores/ui';
import { RouterLink, RouterView, useRoute } from 'vue-router';

const uiStore = useUIStore();
const route = useRoute();

const settingsMenu = [
  {
    name: 'system',
    icon: 'fa-microchip',
    label: 'settings.system'
  },
  {
    name: 'ui',
    icon: 'fa-display',
    label: 'settings.interface'
  },
  {
    name: 'features',
    icon: 'fa-arrows-spin',
    label: 'settings.functions'
  },
  {
    name: 'calibration',
    icon: 'fa-compass',
    label: 'settings.calibration'
  }
];

// Re-enable header links when mounting SettingsView
onBeforeMount(() => {
  uiStore.enableHeaderLinks();
  uiStore.ui.activeSetting = route.name;
});

// Update active setting
onBeforeUpdate(() => {
  uiStore.ui.activeSetting = route.name;
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
  color: var(--text-color-darker);
  transition:
    background-color 0.1s,
    color 0.1s;

  @include medium {
    background-color: var(--content-color-0);
    padding: 0.25em 0.6em;
  }

  @include pointer {
    &:hover,
    &:focus-visible {
      background-color: var(--content-color-2);
      color: var(--text-color);
      filter: unset;
    }
  }

  &:active {
    transform: unset;
  }

  &:disabled {
    filter: unset;
  }
}

.menu-item-active {
  background-color: var(--content-color-2);
  color: var(--text-color);
  cursor: default;
}
</style>
