// This file is auto generated. DO NOT EDIT.

import { MikanEvent } from './MikanAPITypes';
import { MikanPropertyValue } from './MikanPropertyTypes';

export const CLASS_ID_MIKAN_PROPERTY_UPDATE_EVENT = -3166715052004720697n;

export class MikanPropertyUpdateEvent extends MikanEvent {
  propertyValue: MikanPropertyValue = new MikanPropertyValue();

  static __serializationMetadata = [
    { name: 'propertyValue', type: 'MikanPropertyValue' }
  ];
}

