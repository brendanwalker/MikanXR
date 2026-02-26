// This file is auto generated. DO NOT EDIT.

import { MikanEvent } from './MikanAPITypes.js';
import { MikanPropertyValue } from './MikanPropertyTypes.js';

export const CLASS_ID_MIKAN_PROPERTY_UPDATE_EVENT = -3166715052004720697n;

export class MikanPropertyUpdateEvent extends MikanEvent {
  propertyValue: MikanPropertyValue = new MikanPropertyValue();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'propertyValue', type: 'MikanPropertyValue' }
  ];
}

