import { ref, computed, reactive } from 'vue';
import { defineStore } from 'pinia';
import { useSettingsStore } from './settings.js';
import { useRotorStore } from './rotor.js';

export const useUIStore = defineStore('ui', () => {
    // Other stores
    const settings = useSettingsStore();
    const rotor = useRotorStore();

    // **********************
    //    State, Reactives
    // **********************

    const ui = reactive({
        kbscEnabled: true,
        showSettings: false,
        showCalibration: false,

        reqAngle: 0,
        reqAngleLocked: false,
        isMouseInCompass: false,

        cards: {}
    });

    // *************
    //    Getters
    // *************

    // *************
    //    Actions
    // *************

    return { ui };
});
