<template>
  <SettingCard :title="$t('settings.compass.title')">
    <template #icon>
      <Icon icon="fa-solid fa-compass"></Icon>
    </template>

    <template #content>
      <p class="txt-dark">
        {{ $t('settings.compass.dscr') }}
      </p>
      <div class="flex-vst gap-half settings-box-list">
        <div class="border-box flex-cst gap-one">
          <p class="flex-grow" style="">{{ $t('settings.compass.degrees') }}</p>
          <ToggleSwitch
            :toggle="uiStore.ui.hasDegreeLabels"
            @toggle="uiStore.ui.hasDegreeLabels = !uiStore.ui.hasDegreeLabels"
          ></ToggleSwitch>
        </div>
        <div class="border-box flex-cst gap-one">
          <p class="flex-grow" style="">{{ $t('settings.compass.cardinals') }}</p>
          <ToggleSwitch
            :toggle="uiStore.ui.hasCardinalLabels"
            @toggle="uiStore.ui.hasCardinalLabels = !uiStore.ui.hasCardinalLabels"
          ></ToggleSwitch>
        </div>
        <div class="border-box flex-cst gap-one">
          <p class="flex-grow" style="">{{ $t('settings.compass.favorites') }}</p>
          <ToggleSwitch
            :toggle="uiStore.ui.hasFavoritesDots"
            @toggle="uiStore.ui.hasFavoritesDots = !uiStore.ui.hasFavoritesDots"
          ></ToggleSwitch>
        </div>
        <div class="border-box flex-cst gap-one">
          <p class="flex-grow" style="">{{ $t('settings.compass.box') }}</p>
          <ToggleSwitch
            :toggle="uiStore.ui.hasBox"
            @toggle="uiStore.ui.hasBox = !uiStore.ui.hasBox"
          ></ToggleSwitch>
        </div>
        <div class="border-box flex-cst gap-one">
          <p class="flex-grow">{{ $t('settings.compass.preferRequestAngle') }}</p>
          <ToggleSwitch
            :toggle="uiStore.ui.preferRequestAngle"
            @toggle="uiStore.ui.preferRequestAngle = !uiStore.ui.preferRequestAngle"
          ></ToggleSwitch>
        </div>
      </div>
    </template>
  </SettingCard>
</template>

<script setup>
import SettingCard from '@/components/settings/SettingCard.vue';
import ToggleSwitch from '@/components/ToggleSwitch.vue';
import { useUIStore } from '@/stores/ui';
const uiStore = useUIStore();
</script>

<style scoped>
.settings-box-list > div {
  padding: 0.5em 0.5em 0.5em 1em !important;
}
</style>
