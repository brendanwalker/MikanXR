// Bridge for communicating between JavaScript and C++ (CEF)

interface CEFQuery {
    request: string;
    onSuccess: (response: string) => void;
    onFailure: (errorCode: number, errorMessage: string) => void;
}

declare global {
    interface Window {
        cefQuery?: (query: CEFQuery) => void;
    }
}

export interface NativeResponse {
    success: boolean;
    data?: {
        filePath?: string;
        canceled?: boolean;
        [key: string]: any;
    };
    error?: string;
}

export class NativeBridge {
    private callbacks: Map<number, (response: NativeResponse) => void> = new Map();
    private callbackId: number = 0;

    constructor() {
        // Listen for messages from C++
        window.addEventListener('message', (event) => {
            if (event.data && event.data.callbackId) {
                const callback = this.callbacks.get(event.data.callbackId);
                if (callback) {
                    callback(event.data);
                    this.callbacks.delete(event.data.callbackId);
                }
            }
        });
    }

    // Send a message to C++ with optional callback
    sendMessage(type: string, data: any = {}, callback?: (response: NativeResponse) => void): void {
        const message: any = { type, ...data };

        if (callback) {
            const callbackId = ++this.callbackId;
            message.callbackId = callbackId;
            this.callbacks.set(callbackId, callback);
        }

        // CEF provides the window.cefQuery function for JS->C++ communication
        if (window.cefQuery) {
            window.cefQuery({
                request: JSON.stringify(message),
                onSuccess: (response: string) => {
                    if (callback) {
                        try {
                            const parsedResponse = JSON.parse(response);
                            callback(parsedResponse);
                        } catch (e) {
                            callback({ success: true, data: response });
                        }
                    }
                },
                onFailure: (errorCode: number, errorMessage: string) => {
                    console.error('CEF query failed:', errorCode, errorMessage);
                    if (callback) {
                        callback({ success: false, error: errorMessage });
                    }
                }
            });
        } else {
            console.warn('cefQuery not available, running in browser mode');
            if (callback) {
                callback({ success: false, error: 'Not in CEF environment' });
            }
        }
    }

    // Launch Mikan.exe
    launchMikan(): Promise<NativeResponse> {
        return new Promise((resolve) => {
            this.sendMessage('launch_mikan', {}, resolve);
        });
    }

    // Get Mikan installation path
    getMikanPath(): Promise<NativeResponse> {
        return new Promise((resolve) => {
            this.sendMessage('get_mikan_path', {}, resolve);
        });
    }

    // Show a native file open dialog
    showOpenFileDialog(options?: {
        title?: string;
        defaultPath?: string;
        filter?: string;
        filterDescription?: string;
    }): Promise<NativeResponse> {
        return new Promise((resolve) => {
            this.sendMessage('open_file_dialog', options || {}, resolve);
        });
    }

    // Show a native file save dialog
    showSaveFileDialog(options?: {
        title?: string;
        defaultPath?: string;
        filter?: string;
        filterDescription?: string;
    }): Promise<NativeResponse> {
        return new Promise((resolve) => {
            this.sendMessage('save_file_dialog', options || {}, resolve);
        });
    }
}

export const nativeBridge = new NativeBridge();
