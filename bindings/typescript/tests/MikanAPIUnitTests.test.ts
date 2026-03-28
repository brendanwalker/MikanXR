/**
 * Mikan API unit tests for TypeScript bindings
 * Mirrors the C++ tests in mikan_api_unit_tests.cpp
 */

import {
  serializeToJsonString,
  deserializeFromJsonString,
  TypeRegistry
} from '../Serialization';

import {
  MikanRequest,
  MikanResponse,
  MikanEvent,
  MikanAPIResult
} from '../types/MikanAPITypes';

import {
  MikanRemoteControlCommand,
} from '../types/MikanRemoteControlRequest';

import { registerAllTypes } from '../types/TypeRegistration';

// Populate the TypeRegistry before any tests run.
// This is the TS equivalent of TypeRegistry::buildFromRfkDatabase().
beforeAll(() => {
  registerAllTypes();
});

describe('MikanAPIUnitTests', () => {

  // ---- TypeRegistry tests -------------------------------------------------
  // Mirrors: mikan_api_test_type_registry_build,
  //          mikan_api_test_type_registry_lookup,
  //          mikan_api_test_type_registry_unknown_returns_null

  describe('TypeRegistry', () => {
    it('should contain core MikanRequest / MikanResponse / MikanEvent after registration', () => {
      expect(TypeRegistry.has('MikanRequest')).toBe(true);
      expect(TypeRegistry.has('MikanResponse')).toBe(true);
      expect(TypeRegistry.has('MikanEvent')).toBe(true);
    });

    it('should contain known concrete request type MikanRemoteControlCommand', () => {
      expect(TypeRegistry.has('MikanRemoteControlCommand')).toBe(true);
    });

    it('should contain known response type MikanRemoteControlCommandResult', () => {
      expect(TypeRegistry.has('MikanRemoteControlCommandResult')).toBe(true);
    });

    it('should contain known event type MikanDisconnectedEvent', () => {
      expect(TypeRegistry.has('MikanDisconnectedEvent')).toBe(true);
    });

    it('retrieved constructor for MikanRemoteControlCommand should produce a correctly named instance', () => {
      const Ctor = TypeRegistry.get('MikanRemoteControlCommand');
      expect(Ctor).toBeDefined();

      const instance = new Ctor!();
      expect(instance.constructor.name).toBe('MikanRemoteControlCommand');
    });

    it('should map MikanResponse to the correct constructor', () => {
      const Ctor = TypeRegistry.get('MikanResponse');
      expect(Ctor).toBe(MikanResponse);
    });

    it('should return undefined for unknown type names', () => {
      expect(TypeRegistry.get('ThisTypeDoesNotExist')).toBeUndefined();
      expect(TypeRegistry.has('ThisTypeDoesNotExist')).toBe(false);
    });
  });

  // ---- Constructor name tests ---------------------------------------------
  // Mirrors: mikan_api_test_type_name_set_on_construct

  describe('type name on constructed objects', () => {
    it('MikanRequest.name should be "MikanRequest"', () => {
      expect(MikanRequest.name).toBe('MikanRequest');
    });

    it('MikanResponse.name should be "MikanResponse"', () => {
      expect(MikanResponse.name).toBe('MikanResponse');
    });

    it('MikanEvent.name should be "MikanEvent"', () => {
      expect(MikanEvent.name).toBe('MikanEvent');
    });

    it('MikanRemoteControlCommand.name should be the derived name, not the base', () => {
      expect(MikanRemoteControlCommand.name).toBe('MikanRemoteControlCommand');
    });

    it('stamping requestTypeName via constructor.name should give the concrete type name', () => {
      // Mirrors the MikanRequestManager.SendRequest() stamping logic
      const cmd = new MikanRemoteControlCommand();
      cmd.requestTypeName = cmd.constructor.name;
      expect(cmd.requestTypeName).toBe('MikanRemoteControlCommand');
    });
  });

  // ---- Wire format tests --------------------------------------------------
  // Mirrors: mikan_api_test_request_no_type_id_in_json

  describe('request JSON wire format', () => {
    it('should have requestTypeName but no requestTypeId in serialized JSON', () => {
      const cmd = new MikanRemoteControlCommand();
      cmd.requestTypeName = cmd.constructor.name;
      cmd.requestId = 1;
      cmd.command = 'test_command';

      const jsonString = serializeToJsonString(cmd, MikanRemoteControlCommand);
      const j = JSON.parse(jsonString);

      // requestTypeName must be present (needed for server routing)
      expect(j.requestTypeName).toBeDefined();
      expect(j.requestTypeName).toBe('MikanRemoteControlCommand');

      // requestTypeId must NOT be present (removed from wire format)
      expect(j.requestTypeId).toBeUndefined();
    });
  });

  // ---- Round-trip deserialization test ------------------------------------
  // Mirrors: mikan_api_test_response_round_trip

  describe('response JSON round-trip via TypeRegistry', () => {
    it('should deserialize a MikanResponse by looking up its type in the TypeRegistry', () => {
      // Simulate a server-produced response
      const expected = new MikanResponse();
      expected.responseTypeName = MikanResponse.name;
      expected.requestId = 99;
      expected.resultCode = MikanAPIResult.Success;

      const jsonString = serializeToJsonString(expected, MikanResponse);

      // Simulate client parsing: extract responseTypeName from JSON
      const j = JSON.parse(jsonString);
      const responseTypeName: string = j.responseTypeName;
      expect(responseTypeName).toBeTruthy();

      // Look up the concrete constructor by name and create an instance
      const ResponseCtor = TypeRegistry.get(responseTypeName);
      expect(ResponseCtor).toBeDefined();

      const responseObject = new ResponseCtor!();
      const bDeserialize = deserializeFromJsonString(jsonString, responseObject, ResponseCtor!);
      expect(bDeserialize).toBe(true);

      // Verify field values survived the round-trip
      const actual = responseObject as MikanResponse;
      expect(actual.requestId).toBe(expected.requestId);
      expect(actual.resultCode).toBe(expected.resultCode);
      expect(actual.responseTypeName).toBe(expected.responseTypeName);
    });
  });
});
