import { ref, computed } from 'vue';
import { defineStore } from 'pinia';

export const useRotorStore = defineStore('rotor', {
  // >>>>> State
  state: () => ({
    rotor: {
      rotation: 0,
      angle: 0.0,
      adc_v: 0.0,
      speed: 0
    },

    settings: {
      offset: 0.0,
      cal: {
        u1: 0.0,
        u2: 0.0,
        a1: 0.0,
        a2: 0.0
      }
    },

    ui: {
      kbsc_enabled: true,
      show_settings: false,
      show_calibration: false
    },

    cardinals: ["N", "NNO", "NO", "ONO",
                "O", "OSO", "SO", "SSO",
                "S", "SSW", "SW", "WSW",
                "W", "WNW", "NW", "NNW" ]
  }),

  // >>>>> Getters
  getters: {
    // Get cardinal direction
    getCardinal: (state) => {
      var ci = Math.round((state.rotor.angle / (360 / state.cardinals.length)));
      return state.cardinals[ci % state.cardinals.length];
    },

    // Get angle as string with one decimal
    getAngle1D: (state) => {
      return state.rotor.angle.toFixed(1);
    },

    // Get angle as string with two decimals
    getAngle2D: (state) => {
      return state.rotor.angle.toFixed(2);
    }
  },

  // >>>>> Actions
  actions: {
    setRotation(direction, event) {
      this.rotor.rotation = direction;
    }
  }
});
