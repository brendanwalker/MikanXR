import { EventEmitter } from 'events';
import {
  MikanEvent,
  MikanDisconnectedEvent,
  MikanDisconnectCode,
  MikanPropertyUpdateEvent,
} from './types/index.js';
import {
  MikanAppStageChangedEvent,
} from './types/MikanRemoteControlEvents.js';
import { deserializeFromJsonString, TypeRegistry } from './Serialization/index.js';

export class MikanEventManager extends EventEmitter {
  private static readonly WEBSOCKET_DISCONNECT_EVENT = 'disconnect';

  constructor() {
    super();
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

      if (!parsed.eventTypeName) {
        console.error('Event missing eventTypeName');
        return null;
      }

      // Use TypeRegistry for proper deserialization when the event type is known
      const eventType = TypeRegistry.get(parsed.eventTypeName);
      if (eventType) {
        const event = new eventType();
        deserializeFromJsonString(message, event, eventType);
        return event as MikanEvent;
      }

      // Fallback: shallow-copy JSON fields for unknown event types
      const event: MikanEvent = {
        eventTypeName: parsed.eventTypeName
      };
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

  public onAppStageChanged(listener: (event: MikanAppStageChangedEvent) => void): this {
    return this.on('MikanAppStageChangedEvent', listener);
  }

  public onAnyEvent(listener: (event: MikanEvent) => void): this {
    return this.on('event', listener);
  }
}
