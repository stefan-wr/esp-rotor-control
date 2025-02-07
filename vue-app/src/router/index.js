import { createRouter, createWebHistory } from 'vue-router';
import MainView from '../views/MainView.vue';
import SettingsView from '../views/SettingsView.vue';
//import SettingsViewOld from '../views/SettingsViewOld.vue';
import MessageView from '../views/MessageView.vue';
import UpdateView from '../views/UpdateView.vue';
import SettingsSystem from '../components/settings/SettingsSystem.vue';
import SettingsUI from '../components/settings/SettingsUI.vue';
import SettingsFeatures from '../components/settings/SettingsFeatures.vue';
import SettingsCalibration from '../components/settings/SettingsCalibration.vue';
import SettingsLicenses from '../components/settings/SettingsLicenses.vue';

const router = createRouter({
    history: createWebHistory(import.meta.env.BASE_URL),
    routes: [
        // Root Page
        // ---------
        {
            path: '/',
            name: 'main',
            component: MainView
        },

        // Settings Page
        // -------------
        // {
        //     path: '/settings-old',
        //     name: 'settings-old',
        //     component: SettingsViewOld
        // },
        {
            path: '/settings',
            component: SettingsView,
            children: [
                { path: '', redirect: {name: 'system'} },
                { path: 'system', name: 'system', component: SettingsSystem },
                { path: 'ui', name: 'ui', component: SettingsUI },
                { path: 'features', name: 'features', component: SettingsFeatures },
                { path: 'calibration', name : 'calibration', component: SettingsCalibration },
                { path: 'licenses', name: 'licenses', component: SettingsLicenses }
            ]
        },

        // Message Views
        // -------------
        {
            path: '/settings',
            name: 'reboot',
            component: MessageView,
            props: {
                message: 'messages.reboot',
                pushTo: 'system',
                pushAfter: 5000,
                disableHeaderLinks: true
            }
        },
        {
            path: '/settings',
            name: 'reboot-failed',
            component: MessageView,
            props: {
                message: 'messages.rebootFailed',
                pushTo: 'system',
                pushAfter: 5000
            }
        },
        {
            path: '/settings',
            name: 'disconnect',
            component: MessageView,
            props: {
                message: 'messages.disconnect',
                disableHeaderLinks: true
            }
        },
        {
            path: '/settings',
            name: 'disconnect-failed',
            component: MessageView,
            props: {
                message: 'messages.disconnectFailed',
                pushTo: 'system',
                pushAfter: 5000
            }
        },
        {
            path: '/update',
            name: 'update',
            component: UpdateView
        },

        // Catch-all route to main page
        {
            path: '/:pathMatch(.*)',
            component: MainView
        }
    ]
});

export default router;
