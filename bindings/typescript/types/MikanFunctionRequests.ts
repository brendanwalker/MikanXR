// This file is auto generated. DO NOT EDIT.

import { MikanRequest, MikanResponse } from './MikanAPITypes.js';
import { MikanFunctionDescriptor } from './MikanFunctionTypes.js';

export const CLASS_ID_FUNCTION_DESCRIPTOR_RESPONSE = -2965742291485466747n;
export const CLASS_ID_GET_FUNCTION_LIST_REQUEST = 5674758152299327690n;
export const CLASS_ID_INVOKE_COMPONENT_FUNCTION_REQUEST = -7601310242316033921n;
export const CLASS_ID_INVOKE_SYSTEM_FUNCTION_REQUEST = 5600332656811306803n;

export class FunctionDescriptorResponse extends MikanResponse {
  descriptor_list: MikanFunctionDescriptor[] = [];

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'descriptor_list', type: 'MikanFunctionDescriptor', isArray: true }
  ];
}

export class GetFunctionListRequest extends MikanRequest {
  systemFilter: string = '';
  componentFilter: string = '';

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'systemFilter', type: 'string' },
    { name: 'componentFilter', type: 'string' }
  ];
}

export class InvokeComponentFunctionRequest extends MikanRequest {
  ownerSystem: string = '';
  componentId: number = -1;
  functionName: string = '';

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'ownerSystem', type: 'string' },
    { name: 'componentId', type: 'int32' },
    { name: 'functionName', type: 'string' }
  ];
}

export class InvokeSystemFunctionRequest extends MikanRequest {
  ownerSystem: string = '';
  functionName: string = '';

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'ownerSystem', type: 'string' },
    { name: 'functionName', type: 'string' }
  ];
}

