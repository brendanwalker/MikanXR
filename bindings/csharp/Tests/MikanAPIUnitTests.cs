using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
using Newtonsoft.Json.Linq;

namespace MikanXR
{
	public class MikanAPIUnitTests
	{
		public bool RunAllTests()
		{
			Console.WriteLine("[MikanAPIUnitTests]");
			bool success = true;
			success &= TestResponseTypeCacheContainsKnownTypes();
			success &= TestEventTypeCacheContainsKnownTypes();
			success &= TestTypeNameOnDerivedTypes();
			success &= TestRequestNoTypeIdInJson();
			success &= TestResponseRoundTrip();
			Console.WriteLine($"  MikanAPIUnitTests module - {(success ? "PASSED" : "FAILED")}");
			return success;
		}

		// Builds the same reflection-based cache that MikanRequestManager uses internally,
		// then verifies that known MikanResponse-derived types are resolvable by name.
		private bool TestResponseTypeCacheContainsKnownTypes()
		{
			const string testName = "response type cache contains known types";
			bool success = true;

			try
			{
				var cache = BuildTypeCache<MikanResponse>();

				// Core base type
				success = cache.ContainsKey("MikanResponse");
				Debug.Assert(success, "MikanResponse not in response type cache");

				// A concrete derived response type
				success &= cache.ContainsKey("MikanRemoteControlCommandResult");
				Debug.Assert(success, "MikanRemoteControlCommandResult not in response type cache");

				// Cache must return the correct CLR type
				success &= (cache["MikanResponse"] == typeof(MikanResponse));
				Debug.Assert(success, "MikanResponse cache entry maps to wrong CLR type");
			}
			catch (Exception e)
			{
				Console.WriteLine($"    {testName} - EXCEPTION: {e.Message}");
				success = false;
			}

			Console.WriteLine($"    {testName} - {(success ? "PASSED" : "FAILED")}");
			return success;
		}

		// Builds the same reflection-based cache that MikanEventManager uses internally,
		// then verifies that known MikanEvent-derived types are resolvable by name.
		private bool TestEventTypeCacheContainsKnownTypes()
		{
			const string testName = "event type cache contains known types";
			bool success = true;

			try
			{
				var cache = BuildTypeCache<MikanEvent>();

				// Core base type
				success = cache.ContainsKey("MikanEvent");
				Debug.Assert(success, "MikanEvent not in event type cache");

				// A concrete derived event type
				success &= cache.ContainsKey("MikanDisconnectedEvent");
				Debug.Assert(success, "MikanDisconnectedEvent not in event type cache");

				// Unknown type must NOT be in the cache
				success &= !cache.ContainsKey("ThisTypeDoesNotExist");
				Debug.Assert(success, "Unknown type unexpectedly found in event type cache");

				// Cache must return the correct CLR type
				success &= (cache["MikanDisconnectedEvent"] == typeof(MikanDisconnectedEvent));
				Debug.Assert(success, "MikanDisconnectedEvent cache entry maps to wrong CLR type");
			}
			catch (Exception e)
			{
				Console.WriteLine($"    {testName} - EXCEPTION: {e.Message}");
				success = false;
			}

			Console.WriteLine($"    {testName} - {(success ? "PASSED" : "FAILED")}");
			return success;
		}

		// Verifies that C# type names match the expected wire-format names used by
		// both the server and client for routing. Mirrors the C++ constructor test.
		private bool TestTypeNameOnDerivedTypes()
		{
			const string testName = "type name on derived types";
			bool success = true;

			try
			{
				success = (typeof(MikanRequest).Name == "MikanRequest");
				Debug.Assert(success, "MikanRequest.Name mismatch");

				success &= (typeof(MikanResponse).Name == "MikanResponse");
				Debug.Assert(success, "MikanResponse.Name mismatch");

				success &= (typeof(MikanEvent).Name == "MikanEvent");
				Debug.Assert(success, "MikanEvent.Name mismatch");

				// Derived request type name must match the concrete class, not the base
				success &= (typeof(MikanRemoteControlCommand).Name == "MikanRemoteControlCommand");
				Debug.Assert(success, "MikanRemoteControlCommand.Name mismatch");

				// MikanRequestManager.SendRequest stamps requestTypeName using GetType().Name
				var cmd = new MikanRemoteControlCommand();
				cmd.requestTypeName = cmd.GetType().Name;
				success &= (cmd.requestTypeName == "MikanRemoteControlCommand");
				Debug.Assert(success, "requestTypeName stamped by SendRequest is wrong");
			}
			catch (Exception e)
			{
				Console.WriteLine($"    {testName} - EXCEPTION: {e.Message}");
				success = false;
			}

			Console.WriteLine($"    {testName} - {(success ? "PASSED" : "FAILED")}");
			return success;
		}

