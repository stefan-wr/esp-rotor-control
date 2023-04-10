import { createApp } from 'vue'
import { createPinia } from 'pinia'
import { useRotorStore } from '@/stores/rotor'

import App from './App.vue'
import router from './router'

import './assets/main.css'

const app = createApp(App)

app.use(createPinia())
app.use(router)

app.mount('#app')

const store = useRotorStore();



const sockets = [
    {
        gateway: `ws://home.wraase.de:1339/ws`,
        socket: null
    }
];

function initWebSocket(i) {
    console.log('[' + sockets[i].gateway + '] Open connection...');
    sockets[i].socket = new WebSocket(sockets[i].gateway);
    // Open
    sockets[i].socket.onopen = function (event) {
        console.log('[' + sockets[i].gateway + '] Connected.');
    };
    // Close
    sockets[i].socket.onclose = function (event) {
        console.log('[' + sockets[i].gateway + '] Connection closed. (' + event.reason + ')');
        setTimeout(initWebSocket, 1000, i);
    };
    // Error
    sockets[i].socket.onerror = function (event) {
        console.log('[' + sockets[i].gateway + '] Error.');
        sockets[i].socket.close();
    };
    // Receive message
    sockets[i].socket.onmessage = receiveData;
}

const switchesIdentifier = "SWITCHES";
const rotorIdentifier = "ROTOR";
const settingsIdentifier = "SETTINGS";

// Receive data from WebSocket
// ---------------------------
function receiveData(event) {
    // Split type-identifier from JSON data
    var [identifier, data] = event.data.split("|");

    // ROTOR DATA
    // ..........
    if (identifier === "ROTOR") {
        console.log('[' + event.origin + '] ' + event.data);
        var rotor_data = JSON.parse(data);

        
        // Go through rotor data
        for (let key in rotor_data) {
            store.rotor[key] = rotor_data[key];
        }
    }

    // SETTINGS
    // ........
    if (identifier === "SETTINGS") {
        console.log('[' + event.origin + '] ' + event.data);
        settings_data = JSON.parse(data);

        // Go through settings data  
        for (key in settigs_data) {
            if (key === "cal_u1") {
                store.settings.u1 = settings_data[key];
            }
        }
    }
}

initWebSocket(0);
