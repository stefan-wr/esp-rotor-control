<template>
  <!-- Steps -->
  <div class="flex-vst gap-one">
    <ol class="steps flex-vl gap" type="1">
      <li>{{ $t('calibration.guided.step1') }}</li>
      <li>{{ $t('calibration.guided.step2') }}</li>
      <li>{{ $t('calibration.guided.step3') }}</li>
      <li>{{ $t('calibration.guided.step4') }}</li>
      <li>{{ $t('calibration.guided.step5') }}</li>
    </ol>

    <hr />

    <!-- Position 1 Input -->
    <div class="auto-cal-inputs flex-cst gap-one">
      <ShakeOnToggle
        ref="pos1Form"
        class="flex-grow border-box flex-vst gap-one auto-cal-pos"
      >
        <label for="azimuth-1" class="flex gap-half">
          <span>Position&nbsp;1</span>
          <span class="small no-wrap txt-dark">({{ pos1Range[0] }}° - {{ pos1Range[1] }}°)</span>
        </label>

        <input
          class="input-std-resp"
          type="number"
          name="azimuth-1"
          :min="pos1Range[0]"
          :max="pos1Range[1]"
          placeholder="Pos. 1"
          ref="pos1Input"
          v-model="newPos1Angle"
          :class="{ 'red-outline': isNewPos1AngleWrong }"
          @keyup.enter="confirmPos1($event)"
        />

        <button
          class="btn-std-resp bold"
          :title="$t('calibration.guided.confirmPosBtnDscr', {'pos': 1})"
          @click="confirmPos1($event)"
          :disabled="newPos1Angle === null || isNewPos1AngleWrong"
        >
          <Icon icon="fa-solid fa-check"></Icon>
        </button>
      </ShakeOnToggle>

      <!-- Position 2 Input -->
      <ShakeOnToggle
        ref="pos2Form"
        class="flex-grow border-box flex-vst gap-one auto-cal-pos"
      >
        <label for="azimuth-2" class="flex gap-half">
          <span>Position&nbsp;2</span>
          <span class="small no-wrap txt-dark">({{ pos2Range[0] }}° - {{ pos2Range[1] }}°)</span>
        </label>

        <input
          class="input-std-resp"
          type="number"
          name="azimuth-2"
          :min="pos2Range[0]"
          :max="pos2Range[1]"
          placeholder="Pos. 2"
          ref="pos2Input"
          v-model="newPos2Angle"
          :class="{ 'red-outline': isNewPos2AngleWrong }"
          @keyup.enter="confirmPos2($event)"
        />

        <button
          class="btn-std-resp bold"
          :title="$t('calibration.guided.confirmPosBtnDscr', {'pos': 2})"
          @click="confirmPos2($event)"
          :disabled="newPos2Angle === null || isNewPos2AngleWrong"
        >
          <Icon icon="fa-solid fa-check"></Icon>
        </button>
      </ShakeOnToggle>
    </div>

    <!-- Arrows -->
    <div class="auto-cal-arrows flex-cst gap-one medium">
      <div class="flex-grow flex-cc">
        <Icon icon="fa-solid fa-arrow-down"></Icon>
      </div>
      <div class="flex-grow flex-cc">
        <Icon icon="fa-solid fa-arrow-down"></Icon>
      </div>
    </div>

    <!-- Confirm Labels-->
    <div class="auto-cal-confirm flex-vst gap-one">
      <div class="flex-cst gap-one">
        <div class="flex-grow flex-vc border-box auto-cal-confirm-lbl">
          <span class="small txt-dark">Position 1</span>
          <span>{{ dashedInitial(pos1Angle) }}°</span>
          <span>{{ dashedInitial(pos1Adc) }} V</span>
        </div>
        <div class="flex-grow flex-vc border-box auto-cal-confirm-lbl">
          <span class="small txt-dark">Position 2</span>
          <span>{{ dashedInitial(pos2Angle) }}°</span>
          <span>{{ dashedInitial(pos2Adc) }} V</span>
        </div>
      </div>

      <!-- Confirm Button -->
      <button
        class="btn-std-resp bold no-wrap-ellip"
        :title="$t('calibration.guided.confirmBtnDscr')"
        ref="calConfirmBtn"
        :disabled="!isAutoConfirmEnabled"
        @click="confirmCalibration()"
      >
        <Icon icon="fa-solid fa-check"></Icon>&nbsp;{{ $t('commons.accept') }}
      </button>
    </div>
  </div>
</template>

<script setup>
import ShakeOnToggle from '@/components/ShakeOnToggle.vue';
import { ref, computed } from 'vue';
import { useI18n } from 'vue-i18n';
import { useUmbrellaStore } from '@/stores/umbrella';
import { useRotorStore } from '@/stores/rotor';

const { t } = useI18n();
const umbrellaStore = useUmbrellaStore();
const rotorStore = useRotorStore();

// Element Refs
// ------------
const pos1Form = ref(null);
const pos2Form = ref(null);
const pos1Input = ref(null);
const pos2Input = ref(null);
const calConfirmBtn = ref(null);

// Utilitiy functions
// ------------------

// Change initial values of confirm labels to dashes
function dashedInitial(value) {
  if (value) {
    return value;
  } else {
    return '--';
  }
}

// Test wether value of an input is a number
// Used for alert-colored outlines, indicating wrong values.
function testInputForNumber(value) {
  return typeof value === 'number';
}

// Test wether a given number value is in a range [min, max]
function valueInRange(value, range) {
  return value >= range[0] && value <= range[1];
}

