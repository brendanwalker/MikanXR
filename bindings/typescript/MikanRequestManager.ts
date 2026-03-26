import {
  MikanRequest,
  MikanResponse,
  MikanAPIResult,
  CLASS_ID_MIKAN_RESPONSE
} from './types/index.js';
import { MikanResponseFuture } from './MikanResponseFuture.js';
import {
  serializeToJsonString,
  deserializeFromJsonString,
  deserializeFromBytes,
  TypeRegistry
} from './Serialization/index.js';
import { BinaryReader } from './Serialization/BinaryReader.js';

interface PendingRequest {
  requestId: number;
  resolve: (response: MikanResponse) => void;
  reject: (error: Error) => void;
}

export class MikanRequestManager {
  private nextRequestId: number = 0;
  private pendingRequests: Map<number, PendingRequest> = new Map();
  private responseTypeCache: Map<string, bigint> = new Map();
  private sendFunction: ((message: string) => void) | null = null;

  constructor() {
    this.buildResponseTypeCache();
  }

  private buildResponseTypeCache(): void {
    // Map response type names to their class IDs
    // This will be populated as we add more response types
    this.responseTypeCache.set('MikanResponse', CLASS_ID_MIKAN_RESPONSE);
  }

  public setSendFunction(sendFn: ((message: string) => void) | null): void {
    this.sendFunction = sendFn;
  }

  public sendRequest(request: MikanRequest): MikanResponseFuture {
    // Validate that request has required fields
    if (!request.requestTypeName || request.requestTypeId === undefined) {
      throw new Error('Request must have requestTypeName and requestTypeId');
    }

    // Stamp with next request id (only field we need to set)
    request.requestId = this.nextRequestId++;

    // Create promise for response
    const promise = new Promise<MikanResponse>((resolve, reject) => {
      if (!this.sendFunction) {
        const errorResponse: MikanResponse = {
          responseTypeId: CLASS_ID_MIKAN_RESPONSE,
          responseTypeName: 'MikanResponse',
          requestId: request.requestId,
          resultCode: MikanAPIResult.NotConnected
        };
        resolve(errorResponse);
        return;
      }

      // Store pending request
      this.pendingRequests.set(request.requestId, {
        requestId: request.requestId,
        resolve,
        reject
      });

      try {
        // Add requestType field for compatibility with server
        // Server expects both requestType (string) and requestTypeId (bigint as string)
        const requestWithType = {
          ...request,
          requestType: request.requestTypeName
        };

        // NOTE: For advanced serialization with complex types, you can use:
        // const jsonString = serializeToJsonString(request, request.constructor);

        // Serialize and send
        // Convert bigints to strings first, then manually replace with unquoted numbers in JSON
        let jsonString = JSON.stringify(requestWithType, (key, value) => {
          if (typeof value === 'bigint') {
            // Use a special marker that we'll replace
            return `__BIGINT__${value.toString()}__BIGINT__`;
          }
          return value;
        });

        // Replace the quoted bigint markers with actual JSON numbers (handles both positive and negative)
        jsonString = jsonString.replace(/"__BIGINT__(-?\d+)__BIGINT__"/g, '$1');

        this.sendFunction(jsonString);
      } catch (error) {
        this.pendingRequests.delete(request.requestId);
        reject(error);
      }
    });

    return new MikanResponseFuture(promise, request.requestId);
  }

  public handleResponse(responseJson: string): void {
    try {
      const response = this.parseResponse(responseJson);

      if (response) {
        const pendingRequest = this.pendingRequests.get(response.requestId);

        if (pendingRequest) {
          this.pendingRequests.delete(response.requestId);
          pendingRequest.resolve(response);
        }
      }
    } catch (error) {
      console.error('Failed to handle response:', error);
    }
  }

