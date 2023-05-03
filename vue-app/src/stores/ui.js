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
        fontSize: 14,

        cards: {},

        reqAngle: 0,
        reqAngleLocked: false,
        isMouseInCompass: false
    });

    // *************
    //    Getters
    // *************

    // *************
    //    Actions
    // *************

    return { ui };
});
