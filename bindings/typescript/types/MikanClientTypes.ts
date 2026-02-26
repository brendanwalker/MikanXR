// This file is auto generated. DO NOT EDIT.

import { MikanClientGraphicsApi } from './MikanCoreConstants';

export const CLASS_ID_MIKAN_CLIENT_INFO = -2302096980379213470n;

export class MikanClientInfo {
  clientId: string = '';
  engineName: string = '';
  engineVersion: string = '';
  applicationName: string = '';
  applicationVersion: string = '';
  xrDeviceName: string = '';
  graphicsAPI: MikanClientGraphicsApi = MikanClientGraphicsApi.UNKNOWN;
  supportsRGB24: boolean = false;
  supportsRGBA32: boolean = false;
  supportsBGRA32: boolean = false;
  supportsDepth: boolean = false;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'clientId', type: 'string' },
    { name: 'engineName', type: 'string' },
    { name: 'engineVersion', type: 'string' },
    { name: 'applicationName', type: 'string' },
    { name: 'applicationVersion', type: 'string' },
    { name: 'xrDeviceName', type: 'string' },
    { name: 'graphicsAPI', type: 'enum:MikanClientGraphicsApi' },
    { name: 'supportsRGB24', type: 'boolean' },
    { name: 'supportsRGBA32', type: 'boolean' },
    { name: 'supportsBGRA32', type: 'boolean' },
    { name: 'supportsDepth', type: 'boolean' }
  ];
}