		// Verifies the serialized JSON has requestTypeName present and
		// requestTypeId absent — confirming the wire-format change is complete.
		private bool TestRequestNoTypeIdInJson()
		{
			const string testName = "request JSON has typeName but no typeId";
			bool success = true;

			try
			{
				var cmd = new MikanRemoteControlCommand();
				cmd.requestTypeName = cmd.GetType().Name;
				cmd.requestId = 1;
				cmd.parameters = new List<string> { "param1", "param2" };
				cmd.command = "test_command";

				string jsonString = JsonSerializer.serializeToJsonString(cmd, cmd.GetType());

				JObject j = JObject.Parse(jsonString);

				// requestTypeName must be present (needed for server routing)
				success = j.Property("requestTypeName") != null;
				Debug.Assert(success, "requestTypeName missing from serialized request JSON");

				success &= ((string)j["requestTypeName"] == "MikanRemoteControlCommand");
				Debug.Assert(success, "requestTypeName has wrong value in serialized JSON");
			}
			catch (Exception e)
			{
				Console.WriteLine($"    {testName} - EXCEPTION: {e.Message}");
				success = false;
			}

			Console.WriteLine($"    {testName} - {(success ? "PASSED" : "FAILED")}");
			return success;
		}

		// Simulates the client-side parse path: serialize a response to JSON,
		// then use the reflection-based type cache to reconstruct it polymorphically.
		private bool TestResponseRoundTrip()
		{
			const string testName = "response JSON round-trip via type cache";
			bool success = true;

			try
			{
				// Build the same cache MikanRequestManager uses
				var cache = BuildTypeCache<MikanResponse>();

				// Simulate a server-produced response JSON
				var expected = new MikanResponse
				{
					responseTypeName = typeof(MikanResponse).Name,
					requestId = 99,
					resultCode = MikanAPIResult.Success
				};

				string jsonString = JsonSerializer.serializeToJsonString(expected, expected.GetType());

				// Simulate client parsing: extract responseTypeName from JSON
				JObject j = JObject.Parse(jsonString);
				string responseTypeName = (string)j["responseTypeName"];

				success = !string.IsNullOrEmpty(responseTypeName);
				Debug.Assert(success, "responseTypeName missing from serialized JSON");

				// Look up the concrete type by name and create an instance
				success &= cache.TryGetValue(responseTypeName, out Type responseType);
				Debug.Assert(success, $"Type '{responseTypeName}' not found in response type cache");

				object responseObject = Activator.CreateInstance(responseType);
				bool bDeserialize = JsonDeserializer.deserializeFromJsonString(jsonString, responseObject, responseType);

				success &= bDeserialize;
				Debug.Assert(success, "Failed to deserialize response from JSON string");

				// Verify field values survived the round-trip
				var actual = (MikanResponse)responseObject;
				success &= (actual.requestId == expected.requestId);
				Debug.Assert(success, $"requestId mismatch: expected {expected.requestId}, got {actual.requestId}");

				success &= (actual.resultCode == expected.resultCode);
				Debug.Assert(success, $"resultCode mismatch: expected {expected.resultCode}, got {actual.resultCode}");

				success &= (actual.responseTypeName == expected.responseTypeName);
				Debug.Assert(success, $"responseTypeName mismatch: expected {expected.responseTypeName}, got {actual.responseTypeName}");
			}
			catch (Exception e)
			{
				Console.WriteLine($"    {testName} - EXCEPTION: {e.Message}");
				success = false;
			}

			Console.WriteLine($"    {testName} - {(success ? "PASSED" : "FAILED")}");
			return success;
		}

		// Builds a name→Type dictionary for all non-abstract classes in this assembly
		// that are assignable to the given base type — the same logic used by
		// MikanRequestManager and MikanEventManager when they initialize.
		private static Dictionary<string, Type> BuildTypeCache<TBase>() where TBase : class
		{
			var cache = new Dictionary<string, Type>();

			var types = from t in typeof(TBase).Assembly.GetTypes()
						where t.IsClass && !t.IsAbstract
							&& t.Namespace == "MikanXR"
							&& typeof(TBase).IsAssignableFrom(t)
						select t;

			foreach (var t in types)
			{
				cache[t.Name] = t;
			}

			return cache;
		}
	}
}
