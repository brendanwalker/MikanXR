import { EventEmitter } from 'events';
import {
  MikanEvent,
  MikanDisconnectedEvent,
  MikanDisconnectCode,
  MikanPropertyUpdateEvent,
  CLASS_ID_MIKAN_DISCONNECTED_EVENT
} from './types/index.js';
import { deserializeFromJsonString, TypeRegistry } from './Serialization/index.js';

export class MikanEventManager extends EventEmitter {
  private static readonly WEBSOCKET_DISCONNECT_EVENT = 'disconnect';
  private eventTypeCache: Map<string, bigint> = new Map();

  constructor() {
    super();
    this.buildEventTypeCache();
  }

  private buildEventTypeCache(): void {
    // Map event type names to their class IDs
    this.eventTypeCache.set('MikanDisconnectedEvent', CLASS_ID_MIKAN_DISCONNECTED_EVENT);
    // Add more event types as needed
  }

  public handleEventMessage(message: string): void {
    const event = this.parseEvent(message);

    if (event) {
      // Emit the specific event type
      this.emit(event.eventTypeName, event);

      // Also emit a generic 'event' for all events
      this.emit('event', event);
    }
  }

  private parseEvent(message: string): MikanEvent | null {
    try {
      // Handle special disconnect event format
      if (message.startsWith(MikanEventManager.WEBSOCKET_DISCONNECT_EVENT)) {
        return this.parseDisconnectEvent(message);
      }

      // Parse JSON events
      const parsed = JSON.parse(message);

      if (!parsed.eventTypeName || !parsed.eventTypeId) {
        console.error('Event missing type information');
        return null;
      }

      // Convert eventTypeId string back to bigint
      if (typeof parsed.eventTypeId === 'string') {
        parsed.eventTypeId = BigInt(parsed.eventTypeId);
      }

      // NOTE: For advanced serialization with complex types, you can use:
      // const eventType = TypeRegistry.get(parsed.eventTypeName);
      // if (eventType) {
      //   const event = new eventType();
      //   deserializeFromJsonString(message, event, eventType);
      //   return event;
      // }

      const event: MikanEvent = {
        eventTypeId: parsed.eventTypeId,
        eventTypeName: parsed.eventTypeName
      };

      // Copy any additional properties from specific event types
      Object.assign(event, parsed);

      return event;
    } catch (error) {
      console.error('Failed to parse event:', error);
      return null;
    }
  }

  private parseDisconnectEvent(message: string): MikanDisconnectedEvent {
    let disconnectCode = MikanDisconnectCode.Normal;
    let disconnectReason = '';

    const tokens = message.split(':');
    if (tokens.length >= 3) {
      disconnectCode = parseInt(tokens[1]) as MikanDisconnectCode;
      disconnectReason = tokens[2];
    }

    return {
      eventTypeId: CLASS_ID_MIKAN_DISCONNECTED_EVENT,
      eventTypeName: 'MikanDisconnectedEvent',
      code: disconnectCode,
      reason: disconnectReason
    };
  }

  // Typed event listeners
  public onConnected(listener: (event: MikanEvent) => void): this {
    return this.on('MikanConnectedEvent', listener);
  }

  public onDisconnected(listener: (event: MikanDisconnectedEvent) => void): this {
    return this.on('MikanDisconnectedEvent', listener);
  }

  public onAnchorListUpdate(listener: (event: MikanEvent) => void): this {
    return this.on('MikanAnchorListUpdateEvent', listener);
  }

  public onAnchorNameUpdate(listener: (event: any) => void): this {
    return this.on('MikanAnchorNameUpdateEvent', listener);
  }

  public onAnchorPoseUpdate(listener: (event: any) => void): this {
    return this.on('MikanAnchorPoseUpdateEvent', listener);
  }

  public onPropertyUpdate(listener: (event: MikanPropertyUpdateEvent) => void): this {
    return this.on('MikanPropertyUpdateEvent', listener);
  }

  public onAnyEvent(listener: (event: MikanEvent) => void): this {
    return this.on('event', listener);
  }
}
