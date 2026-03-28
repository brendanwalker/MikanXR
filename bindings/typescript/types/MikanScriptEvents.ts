// This file is auto generated. DO NOT EDIT.

import { MikanEvent } from './MikanAPITypes.js';

export class MikanScriptMessagePostedEvent extends MikanEvent {
  message: string = '';

  constructor() {
    super();
    this.eventTypeName = 'MikanScriptMessagePostedEvent';
  }

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'message', type: 'string' }
  ];
}

