<template>
  <header id="header" class="flex-hc gap-one">
    <span
      class="led"
      :class="[umbrellaStore.hasLostConnection ? 'disconnected' : 'connected']"
      :title="ledTitle"
    ></span>
    <h1
      v-if="!umbrellaStore.hasLostConnection"
      class="normal l-align no-wrap-ellip"
      :title="ledTitle"
    >
      RotorControl
      <!--span class="small" style="color:var(--accent-color);">{{ fps }} FPS</span-->
    </h1>
    <h1 v-else class="normal l-align blink-txt no-wrap-ellip" :title="ledTitle">
      <span class="hide-s">{{ $t('header.no') }}&nbsp;</span>{{ $t('header.connection') }}&hellip;
    </h1>
    <nav class="flex-hc gap" v-if="!uiStore.ui.disableHeaderLinks">
      <RouterLink to="/" class="view-tab" disabled>Controller</RouterLink>
      <RouterLink :to="{ name: uiStore.ui.activeSetting }" class="view-tab">
        <Icon icon="fa-solid fa-gear" class="show-s-only"></Icon>
        <span class="hide-s">{{ $t('commons.settings') }}</span>
      </RouterLink>
    </nav>
  </header>
</template>

<script setup>
import { computed } from 'vue';
import { useUmbrellaStore } from '@/stores/umbrella';
import { useUIStore } from '@/stores/ui';

import { useI18n } from "vue-i18n";
const { t } = useI18n();

const umbrellaStore = useUmbrellaStore();
const uiStore = useUIStore();

/** 
import { useFps } from '@vueuse/core';
var fps = useFps();
*/

const ledTitle = computed(() => {
  if (umbrellaStore.hasLostConnection) {
    return t('header.notConnectedLedDscr');
  } else {
    return t('header.connectedLedDscr');
  }
});
</script>

<style lang="scss" scoped>
#header {
  padding: 0.5em 2em 0.5em 1.3em;
  color: var(--header-text-color);
  background-color: var(--header-color);
  border-radius: $content-radius $content-radius 0 0;
  min-height: 3.2em;

  @include small {
    border-radius: 0;
    padding: 0.5em 1em 0.5em 1em;
  }

}

#header > h1 {
  flex-grow: 1;
}

.led {
  height: 10px;
  width: 10px;
  border-radius: 100px;
  flex-shrink: 0;
  margin-right: -0.3em;
  cursor: help;
  transition: background-color 0.5s, box-shadow 0.5s;

  &.disconnected {
    background-color: var(--alert-color);
    box-shadow: 0 0px 6px var(--alert-color);
  }

  &.connected {
    background-color: var(--ok-color);
    box-shadow: 0 0px 6px var(--ok-color);
  }
}

.view-tab {
  position: relative;
  padding: 0.5em 0.05em;
  opacity: 0.8;
  transition: opacity 0.1s;

  &:hover, &:focus-visible {
    opacity: 1;
  }
}

.view-tab.router-link-active {
  opacity: 1;
}

.view-tab.router-link-active::after {
  content: '';
  position: absolute;
  bottom: 0;
  left: 0;
  width: 100%;
  opacity: 0.8;
  border-bottom: 0.1em solid var(--header-text-color);
  border-radius: 0.1em;
  animation: growX 0.15s ease-out;
}

@keyframes growX {
  0% {
    transform: scaleX(0%);
  }
  100% {
    transform: scaleX(100%);
  }
}

.blink-txt {
  animation: 1s ease infinite alternate blink-txt;
}

@keyframes blink-txt {
  0% {
    color: var(--header-text-color);
  }
  100% {
    color: var(--alert-color);
  }
}

</style>
