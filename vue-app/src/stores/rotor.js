import { ref, computed, reactive, watch } from 'vue';
import { defineStore } from 'pinia';

export const useRotorStore = defineStore('rotor', () => {
    // Cardinal directions
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
        rotation: 0, // -1: CCW, 0:Stop, 1:CW
        angle: 0.0,
        adc_v: 0.0,
        speed: 0, // 0-100
        target: null
    });

    // *************
    //    Getters
    // *************

    // Get cardinal direction
    const cardinal = computed(() => {
        const ci = Math.round(rotor.angle / (360 / cardinals.length));
        return cardinals[ci % cardinals.length];
    });

    // Get angle as string with zero decimals
    const angle = computed(() => {
        return rotor.angle.toFixed(0);
    });

    // Get angle as string with one decimal
    const angle1D = computed(() => {
        return rotor.angle.toFixed(1);
    });

    // Get wether rotor is in overlap mode
    const isOverlap = computed(() => {
        return rotor.angle > 360;
    });

    // Get wether a target has been set
    const hasTarget = computed(() => {
        if (rotor.target == null) {
            return false;
        } else {
            return true;
        }
    });

    // Get speed JSON message
    const getSpeedMsg = computed(() => {
        //return `{\"speed\":${rotor.speed}}`;
        return JSON.stringify({ speed: rotor.speed });
    });

    // Get rotation JSON message
    function getRotationMsg(dir) {
        return JSON.stringify({ rotation: dir });
    }

    // Get auto-rotation request JSON message
    function getTargetMessage(angle, useOverlap) {
        return JSON.stringify({
            target: angle,
            useOverlap: useOverlap
        });
    }

    // *************
    //    Actions
    // *************

    // Reset target when rotor rotation stops
    watch(
        () => rotor.rotation,
        (newRotation, oldRotation) => {
            if (!newRotation && oldRotation) {
                rotor.target = null;
            }
        }
    );

    // *****
    return {
        rotor,
        cardinal,
        angle,
        angle1D,
        isOverlap,
        hasTarget,
        getRotationMsg,
        getSpeedMsg,
        getTargetMessage
    };
});
