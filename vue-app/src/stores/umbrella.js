import { ref, computed, reactive } from 'vue';
import { defineStore } from 'pinia';

import { useRotorStore } from './rotor.js';
import { useSettingsStore } from './settings.js';
import { useUIStore } from './ui.js';

/**
 * Main store handling all sockets sends and receives.
 * Distributes changes down to all other stores.
 */
export const useUmbrellaStore = defineStore('umbrella', () => {
    // Other stores
    const rotorStore = useRotorStore();
    const settingsStore = useSettingsStore();
    const uiStore = useUIStore();

    // Check Connection
    // ****************
    const connectionTimeout = 5000;
    const hasLostConnection = ref(false);
    let timeOfLastMsg = (new Date()).getTime();

    function updateTimeOfLastMsg() {
        const date = new Date();
        timeOfLastMsg = date.getTime();
    }

    // Check for loss of connection by checking for incoming messages
    // Close socket if connection lost -> this will trigger initWebSocket
    function checkConnection() {
        const date = new Date();
        const now = date.getTime();
        if (now - timeOfLastMsg >= connectionTimeout) {
            hasLostConnection.value = true;
            console.log("Connection lost. Closing WebSocket.")
            socket.socket.close();
        }
    }

    setInterval(checkConnection, connectionTimeout);

    // Socket
    // ******
    const socket = 
        {
            gateway: `ws://home.wraase.de:1339/ws`,
            socket: null
        }

    // Initialise Socket
    // -----------------
    function initWebSocket() {
        console.log('[' + socket.gateway + '] Open connection...');
        updateTimeOfLastMsg();
        socket.socket = new WebSocket(socket.gateway);
        // Open
        socket.socket.onopen = function (event) {
            console.log('[' + socket.gateway + '] Connected.');
        };
        // Close
        socket.socket.onclose = function (event) {
            console.log('[' + socket.gateway + '] Connection closed. (' + event.reason + ')');
            setTimeout(initWebSocket, 1000);
        };
        // Error
        socket.socket.onerror = function (event) {
            console.log('[' + socket.gateway + '] Error.');
            socket.socket.close();
        };
        // Receive message
        socket.socket.onmessage = receiveData;
    }

    // Receive data from Socket
    // ------------------------
    const rotorIdentifier = 'ROTOR';
    const settingsIdentifier = 'SETTINGS';
    const uiIdentifier = 'UI';

    function receiveData(event) {
        hasLostConnection.value = false;
        updateTimeOfLastMsg();
        
        // Split type-identifier from JSON data
        var [identifier, msg] = event.data.split('|');

        // ROTOR MESSAGE
        if (identifier === rotorIdentifier) {
            console.log('[' + event.origin + '] ' + event.data);
            receiveRotorMsg(msg);
        }

        // SETTINGS MESSAGE
        if (identifier === settingsIdentifier) {
            console.log('[' + event.origin + '] ' + event.data);
            receiveSettingsMsg(msg);
        }
    }

    // Receiver for rotor message
    function receiveRotorMsg(msg) {
        let rotorMsg = JSON.parse(msg);
        for (let key in rotorMsg) {
            if (key in rotorStore.rotor) {
                rotorStore.rotor[key] = rotorMsg[key];
            } else {
                console.log(`ERROR: key '${key}' not in rotor.`);
            }
        }
    }

    // Receiver for settings message
    function receiveSettingsMsg(msg) {
        settingsMsg = JSON.parse(msg);
        for (key in settigsMsg) {
            if (key === 'cal_u1') {
                settingsStore.settings.cal_u1;
            }
        }
    }

    // Send data over socket[i]
    // ------------------------
    function sendData(data) {
        socket.socket.send(data);
    }

    // Init sockets
    // ------------
    initWebSocket();

    // *************
    //    Actions
    // *************

    // General message senders
    // -----------------------
    function sendRotorMsg(msg) {
        sendData(`${rotorIdentifier}|${msg}`);
    }

    function sendSettingsMsg(msg) {
        sendData(`${settingsIdentifier}|${msg}`);
    }

    function sendUiMsg(msg) {
        sendData( `${uiIdentifier}|${msg}`);
    }

    // Specific senders
    function sendRotation(dir) {
        sendRotorMsg(rotorStore.getRotationMsg(dir));
    }

    function sendSpeed(spd) {
        sendRotorMsg(rotorStore.getSpeedMsg());
    }

    // Setters for setting state in used stores
    // ----------------------------------------


    return {
        rotorStore,
        settingsStore,
        uiStore,
        hasLostConnection,
        sendRotation,
        sendSpeed
    };
});
