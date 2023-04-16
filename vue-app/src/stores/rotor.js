import { ref, computed, reactive } from 'vue';
import { defineStore } from 'pinia';

export const useRotorStore = defineStore('rotor', () => {
    // Cardinal directions, non-reactive
    const cardinals = [
        'N',
        'NNO',
        'NO',
        'ONO',
        'O',
        'OSO',
        'SO',
        'SSO',
        'S',
        'SSW',
        'SW',
        'WSW',
        'W',
        'WNW',
        'NW',
        'NNW'
    ];

    // **********************
    //    State, Reactives
    // **********************

    const rotor = reactive({
        rotation: 0,    // -1: CCW, 0:Stop, 1:CW
        angle: 0.0,
        adc_v: 0.0,
        speed: 0        // 0-100
    });

    // *************
    //    Getters
    // *************

    // Get cardinal direction
    const cardinal = computed(() => {
        const ci = Math.round(rotor.angle / (360 / cardinals.length));
        return cardinals[ci % cardinals.length];
    });

    // Get angle as string with one decimal
    const angle1D = computed(() => {
        return rotor.angle.toFixed(1);
    });

    // Get angle as string with two decimals
    const angle2D = computed(() => {
        return rotor.angle.toFixed(2);
    });

    // Get wether rotor is in overlap mode
    const isOverlap = computed(() => {
        return (rotor.angle > 360);
    });


    // *************
    //    Actions
    // *************

    // Get messages for sending to ESP32
    function getRotationMsg(dir) {
        return `{\"rotation\":${dir}}`;
    }

    function getSpeedMsg() {
        return `{\"speed\":${rotor.speed}}`;
    }

    return {
        rotor,
        cardinal,
        angle1D,
        angle2D,
        isOverlap,
        getRotationMsg,
        getSpeedMsg,
    };
});
