// This file is auto generated. DO NOT EDIT.

import { MikanComponentValues } from './MikanComponentTypes.js';

export class MikanTextureSourceValues extends MikanComponentValues {

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
  ];
}

export class MikanClientTextureSourceValues extends MikanTextureSourceValues {
  client_source: string = '';

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'client_source', type: 'string' }
  ];
}

export class MikanSpoutTextureSourceValues extends MikanTextureSourceValues {
  spout_source: string = '';

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'spout_source', type: 'string' }
  ];
}