// Calibration parameters
// ----------------------
const newPos1Angle = ref(null);
const pos1Range = [20, 40];

// Test wether entered value on pos 1 input is not allowed
const isNewPos1AngleWrong = computed(() => {
  if (newPos1Angle.value === null) {
    // Null is initial value
    return false;
  } else if (
    testInputForNumber(newPos1Angle.value) &&
    valueInRange(newPos1Angle.value, pos1Range)
  ) {
    return false;
  } else {
    return true;
  }
});

// ----------

const newPos2Angle = ref(null);
const pos2Range = [420, 450];

// Test wether entered value on pos 2 input is not allowed
const isNewPos2AngleWrong = computed(() => {
  if (newPos2Angle.value === null) {
    // Null is initial value
    return false;
  } else if (
    testInputForNumber(newPos2Angle.value) &&
    valueInRange(newPos2Angle.value, pos2Range)
  ) {
    return false;
  } else {
    return true;
  }
});

// Confirm parameters
// ------------------
const pos1Angle = ref(0);
const pos1Adc = ref(0);

// Confirm pos 1 angle
function confirmPos1(event) {
  if (newPos1Angle.value !== null && !isNewPos1AngleWrong.value) {
    pos1Angle.value = Number(newPos1Angle.value.toFixed(1));
    pos1Adc.value = Number(rotorStore.rotor.adc_v.toFixed(4));
    pos2Input.value.focus();
    newPos1Angle.value = null;
  } else if (event.target.tagName === 'BUTTON') {
    // Only reset input if event came from button, shouldn't be reachable
    shakePos1Form();
    newPos1Angle.value = null;
    pos1Input.value.focus();
  } else {
    shakePos1Form();
  }
}

// ----------

const pos2Angle = ref(0);
const pos2Adc = ref(0);

// Confirm pos 2 angle
function confirmPos2(event) {
  if (newPos2Angle.value !== null && !isNewPos2AngleWrong.value) {
    pos2Angle.value = Number(newPos2Angle.value.toFixed(1));
    pos2Adc.value = Number(rotorStore.rotor.adc_v.toFixed(4));
    calConfirmBtn.value.focus();
    newPos2Angle.value = null;
  } else if (event.target.tagName === 'BUTTON') {
    // Only reset input if event came from button, shouldn't be reachable
    shakePos2Form();
    newPos2Angle.value = null;
    pos2Input.value.focus();
  } else {
    shakePos2Form();
  }
}

// Shake Forms
// -----------
function shakePos1Form() {
  pos1Form.value.shake();
}

function shakePos2Form() {
  pos2Form.value.shake();
}

// Enable / disable main confirm button
// ------------------------------------
const isAutoConfirmEnabled = computed(() => {
  if (
    pos1Angle.value >= pos1Range[0] &&
    pos1Angle.value <= pos1Range[1] &&
    pos2Angle.value >= pos2Range[0] &&
    pos2Angle.value <= pos2Range[1]
  ) {
    return true;
  } else {
    return false;
  }
});

// Confirm Calibration
// -------------------
function testNewParams() {
  if (pos2Adc.value <= pos1Adc.value) {
    alert(t('calibration.guided.errorVoltage'));
    return false;
  } else {
    return true;
  }
}

function confirmCalibration() {
  if (testNewParams()) {
    umbrellaStore.sendCalibration(pos1Angle.value, pos1Adc.value, pos2Angle.value, pos2Adc.value);
  } else {
    shakePos1Form();
    shakePos2Form();
  }
}

// Reset parameters and inputs
// ---------------------------
function reset() {
  newPos1Angle.value = null;
  newPos2Angle.value = null;

  pos1Angle.value = null;
  pos2Angle.value = null;
  pos1Adc.value = null;
  pos2Adc.value = null;
}

// Expose reset function to parent components
defineExpose({
  reset
});
</script>

<style lang="scss" scoped>
/* Steps */
.steps {
  list-style-type: none;
  position: relative;
  counter-reset: item;

  $list-style-dim: 1.6em;
  $vl-width: 2px;

  &:before {
    content: '';
    width: $vl-width;
    height: calc(100% - $list-style-dim);
    border-radius: 100px;
    position: absolute;
    left: calc($list-style-dim / 2 - $vl-width / 2);
    top: 50%;
    transform: translateY(-50%);
    background-color: var(--content-color-2);
  }

  li {
    counter-increment: item;
    position: relative;

    &:before {
      content: counter(item);
      display: flex;
      align-items: center;
      justify-content: center;
      width: $list-style-dim;
      height: $list-style-dim;
      border-radius: 100px;
      position: absolute;
      left: -$ol-left-padding;
      background-color: var(--content-color-2);
      color: var(--text-color-darker);
    }
  }
}

/* Calibration Inputs */
.auto-cal-inputs {
  label {
    flex-wrap: wrap;
    align-items: baseline;
    justify-content: center;
    text-align: center;

    /* Makes sure labels wraps evenly on both positions */
    span:first-child {
      flex-basis: 5em;
    }

    span:nth-child(2) {
      flex-basis: 6.5em;
    }
  }
}

/* Calibration Position Labels */
.auto-cal-pos {
  /* Both position boxes have same start width and grow evenly */
  flex-basis: 0;
}

.auto-cal-arrows {
  color: var(--content-color-2);
  margin: -0.5em 0;
}

.auto-cal-confirm-lbl {
  padding: 0.5em !important;
  flex-basis: 0;
}
</style>
