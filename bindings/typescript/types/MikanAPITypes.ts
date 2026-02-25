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

export const CLASS_ID_MIKAN_EVENT = 8521159033538382795n;
export const CLASS_ID_MIKAN_REQUEST = 1095719431187359814n;
export const CLASS_ID_MIKAN_RESPONSE = 7094118849615581562n;

export interface MikanEvent {
  eventTypeId: bigint;
  eventTypeName: string;
}

export interface MikanRequest {
  requestTypeId: bigint;
  requestTypeName: string;
  requestId: number;
}

export interface MikanResponse {
  responseTypeId: bigint;
  responseTypeName: string;
  requestId: number;
  resultCode: MikanAPIResult;
}

