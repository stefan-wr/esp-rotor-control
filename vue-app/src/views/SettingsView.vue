<template>
  <main class="gap-one">
    <div id="settings-left" class="flex-vst gap-one">
      <h2 class="medium">System</h2>
      <Disconnect></Disconnect>
      <Reboot></Reboot>
      <FirmwareVersion></FirmwareVersion>
      <FirmwareUpdate></FirmwareUpdate>
      <h2 class="medium">Benutzeroberfläche</h2>
      <FontSize></FontSize>
      <ColorTheme></ColorTheme>
      <CompassSettings></CompassSettings>
    </div>

    <div id="settings-right" class="flex-vst gap-one">
      <h2 class="medium">Funktionen</h2>
      <KeyboardShortcuts></KeyboardShortcuts>
      <UseOverlap></UseOverlap>
      <ResetLock></ResetLock>
      <h2 class="medium">Kalibrierung</h2>
      <ADC></ADC>
      <CalibrationCurrent></CalibrationCurrent>
      <Calibration></Calibration>
    </div>

    <Footer id="footer"></Footer>
  </main>
</template>

<script setup>
import KeyboardShortcuts from '@/components/settings/KeyboardShortcuts.vue';
import UseOverlap from '@/components/settings/UseOverlap.vue';
import Disconnect from '@/components/settings/Disconnect.vue';
import Reboot from '@/components/settings/Reboot.vue';
import FirmwareVersion from '@/components/settings/FirmwareVersion.vue';
import FirmwareUpdate from '@/components/settings/FirmwareUpdate.vue';
import FontSize from '@/components/settings/FontSize.vue';
import ColorTheme from '@/components/settings/ColorTheme.vue';
import ResetLock from '@/components/settings/ResetLock.vue';
import CompassSettings from '@/components/settings/CompassSettings.vue';
import CalibrationCurrent from '@/components/settings/CalibrationCurrent.vue';
import ADC from '@/components/settings/ADC.vue';
import Calibration from '@/components/settings/Calibration.vue';
import Footer from '@/components/Footer.vue';
</script>

<style lang="scss" scoped>
main {
  display: grid;
  align-items: start;
  justify-items: stretch;
  grid-template-columns: minmax(0, 1fr) minmax(0, 1fr);

  h2:not(h2:first-child) {
    margin-top: 1em;
  }

  @include medium {
    display: flex;
    flex-direction: column;
    align-items: start;
  }
}

#settings-left,
#settings-right {
  width: 100%;
}

#settings-right {
  @include medium {
    h2 {
      margin-top: 1em;
    }
  }
}

#footer {
  grid-column: 1 / 3;
  width: 100%;
}
</style>
