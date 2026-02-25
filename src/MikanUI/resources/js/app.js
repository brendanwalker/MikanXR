// Main application logic
const statusDiv = document.getElementById('status');
const controlsDiv = document.getElementById('controls');

// Initialize WebSocket connection
const mikanWS = new MikanWebSocket('ws://localhost:8080');

// Handle connection status changes
mikanWS.onStatusChange = (status) => {
    statusDiv.className = status;
    switch (status) {
        case 'connected':
            statusDiv.textContent = 'Connected to Mikan';
            break;
        case 'disconnected':
            statusDiv.textContent = 'Disconnected from Mikan - Reconnecting...';
            break;
        case 'error':
            statusDiv.textContent = 'Connection Error';
            break;
    }
};

// Example: Listen for video source updates
mikanWS.on('video_source_update', (message) => {
    console.log('Video source updated:', message);
    // Update UI based on message data
});

// Example: Send a command to Mikan
function sendTestCommand() {
    mikanWS.send('test_command', {
        param1: 'value1',
        param2: 'value2'
    });
}

// Connect on page load
mikanWS.connect();

// Cleanup on page unload
window.addEventListener('beforeunload', () => {
    mikanWS.disconnect();
});
