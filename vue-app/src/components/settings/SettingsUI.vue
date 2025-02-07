<template>
  <div class="flex-vst gap-one">
    <h2 class="medium hide-m flex-self-start">{{ $t('settings.interface') }}</h2>
    <LanguageSelector></LanguageSelector>
    <FontSize></FontSize>
    <ColorTheme></ColorTheme>
    <CompassSettings></CompassSettings>
  </div>
</template>

<script setup>
import LanguageSelector from '@/components/settings/LanguageSelector.vue';
import FontSize from '@/components/settings/FontSize.vue';
import ColorTheme from '@/components/settings/ColorTheme.vue';
import CompassSettings from '@/components/settings/CompassSettings.vue';
</script>
