// This file is auto generated. DO NOT EDIT.

import { MikanRequest, MikanResponse } from './MikanAPITypes.js';
import { MikanFunctionDescriptor } from './MikanFunctionTypes.js';

export class InvokeSystemFunctionRequest extends MikanRequest {
  ownerSystem: string = '';
  functionName: string = '';

  constructor() {
    super();
    this.requestTypeName = 'InvokeSystemFunctionRequest';
  }

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'ownerSystem', type: 'string' },
    { name: 'functionName', type: 'string' }
  ];
}

export class InvokeComponentFunctionRequest extends MikanRequest {
  ownerSystem: string = '';
  componentId: number = -1;
  functionName: string = '';

  constructor() {
    super();
    this.requestTypeName = 'InvokeComponentFunctionRequest';
  }

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'ownerSystem', type: 'string' },
    { name: 'componentId', type: 'int32' },
    { name: 'functionName', type: 'string' }
  ];
}

export class GetFunctionListRequest extends MikanRequest {
  systemFilter: string = '';
  componentFilter: string = '';

  constructor() {
    super();
    this.requestTypeName = 'GetFunctionListRequest';
  }

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'systemFilter', type: 'string' },
    { name: 'componentFilter', type: 'string' }
  ];
}

export class FunctionDescriptorResponse extends MikanResponse {
  descriptor_list: MikanFunctionDescriptor[] = [];

  constructor() {
    super();
    this.responseTypeName = 'FunctionDescriptorResponse';
  }

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'descriptor_list', type: 'MikanFunctionDescriptor', isArray: true }
  ];
}

