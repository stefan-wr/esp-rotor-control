<template>
  <header id="header" class="flex-hc gap-one">
    <span
      class="led"
      :class="{
        connected: !umbrellaStore.hasLostConnection,
        disconnected: umbrellaStore.hasLostConnection
      }"
      :title="ledTitle"
    ></span>
    <h1
      v-if="!umbrellaStore.hasLostConnection"
      class="normal l-align no-wrap-ellip"
      :title="ledTitle"
    >
      RotorControl
      <span class="small">{{ fps }} FPS</span>
    </h1>
    <h1 v-else class="normal l-align blink-txt no-wrap-ellip" :title="ledTitle">
      <span class="hide-s">Keine </span>Verbindung&hellip;
    </h1>
    <nav class="flex-hc gap">
      <RouterLink to="/" class="view-tab">Controller</RouterLink>
      <RouterLink to="/settings" class="view-tab">
        <Icon icon="fa-solid fa-gear" class="show-s-only"></Icon>
        <span class="hide-s">Einstellungen</span>
      </RouterLink>
    </nav>
  </header>
</template>

<script setup>
import { computed } from 'vue';
import { useFps } from '@vueuse/core';
import { useUmbrellaStore } from '@/stores/umbrella';

const umbrellaStore = useUmbrellaStore();

var fps = useFps();

const ledTitle = computed(() => {
  if (umbrellaStore.hasLostConnection) {
    return 'Keine Verbindung zum Rotor Controller.';
  } else {
    return 'Verbindung zum Rotor Controller steht.';
  }
});
</script>

<style lang="scss" scoped>
#header {
  padding: 0.5em 2em 0.5em 1.3em;
  color: var(--header-text-color);
  background-color: var(--header-color);
  border-radius: $content-radius $content-radius 0 0;

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

  &:hover {
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
