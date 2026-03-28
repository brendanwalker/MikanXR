// This file is auto generated. DO NOT EDIT.

import { MikanRequest, MikanResponse } from './MikanAPITypes.js';

export class ArucoMarkerImageResponse extends MikanResponse {
  imageData: string = '';

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'imageData', type: 'string' }
  ];
}

export class GetArucoMarkerImageRequest extends MikanRequest {
  markerId: number = 0;
  imageSize: number = 200;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'markerId', type: 'int32' },
    { name: 'imageSize', type: 'int32' }
  ];
}

