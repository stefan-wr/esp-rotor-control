<template>
    <div class="flex-vst gap-one">
        <h2 class="medium hide-m flex-self-start">{{ $t('settings.calibration') }}</h2>
        <ADC></ADC>
        <CalibrationCurrent></CalibrationCurrent>
        <Calibration></Calibration>
    </div>
</template>

<script setup>
import CalibrationCurrent from '@/components/settings/CalibrationCurrent.vue';
import ADC from '@/components/settings/ADC.vue';
import Calibration from '@/components/settings/Calibration.vue';
</script>
