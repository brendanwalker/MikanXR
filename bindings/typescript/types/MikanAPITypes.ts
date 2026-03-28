// This file is auto generated. DO NOT EDIT.

export enum MikanAPIResult {
  Success = 0,
  GeneralError = 1,
  Uninitialized = 2,
  NullParam = 3,
  InvalidParam = 4,
  RequestFailed = 5,
  NotConnected = 6,
  AlreadyConnected = 7,
  SocketError = 8,
  Timeout = 9,
  Canceled = 10,
  NoData = 11,
  BufferTooSmall = 12,
  UnknownClient = 13,
  UnknownFunction = 14,
  MalformedParameters = 15,
  MalformedResponse = 16,
  NoVideoSource = 100,
  NoVideoSourceAssignedTracker = 101,
  InvalidDeviceId = 102,
  InvalidStencilID = 103,
  InvalidAnchorID = 104,
  InvalidCameraID = 105,
  InvalidSceneID = 106,
  InvalidStageID = 106
}

export class MikanResponse {
  responseTypeName: string = '';
  requestId: number = -1;
  resultCode: MikanAPIResult = MikanAPIResult.Success;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'responseTypeName', type: 'string' },
    { name: 'requestId', type: 'int32' },
    { name: 'resultCode', type: 'enum:MikanAPIResult' }
  ];
}

export class MikanEvent {
  eventTypeName: string = '';

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'eventTypeName', type: 'string' }
  ];
}

export class MikanRequest {
  requestTypeName: string = '';
  requestId: number = -1;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'requestTypeName', type: 'string' },
    { name: 'requestId', type: 'int32' }
  ];
}

