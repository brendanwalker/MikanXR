import {
    MikanClient,
    MikanClientOptions,
    MikanLogLevel,
    MikanClientGraphicsApi,
    MikanAPIResult,
    GetAppStageInfo,
    CLASS_ID_GET_APP_STAGE_INFO,
    MikanAppStageInfoResponse
} from '@mikanxr/client';
import { nativeBridge } from './native-bridge.js';

// UI Elements
const statusDiv = document.getElementById('status')!;
const connectBtn = document.getElementById('connect-btn') as HTMLButtonElement;
const launchBtn = document.getElementById('launch-btn') as HTMLButtonElement;
const serverUrlInput = document.getElementById('server-url') as HTMLInputElement;
const connectionPanel = document.getElementById('connection-panel')!;
const mainContent = document.getElementById('main-content')!;

// Initialize Mikan Client
let mikanClient: MikanClient | null = null;
let isConnecting = false;

// Update status UI
function updateStatus(status: 'connecting' | 'connected' | 'disconnected' | 'error', message?: string) {
    statusDiv.className = 'status ' + status;

    switch (status) {
        case 'connecting':
            statusDiv.textContent = 'Connecting to Mikan...';
            connectBtn.disabled = true;
            break;
        case 'connected':
            statusDiv.textContent = 'Connected to Mikan';
            connectBtn.textContent = 'Disconnect';
            connectBtn.disabled = false;
            break;
        case 'disconnected':
            statusDiv.textContent = 'Disconnected from Mikan';
            connectBtn.textContent = 'Connect';
            connectBtn.disabled = false;
            connectionPanel.classList.remove('hidden');
            mainContent.classList.add('hidden');
            break;
        case 'error':
            statusDiv.textContent = 'Connection Error: ' + (message || 'Unknown error');
            connectBtn.textContent = 'Connect';
            connectBtn.disabled = false;
            break;
    }
}

// Parse WebSocket URL
function parseWebSocketUrl(url: string): { host: string; port: string } {
    try {
        const wsUrl = new URL(url);
        return {
            host: wsUrl.hostname,
            port: wsUrl.port || '8080'
        };
    } catch (e) {
        // Fallback parsing
        const parts = url.replace('ws://', '').replace('wss://', '').split(':');
        return {
            host: parts[0] || 'localhost',
            port: parts[1] || '8080'
        };
    }
}

// Connect to Mikan
async function connectToMikan(url: string) {
    if (isConnecting) return;

    isConnecting = true;
    updateStatus('connecting');

    try {
        const { host, port } = parseWebSocketUrl(url);

        const options: MikanClientOptions = {
            host,
            port,
            autoReconnect: false
        };

        mikanClient = new MikanClient(options);

        // Set log callback
        mikanClient.setLogCallback((level: MikanLogLevel, message: string) => {
            console.log(`[Mikan ${MikanLogLevel[level]}] ${message}`);
        });

        // Initialize client
        const initResult = mikanClient.initialize(MikanLogLevel.Info);
        if (initResult !== 0) {
            throw new Error('Failed to initialize Mikan client');
        }

        // Connect
        const connectResult = await mikanClient.connect();
        if (connectResult !== 0) {
            throw new Error('Failed to connect to Mikan');
        }

        updateStatus('connected');
        await onConnected();

    } catch (error) {
        console.error('Connection error:', error);
        updateStatus('error', error instanceof Error ? error.message : 'Unknown error');
        mikanClient = null;
    } finally {
        isConnecting = false;
    }
}

// Disconnect from Mikan
function disconnectFromMikan() {
    if (mikanClient) {
        mikanClient.disconnect();
        mikanClient.shutdown();
        mikanClient = null;
    }
    updateStatus('disconnected');
}

