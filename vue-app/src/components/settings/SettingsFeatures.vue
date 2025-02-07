<template>
    <div class="flex-vst gap-one">
        <h2 class="medium hide-m flex-self-start">{{ $t('settings.functions') }}</h2>
        <KeyboardShortcuts></KeyboardShortcuts>
        <UseOverlap></UseOverlap>
        <UseSmoothSpeed></UseSmoothSpeed>
        <UseScreen v-if="settingsStore.settings.hasScreen"></UseScreen>
        <ResetLock></ResetLock>
    </div>
</template>

<script setup>
import KeyboardShortcuts from '@/components/settings/KeyboardShortcuts.vue';
import UseOverlap from '@/components/settings/UseOverlap.vue';
import UseSmoothSpeed from '@/components/settings/UseSmoothSpeed.vue';
import UseScreen from '@/components/settings/UseScreen.vue';
import ResetLock from '@/components/settings/ResetLock.vue';

import { useSettingsStore } from '@/stores/settings';
const settingsStore = useSettingsStore();
</script>
