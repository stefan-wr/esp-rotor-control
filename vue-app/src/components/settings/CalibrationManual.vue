<template>
  <div class="flex-vst gap-one">

    <i18n-t keypath="calibration.manual.dscr" tag="p" scope="global">
      <template #comparison>
        <span class="no-wrap">Pos. 1 &lt; Pos. 2</span>
      </template>
    </i18n-t>

    <ShakeOnToggle class="man-cal-inputs border-box" ref="manForm">
      <span></span>
      <label for="azimuth-1" class="small txt-dark">{{ $t('commons.azimut') }} in °</label>
      <label for="adc-1" class="small txt-dark">{{ $t('commons.voltage') }} in V</label>

      <!-- Position 1 -->
      <span>Pos.&nbsp;1</span>

      <input
        class="input-std-resp"
        :class="{ 'red-outline': isNewPos1AngleWrong }"
        type="number"
        name="azimuth-1"
        :placeholder="$t('commons.azimut')"
        v-model="newPos1Angle"
      />

      <input
        class="input-std-resp"
        :class="{ 'red-outline': isNewPos1AdcWrong }"
        type="number"
        step="0.001"
        name="adc-1"
        :placeholder="$t('commons.voltage')"
        v-model="newPos1Adc"
      />

      <!-- Position 2 -->
      <span></span>
      <label for="azimuth-2" class="small txt-dark">{{ $t('commons.azimut') }} in °</label>
      <label for="adc-2" class="small txt-dark">{{ $t('commons.voltage') }} in V</label>

      <span>Pos.&nbsp;2</span>
      <input
        class="input-std-resp"
        :class="{ 'red-outline': isNewPos2AngleWrong }"
        type="number"
        name="azimuth-2"
        :placeholder="$t('commons.azimut')"
        v-model="newPos2Angle"
      />

      <input
        class="input-std-resp"
        :class="{ 'red-outline': isNewPos2AdcWrong }"
        type="number"
        step="0.001"
        name="adc-2"
        :placeholder="$t('commons.voltage')"
        v-model="newPos2Adc"
      />

      <hr />

      <button
        class="btn-std-resp bold no-wrap-ellip"
        :title="$t('calibration.manual.btnDscr')"
        ref="confirmBtn"
        :disabled="!isManualConfirmEnabled"
        @click="confirmManualCal"
      >
        <Icon icon="fa-solid fa-check"></Icon>&nbsp;{{ $t('commons.accept') }}
      </button>
    </ShakeOnToggle>
  </div>
</template>

<script setup>
import ShakeOnToggle from '@/components/ShakeOnToggle.vue';
import { ref, computed } from 'vue';
import { useUmbrellaStore } from '@/stores/umbrella';
const umbrellaStore = useUmbrellaStore();

import { useI18n } from "vue-i18n";
const { t } = useI18n();

// Utilitiy functions
// ------------------

// Test wether value of an input is a number
// Used for alert-colored outlines, indicating wrong values.
function testInputForNumber(value) {
  return typeof value === 'number';
}

// Calibration parameters
// ----------------------

// V-Models
const newPos1Angle = ref(null);
const newPos2Angle = ref(null);
const newPos1Adc = ref(null);
const newPos2Adc = ref(null);

// Test wether values in inputs are not allowed
const isNewPos1AngleWrong = computed(() => {
  return !(newPos1Angle.value === null || testInputForNumber(newPos1Angle.value));
});
const isNewPos2AngleWrong = computed(() => {
  return !(newPos2Angle.value === null || testInputForNumber(newPos2Angle.value));
});
const isNewPos1AdcWrong = computed(() => {
  return !(newPos1Adc.value === null || testInputForNumber(newPos1Adc.value));
});
const isNewPos2AdcWrong = computed(() => {
  return !(newPos2Adc.value === null || testInputForNumber(newPos2Adc.value));
});

// Confirm parameters
// ------------------

// Enable / disable confirm button
const isManualConfirmEnabled = computed(() => {
  if (
    newPos1Angle.value !== null &&
    testInputForNumber(newPos1Angle.value) &&
    newPos2Angle.value !== null &&
    testInputForNumber(newPos2Angle.value) &&
    newPos1Adc.value !== null &&
    testInputForNumber(newPos1Adc.value) &&
    newPos2Adc.value !== null &&
    testInputForNumber(newPos2Adc.value)
  ) {
    return true;
  } else {
    return false;
  }
});

// Shake form
// ----------
const manForm = ref(null);

function shakeForm() {
  manForm.value.shake();
}

// Confirm calibration
// -------------------
function testNewParams() {
  if (newPos2Angle.value <= newPos1Angle.value) {
    alert(t('calibration.manual.errorAngle'));
    return false;
  } else if (newPos2Adc.value <= newPos1Adc.value) {
    alert(t('calibration.manual.errorVoltage'));
    return false;
  } else {
    return true;
  }
}

function confirmManualCal() {
  if (testNewParams()) {
    umbrellaStore.sendCalibration(
      Number(newPos1Angle.value.toFixed(4)),
      Number(newPos1Adc.value.toFixed(4)),
      Number(newPos2Angle.value.toFixed(4)),
      Number(newPos2Adc.value.toFixed(4))
    );
  } else {
    shakeForm();
  }
}

// Reset parameters and inputs
// ---------------------------
function reset() {
  newPos1Angle.value = null;
  newPos2Angle.value = null;
  newPos1Adc.value = null;
  newPos2Adc.value = null;
}

// Expose reset function to parent components
defineExpose({
  reset
});
</script>

<style lang="scss" scoped>
.man-cal-inputs {
  display: grid;
  grid-template-columns: auto 1fr 1fr;
  grid-template-rows: auto 1fr auto 1fr;
  align-items: center;
  column-gap: 1em;

  label {
    margin-bottom: 0.4em;
  }

  label:nth-child(n + 8) {
    margin-top: 1em;
  }

  hr {
    margin: 1em 0;
    grid-column: 1 / 4;
  }

  button {
    grid-column: 1 / 4;
  }
}
</style>
