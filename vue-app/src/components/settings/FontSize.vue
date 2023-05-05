<template>
  <SettingCard :title="'Schriftgröße: ' + uiStore.ui.fontSize">
    <template #icon>
      <Icon icon="fa-solid fa-text-height"></Icon>
    </template>

    <template #action>
      <div class="flex-cst gap-half">
        <button class="btn-std-resp" title="Schrift verkleinern." @click="uiStore.fontSizeMinus()">
          <Icon icon="fa-solid fa-minus"></Icon>
        </button>
        <button class="btn-std-resp" title="Schrift vergrößen." @click="uiStore.fontSizePlus()">
          <Icon icon="fa-solid fa-plus"></Icon>
        </button>
      </div>
    </template>

    <template #content>
      <p class="txt-dark">
        Vergrößert oder verkleinert die Schriftgröße der Benutzeroberfläche.
      </p>
    </template>
  </SettingCard>
</template>

<script setup>
import SettingCard from '@/components/settings/SettingCard.vue';

import { useUIStore } from '@/stores/ui';

const uiStore = useUIStore();

</script>
