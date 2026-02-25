import { MikanRequestManager } from './MikanRequestManager';
import { MikanEventManager } from './MikanEventManager';
import { MikanResponseFuture } from './MikanResponseFuture';
import {
  MikanRequest,
  MikanAPIResult,
  MikanLogLevel,
  MikanClientInfo,
  MikanClientGraphicsApi,
  MikanConstants
} from './types';

export type MikanLogCallback = (level: MikanLogLevel, message: string) => void;

export interface MikanClientOptions {
  host?: string;
  port?: string;
  autoReconnect?: boolean;
  reconnectInterval?: number;
}

export class MikanClient {
  private ws: WebSocket | null = null;
  private requestManager: MikanRequestManager;
  private eventManager: MikanEventManager;
  private logCallback: MikanLogCallback | null = null;
  private isInitialized: boolean = false;
  private isConnected: boolean = false;
  private clientId: string = '';
  private options: MikanClientOptions;

  constructor(options: MikanClientOptions = {}) {
    this.options = {
      host: options.host || 'localhost',
      port: options.port || '8080',
      autoReconnect: options.autoReconnect ?? false,
      reconnectInterval: options.reconnectInterval ?? 5000
    };

    this.requestManager = new MikanRequestManager();
    this.eventManager = new MikanEventManager();
  }

  // Initialization
  public initialize(minLogLevel: MikanLogLevel = MikanLogLevel.Info): MikanAPIResult {
    if (this.isInitialized) {
      return MikanAPIResult.Success;
    }

    this.clientId = this.generateClientId();
    this.isInitialized = true;

    this.log(MikanLogLevel.Info, `MikanClient initialized with ID: ${this.clientId}`);

    return MikanAPIResult.Success;
  }

  public getIsInitialized(): boolean {
    return this.isInitialized;
  }

  public shutdown(): void {
    if (this.isConnected) {
      this.disconnect();
    }

    this.isInitialized = false;
    this.clientId = '';
  }

  // Client Info
  public getClientAPIVersion(): number {
    return MikanConstants.ClientAPIVersion;
  }

  public getClientUniqueID(): string {
    return this.clientId;
  }

  public createClientInfo(
    engineName: string = 'Electron',
    engineVersion: string = '1.0.0',
    applicationName: string = 'MikanXR',
    applicationVersion: string = '1.0.0',
    xrDeviceName: string = 'Unknown'
  ): MikanClientInfo {
    return {
      clientId: this.clientId,
      engineName,
      engineVersion,
      applicationName,
      applicationVersion,
      xrDeviceName,
      graphicsAPI: MikanClientGraphicsApi.UNKNOWN,
      supportsRGB24: false,
      supportsRGBA32: false,
      supportsBGRA32: false,
      supportsDepth: false
    };
  }

  // Connection
  public async connect(): Promise<MikanAPIResult> {
    if (!this.isInitialized) {
      return MikanAPIResult.Uninitialized;
    }

    if (this.isConnected) {
      return MikanAPIResult.AlreadyConnected;
    }

    return new Promise((resolve) => {
      try {
        const url = `ws://${this.options.host}:${this.options.port}`;
        const protocol = `Mikan-${MikanConstants.ClientAPIVersion}`;
        this.log(MikanLogLevel.Info, `Connecting to ${url} with protocol ${protocol}`);

        this.ws = new WebSocket(url, protocol);

        this.ws.onopen = () => {
          this.isConnected = true;
          this.requestManager.setSendFunction((message: string) => {
            if (this.ws && this.ws.readyState === WebSocket.OPEN) {
              this.ws.send(message);
            }
          });

          this.log(MikanLogLevel.Info, 'Connected to Mikan server');
          resolve(MikanAPIResult.Success);
        };

        this.ws.onmessage = (event) => {
          this.handleMessage(event.data);
        };

        this.ws.onerror = (error) => {
          this.log(MikanLogLevel.Error, `WebSocket error: ${error}`);
          if (!this.isConnected) {
            resolve(MikanAPIResult.SocketError);
          }
        };

        this.ws.onclose = (event) => {
          this.log(MikanLogLevel.Info, `Disconnected from Mikan server: ${event.code} ${event.reason}`);
          this.isConnected = false;
          this.requestManager.setSendFunction(null);
          this.requestManager.cancelAllRequests();

          // Emit disconnect event
          const disconnectMessage = `disconnect:${event.code}:${event.reason}`;
          this.eventManager.handleEventMessage(disconnectMessage);

          if (!this.isConnected) {
            resolve(MikanAPIResult.SocketError);
          }
        };
      } catch (error) {
        this.log(MikanLogLevel.Error, `Failed to connect: ${error}`);
        resolve(MikanAPIResult.SocketError);
      }
    });
  }

  public getIsConnected(): boolean {
    return this.isConnected && this.ws !== null && this.ws.readyState === WebSocket.OPEN;
  }

  public disconnect(): MikanAPIResult {
    if (!this.isConnected || !this.ws) {
      return MikanAPIResult.NotConnected;
    }

    this.ws.close(1000, 'Client disconnect');
    this.ws = null;
    this.isConnected = false;

    return MikanAPIResult.Success;
  }

  // Messaging
  public sendRequest(request: MikanRequest): MikanResponseFuture {
    return this.requestManager.sendRequest(request);
  }

  private handleMessage(data: string): void {
    // Try to determine if this is a response or an event
    try {
      const parsed = JSON.parse(data);

      if (parsed.responseTypeName) {
        // This is a response
        this.requestManager.handleResponse(data);
      } else if (parsed.eventTypeName) {
        // This is an event
        this.eventManager.handleEventMessage(data);
      } else {
        this.log(MikanLogLevel.Warning, `Unknown message type: ${data}`);
      }
    } catch (error) {
      // If not JSON, might be a disconnect event
      if (data.startsWith('disconnect')) {
        this.eventManager.handleEventMessage(data);
      } else {
        this.log(MikanLogLevel.Error, `Failed to parse message: ${error}`);
      }
    }
  }

  // Event Manager Access
  public get events(): MikanEventManager {
    return this.eventManager;
  }

  // Logging
  public setLogCallback(callback: MikanLogCallback): void {
    this.logCallback = callback;
  }

  private log(level: MikanLogLevel, message: string): void {
    if (this.logCallback) {
      this.logCallback(level, message);
    } else {
      console.log(`[Mikan ${MikanLogLevel[level]}] ${message}`);
    }
  }

  // Utility
  private generateClientId(): string {
    return `electron-client-${Date.now()}-${Math.random().toString(36).substr(2, 9)}`;
  }
}
