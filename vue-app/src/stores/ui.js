import { ref, computed, reactive } from 'vue';
import { defineStore } from 'pinia';

export const useUIStore = defineStore('ui', () => {

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
