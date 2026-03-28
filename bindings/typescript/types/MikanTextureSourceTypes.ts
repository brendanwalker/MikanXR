// This file is auto generated. DO NOT EDIT.

import { MikanComponentValues } from './MikanComponentTypes.js';
import type { SerializationField } from './SerializationTypes.js';

export class MikanTextureSourceValues extends MikanComponentValues {

  static __serializationMetadata: SerializationField[] = [
  ];
}

export class MikanClientTextureSourceValues extends MikanTextureSourceValues {
  client_source: string = '';

  static __serializationMetadata: SerializationField[] = [
    { name: 'client_source', type: 'string' }
  ];
}

export class MikanSpoutTextureSourceValues extends MikanTextureSourceValues {
  spout_source: string = '';

  static __serializationMetadata: SerializationField[] = [
    { name: 'spout_source', type: 'string' }
  ];
}

