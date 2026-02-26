// Mikan WebSocket Client
class MikanWebSocket {
    constructor() {
        this.url = null;
        this.ws = null;
        this.reconnectInterval = 3000;
        this.reconnectTimer = null;
        this.messageHandlers = new Map();
        this.onStatusChange = null;
        this.autoReconnect = false;
        this.requestId = 0;
        this.pendingRequests = new Map();
    }

    connect(url) {
        this.url = url || this.url;
        if (!this.url) {
            console.error('No URL provided');
            if (this.onStatusChange) {
                this.onStatusChange('error', 'No URL provided');
            }
            return;
        }

        try {
            if (this.onStatusChange) {
                this.onStatusChange('connecting');
            }

            this.ws = new WebSocket(this.url);

            this.ws.onopen = () => {
                console.log('Connected to Mikan');
                if (this.onStatusChange) {
                    this.onStatusChange('connected');
                }
                if (this.reconnectTimer) {
                    clearTimeout(this.reconnectTimer);
                    this.reconnectTimer = null;
                }
            };

            this.ws.onmessage = (event) => {
                try {
                    const message = JSON.parse(event.data);
                    this.handleMessage(message);
                } catch (e) {
                    console.error('Failed to parse message:', e);
                }
            };

            this.ws.onerror = (error) => {
                console.error('WebSocket error:', error);
                if (this.onStatusChange) {
                    this.onStatusChange('error', 'Connection failed');
                }
            };

            this.ws.onclose = () => {
                console.log('Disconnected from Mikan');
                if (this.onStatusChange) {
                    this.onStatusChange('disconnected');
                }
                if (this.autoReconnect) {
                    this.scheduleReconnect();
                }
            };
        } catch (e) {
            console.error('Failed to connect:', e);
            if (this.onStatusChange) {
                this.onStatusChange('error', e.message);
            }
            if (this.autoReconnect) {
                this.scheduleReconnect();
            }
        }
    }

    scheduleReconnect() {
        if (!this.reconnectTimer && this.autoReconnect) {
            this.reconnectTimer = setTimeout(() => {
                console.log('Attempting to reconnect...');
                this.connect();
            }, this.reconnectInterval);
        }
    }

    send(type, data = {}) {
        if (this.ws && this.ws.readyState === WebSocket.OPEN) {
            const message = { type, ...data };
            this.ws.send(JSON.stringify(message));
        } else {
            console.warn('WebSocket not connected, cannot send message');
        }
    }

    // Send a request and get a promise for the response
    async request(type, data = {}) {
        return new Promise((resolve, reject) => {
            if (this.ws && this.ws.readyState === WebSocket.OPEN) {
                const requestId = ++this.requestId;
                const message = { type, requestId, ...data };

                this.pendingRequests.set(requestId, { resolve, reject });

                this.ws.send(JSON.stringify(message));

                // Timeout after 10 seconds
                setTimeout(() => {
                    if (this.pendingRequests.has(requestId)) {
                        this.pendingRequests.delete(requestId);
                        reject(new Error('Request timeout'));
                    }
                }, 10000);
            } else {
                reject(new Error('WebSocket not connected'));
            }
        });
    }

    on(messageType, handler) {
        if (!this.messageHandlers.has(messageType)) {
            this.messageHandlers.set(messageType, []);
        }
        this.messageHandlers.get(messageType).push(handler);
    }

    handleMessage(message) {
        // Check if this is a response to a pending request
        if (message.requestId && this.pendingRequests.has(message.requestId)) {
            const { resolve } = this.pendingRequests.get(message.requestId);
            this.pendingRequests.delete(message.requestId);
            resolve(message);
            return;
        }

        // Otherwise, call registered handlers
        const handlers = this.messageHandlers.get(message.type);
        if (handlers) {
            handlers.forEach(handler => handler(message));
        }
    }

    disconnect() {
        this.autoReconnect = false;
        if (this.reconnectTimer) {
            clearTimeout(this.reconnectTimer);
            this.reconnectTimer = null;
        }
        if (this.ws) {
            this.ws.close();
            this.ws = null;
        }
    }

    isConnected() {
        return this.ws && this.ws.readyState === WebSocket.OPEN;
    }
}
