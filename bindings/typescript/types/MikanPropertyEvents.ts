// This file is auto generated. DO NOT EDIT.

import { MikanEvent } from './MikanAPITypes.js';
import { MikanPropertyValue } from './MikanPropertyTypes.js';

export class MikanPropertyUpdateEvent extends MikanEvent {
  propertyValue: MikanPropertyValue = new MikanPropertyValue();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'propertyValue', type: 'MikanPropertyValue' }
  ];
}

