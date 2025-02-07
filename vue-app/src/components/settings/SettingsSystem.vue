<template>
  <div class="flex-vst gap-one">
    <h2 class="medium hide-m flex-self-start">{{ $t('settings.system') }}</h2>
    <Disconnect></Disconnect>
    <Reboot></Reboot>
    <FirmwareVersion></FirmwareVersion>
    <FirmwareUpdate></FirmwareUpdate>
  </div>
</template>

<script setup>
import Disconnect from '@/components/settings/Disconnect.vue';
import Reboot from '@/components/settings/Reboot.vue';
import FirmwareVersion from '@/components/settings/FirmwareVersion.vue';
import FirmwareUpdate from '@/components/settings/FirmwareUpdate.vue';
</script>