// Called when successfully connected
async function onConnected() {
    if (!mikanClient) return;

    try {
        // Get app stage info
        const request: GetAppStageInfo = {
            requestTypeId: CLASS_ID_GET_APP_STAGE_INFO,
            requestTypeName: 'GetAppStageInfo',
            requestId: 0
        };

        const future = mikanClient.sendRequest(request);
        const response = await future.await();

        if (response.resultCode === MikanAPIResult.Success) {
            const appStageInfoResponse = response as MikanAppStageInfoResponse;
            showAppStage(appStageInfoResponse.app_stage_info.app_state_name);
        }
    } catch (error) {
        console.error('Failed to get app stage info:', error);
        updateStatus('error', 'Failed to get app info');
    }
}

// Show the appropriate UI based on app stage
function showAppStage(appStage: string) {
    console.log('App stage:', appStage);

    // Hide connection panel, show main content
    connectionPanel.classList.add('hidden');
    mainContent.classList.remove('hidden');

    // Load the appropriate page based on app stage
    switch (appStage) {
        case 'MainMenu':
            loadPage('main_menu.html');
            break;
        case 'VideoSourceSettings':
            loadPage('video_source_settings.html');
            break;
        case 'AnchorSetup':
            loadPage('anchor_setup.html');
            break;
        default:
            mainContent.innerHTML = `<h2>Unknown App Stage: ${appStage}</h2>`;
    }
}

// Load a page into main content
async function loadPage(pageName: string) {
    try {
        const response = await fetch(pageName);
        const html = await response.text();
        mainContent.innerHTML = html;
        initializePage(pageName);
    } catch (error) {
        console.error('Failed to load page:', error);
        mainContent.innerHTML = '<h2>Error loading page</h2>';
    }
}

// Initialize page-specific functionality
function initializePage(pageName: string) {
    console.log('Initialized page:', pageName);

    // Add page-specific event handlers here
    if (pageName === 'main_menu.html') {
        initializeMainMenu();
    }
}

// Initialize main menu page
function initializeMainMenu() {
    const resumeBtn = document.getElementById('resume-project-btn');
    const openBtn = document.getElementById('open-project-btn');
    const newBtn = document.getElementById('new-project-btn');
    const tutorialBtn = document.getElementById('launch-tutorial-btn');
    const exitBtn = document.getElementById('exit-btn');

    if (resumeBtn) {
        resumeBtn.addEventListener('click', () => {
            // TODO: Implement resume project
            console.log('Resume project clicked');
        });
    }

    if (openBtn) {
        openBtn.addEventListener('click', () => {
            // TODO: Implement open project
            console.log('Open project clicked');
        });
    }

    if (newBtn) {
        newBtn.addEventListener('click', () => {
            // TODO: Implement new project
            console.log('New project clicked');
        });
    }

    if (tutorialBtn) {
        tutorialBtn.addEventListener('click', () => {
            // TODO: Implement tutorial
            console.log('Tutorial clicked');
        });
    }

    if (exitBtn) {
        exitBtn.addEventListener('click', () => {
            // TODO: Implement exit
            console.log('Exit clicked');
        });
    }
}

// Event Handlers
connectBtn.addEventListener('click', () => {
    if (mikanClient) {
        disconnectFromMikan();
    } else {
        const url = serverUrlInput.value.trim();
        if (url) {
            connectToMikan(url);
        }
    }
});

launchBtn.addEventListener('click', async () => {
    launchBtn.disabled = true;
    updateStatus('connecting');
    statusDiv.textContent = 'Launching Mikan...';

    try {
        const response = await nativeBridge.launchMikan();

        if (response.success) {
            // Wait a moment for Mikan to start, then connect
            setTimeout(() => {
                connectToMikan('ws://localhost:8080');
            }, 2000);
        } else {
            updateStatus('error', 'Failed to launch Mikan: ' + (response.error || 'Unknown error'));
            launchBtn.disabled = false;
        }
    } catch (error) {
        updateStatus('error', 'Failed to launch Mikan');
        launchBtn.disabled = false;
    }
});

// Cleanup on page unload
window.addEventListener('beforeunload', () => {
    disconnectFromMikan();
});

// Export for global access if needed
(window as any).mikanClient = mikanClient;
