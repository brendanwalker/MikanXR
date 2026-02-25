// Mikan WebSocket Client
class MikanWebSocket {
    constructor(url = 'ws://localhost:8080') {
        this.url = url;
        this.ws = null;
        this.reconnectInterval = 3000;
        this.reconnectTimer = null;
        this.messageHandlers = new Map();
        this.onStatusChange = null;
    }

    connect() {
        try {
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
                    this.onStatusChange('error');
                }
            };

            this.ws.onclose = () => {
                console.log('Disconnected from Mikan');
                if (this.onStatusChange) {
                    this.onStatusChange('disconnected');
                }
                this.scheduleReconnect();
            };
        } catch (e) {
            console.error('Failed to connect:', e);
            this.scheduleReconnect();
        }
    }

    scheduleReconnect() {
        if (!this.reconnectTimer) {
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

    on(messageType, handler) {
        if (!this.messageHandlers.has(messageType)) {
            this.messageHandlers.set(messageType, []);
        }
        this.messageHandlers.get(messageType).push(handler);
    }

    handleMessage(message) {
        const handlers = this.messageHandlers.get(message.type);
        if (handlers) {
            handlers.forEach(handler => handler(message));
        }
    }

    disconnect() {
        if (this.reconnectTimer) {
            clearTimeout(this.reconnectTimer);
            this.reconnectTimer = null;
        }
        if (this.ws) {
            this.ws.close();
            this.ws = null;
        }
    }
}