  private parseResponse(responseJson: string): MikanResponse | null {
    try {
      const parsed = JSON.parse(responseJson);

      if (!parsed.responseTypeName || !parsed.responseTypeId) {
        console.error('Response missing type information');
        return null;
      }

      // Convert responseTypeId string back to bigint
      if (typeof parsed.responseTypeId === 'string') {
        parsed.responseTypeId = BigInt(parsed.responseTypeId);
      }

      // Use visitor-based deserialization when the response type is registered.
      // This correctly handles nested PolymorphicObject fields (e.g. valuesObject)
      // by calling setInstance() so that .instance returns the proper typed object.
      const responseType = TypeRegistry.get(parsed.responseTypeName);
      if (responseType) {
        const response = new responseType();
        deserializeFromJsonString(responseJson, response, responseType);
        return response as MikanResponse;
      }

      // Fallback for unknown response types: shallow-copy JSON fields.
      // PolymorphicObject fields will be plain objects ({class_id, class_name, value})
      // rather than proper PolymorphicObject instances in this path.

      // Parse resultCode - can be either a number or string enum name
      let resultCode = MikanAPIResult.Success;
      if (parsed.resultCode !== undefined) {
        if (typeof parsed.resultCode === 'string') {
          // Map string enum name to numeric value
          resultCode = (MikanAPIResult as any)[parsed.resultCode];
          if (resultCode === undefined) {
            console.warn(`Unknown resultCode string: ${parsed.resultCode}`);
            resultCode = MikanAPIResult.GeneralError;
          }
        } else {
          resultCode = Number(parsed.resultCode);
        }
      }

      const response: MikanResponse = {
        responseTypeId: parsed.responseTypeId,
        responseTypeName: parsed.responseTypeName,
        requestId: parsed.requestId,
        resultCode: resultCode
      };

      // Copy any additional properties from specific response types
      Object.assign(response, parsed);

      // Restore the numeric resultCode after Object.assign
      response.resultCode = resultCode;

      return response;
    } catch (error) {
      console.error('Failed to parse response:', error);
      return null;
    }
  }

  /**
   * Handle binary response callback (similar to C# InternalBinaryResponseCallback)
   * This would be called when receiving binary websocket messages
   */
  public handleBinaryResponse(buffer: ArrayBuffer): void {
    try {
      const managedBuffer = new Uint8Array(buffer);
      const binaryReader = new BinaryReader(managedBuffer);

      // Read the response type id
      const responseTypeId = binaryReader.readInt64();

      // Read the response type name
      const responseTypeName = binaryReader.readUTF8String();

      // Read the request ID
      const requestId = binaryReader.readInt32();

      // Read the result code
      const resultCode = binaryReader.readInt32() as MikanAPIResult;

      // Look up the pending request
      const pendingRequest = this.pendingRequests.get(requestId);

      // Bail if the corresponding pending request is not found
      if (!pendingRequest) {
        console.error(`Invalid pending request id(${requestId}) for response type ${responseTypeName}`);
        return;
      }

      // Remove from pending requests
      this.pendingRequests.delete(requestId);

      // Attempt to create the response object by class name
      let response: MikanResponse | null = null;
      const responseType = TypeRegistry.get(responseTypeName);

      if (responseType) {
        const responseObject = new responseType();

        // Deserialize the response object from the byte array
        if (deserializeFromBytes(managedBuffer, responseObject, responseType)) {
          response = responseObject as MikanResponse;
        } else {
          console.error('Failed to deserialize response object from byte array');
        }
      } else {
        console.error(`Unknown response type: ${responseTypeName}`);
      }

      if (!response) {
        response = {
          responseTypeId: CLASS_ID_MIKAN_RESPONSE,
          responseTypeName: 'MikanResponse',
          requestId: requestId,
          resultCode: MikanAPIResult.MalformedResponse
        };
      }

      pendingRequest.resolve(response);
    } catch (error) {
      console.error('Malformed binary response:', error);
    }
  }

  public cancelRequest(requestId: number): MikanAPIResult {
    const pendingRequest = this.pendingRequests.get(requestId);

    if (pendingRequest) {
      this.pendingRequests.delete(requestId);
      pendingRequest.reject(new Error('Request cancelled'));
      return MikanAPIResult.Success;
    }

    return MikanAPIResult.InvalidParam;
  }

  public cancelAllRequests(): void {
    for (const [requestId, pendingRequest] of this.pendingRequests) {
      pendingRequest.reject(new Error('All requests cancelled'));
    }
    this.pendingRequests.clear();
  }
}
