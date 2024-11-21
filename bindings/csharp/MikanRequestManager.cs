using System.Threading.Tasks;
using System.Collections.Generic;
using System;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System.IO;
using System.Runtime.InteropServices;

namespace MikanXR
{
	public class MikanRequestManager
	{
		private MikanCoreNative.NativeLogCallback _nativeLogCallback;
		private MikanCoreNative.NativeTextResponseCallback _nativeTextResponseCallback;
		private MikanCoreNative.NativeBinaryResponseCallback _nativeBinaryResponseCallback;

		private class PendingRequest
		{
			public int requestId;
			public TaskCompletionSource<MikanResponse> promise;
		};
		private Dictionary<int, PendingRequest> _pendingRequests;

		public MikanRequestManager(MikanCoreNative.NativeLogCallback logCallback)
		{
			_nativeLogCallback = logCallback;
			_nativeTextResponseCallback = new MikanCoreNative.NativeTextResponseCallback(InternalTextResponseCallback);
			_nativeBinaryResponseCallback = new MikanCoreNative.NativeBinaryResponseCallback(InternalBinaryResponseCallback);
			_pendingRequests = new Dictionary<int, PendingRequest>();
		}

		public MikanResult Initialize()
		{
			MikanResult result =
				(MikanResult)MikanCoreNative.Mikan_SetTextResponseCallback(
					_nativeTextResponseCallback, IntPtr.Zero);
			if (result != MikanResult.Success)
			{
				return result;
			}

			return MikanResult.Success;
		}

		public Task<MikanResponse> AddResponseHandler(int requestId, MikanResult result)
		{
			TaskCompletionSource<MikanResponse> promise = new TaskCompletionSource<MikanResponse>();

			if (result == MikanResult.Success)
			{
				PendingRequest pendingRequest = new PendingRequest()
				{
					requestId = requestId,
					promise = promise
				};

				lock (_pendingRequests)
				{
					_pendingRequests.Add(requestId, pendingRequest);
				}
			}
			else
			{
				MikanResponse response = new MikanResponse()
				{
					requestId = requestId,
					resultCode = result
				};

				promise.SetResult(response);
			}

			return promise.Task;
		}

		public Task<MikanResponse> SendRequest(string utf8RequestType, int version = 0)
		{
			return SendRequestIntenal(utf8RequestType, string.Empty, version);
		}

		public Task<MikanResponse> SendRequestWithPayload<T>(string utf8RequestType, T payload, int version = 0)
		{
			// Serialize enumerations from strings rather than from integers
			var stringEnumConverter = new Newtonsoft.Json.Converters.StringEnumConverter();
			string payloadString = JsonConvert.SerializeObject(payload, stringEnumConverter);

			return SendRequestIntenal(utf8RequestType, payloadString, version);
		}

		private Task<MikanResponse> SendRequestIntenal(
			string utf8RequestType,
			string utf8Payload,
			int requestVersion)
		{
			MikanResult result =
				(MikanResult)MikanCoreNative.Mikan_SendRequest(
					utf8RequestType,
					utf8Payload,
					requestVersion,
					out int requestId);

			return AddResponseHandler(requestId, result);
		}

		private void InternalTextResponseCallback(int requestId, string utf8ResponseString, IntPtr userData)
		{
			PendingRequest pendingRequest = null;

			lock (_pendingRequests)
			{
				if (_pendingRequests.TryGetValue(requestId, out pendingRequest))
				{
					_pendingRequests.Remove(requestId);
				}
			}

			if (pendingRequest != null)
			{
				MikanResponse response = ParseResponseString(utf8ResponseString);

				if (response == null)
				{
					response = new MikanResponse()
					{
						requestId = requestId,
						resultCode = MikanResult.MalformedResponse
					};
				}

				pendingRequest.promise.SetResult(response);
			}
		}

		private MikanResponse ParseResponseString(string utf8ResponseString)
		{
			MikanResponse response = null;

			var root = JObject.Parse(utf8ResponseString);

			// Check if the key "responseType" exists
			if (root.TryGetValue("responseType", out JToken responseTypeElement))
			{
				// Check if the value of "responseType" is a string
				if (responseTypeElement.Type == JTokenType.String)
				{
					// Get the string value of "responseType"
					string responseTypeName = (string)responseTypeElement;

					// Attempt to create the response object by class name
					object responseObject = Utils.allocateMikanTypeByName(responseTypeName, out Type responseType);
					if (responseObject != null)
					{
						// Deserialize the response object from the JSON string
						if (JsonDeserializer.deserializeFromJsonString(utf8ResponseString, responseObject, responseType))
						{
							response = (MikanResponse)responseObject;
						}
						else
						{
							_nativeLogCallback(
								(int)MikanLogLevel.Error,
								"Failed to deserialize response object from JSON string: " + utf8ResponseString);
						}
					}
					else
					{
						_nativeLogCallback((int)MikanLogLevel.Error, "Unknown response type: " + responseTypeName);
					}
				}
				else
				{
					_nativeLogCallback((int)MikanLogLevel.Error, "responseType is not a string.");
				}
			}
			else
			{
				_nativeLogCallback((int)MikanLogLevel.Error, "responseType key not found.");
			}

			return response;
		}

		private void InternalBinaryResponseCallback(IntPtr buffer, UIntPtr bufferSize, IntPtr userData)
		{
			int bufferSizeInt = (int)bufferSize.ToUInt32();
			byte[] managedBuffer = new byte[bufferSizeInt];
			Marshal.Copy(buffer, managedBuffer, 0, bufferSizeInt);

			var binaryReader = new BinaryReader(new MemoryStream(managedBuffer));

			try
			{
				// Read the response type
				int requestTypeUTF8StringLength = binaryReader.ReadInt32();
				string responseTypeName =
					requestTypeUTF8StringLength > 0
					? System.Text.Encoding.UTF8.GetString(binaryReader.ReadBytes(requestTypeUTF8StringLength))
					: "";

				// Read the request ID
				int requestId = binaryReader.ReadInt32();

				// Read the result code
				MikanResult resultCode = (MikanResult)binaryReader.ReadInt32();

				// Look up the pending request
				PendingRequest pendingRequest = null;
				lock (_pendingRequests)
				{
					if (_pendingRequests.TryGetValue(requestId, out pendingRequest))
					{
						_pendingRequests.Remove(requestId);
					}
				}

				// Bail if the corresponding pending request is not found
				if (pendingRequest != null)
				{
					// Attempt to create the response object by class name
					MikanResponse response = null;
					object responseObject = Utils.allocateMikanTypeByName(responseTypeName, out Type responseType);
					if (responseObject != null)
					{
						// Deserialize the event object from the byte array
						if (BinaryDeserializer.DeserializeFromBytes(managedBuffer, responseObject, responseType))
						{
							response = (MikanResponse)responseObject;
						}
						else
						{
							_nativeLogCallback((int)MikanLogLevel.Error, "Failed to deserialize response object from byte array");
						}
					}
					else
					{
						_nativeLogCallback((int)MikanLogLevel.Error, "Unknown response type: " + responseTypeName);
					}

					if (response == null)
					{
						response = new MikanResponse()
						{
							requestId = requestId,
							resultCode = MikanResult.MalformedResponse
						};
					}

					pendingRequest.promise.SetResult(response);
				}
				else
				{
					_nativeLogCallback((int)MikanLogLevel.Error, $"Invalid pending request id({requestId}) for response type {responseTypeName}");
				}
			}
			catch (Exception e)
			{
				_nativeLogCallback((int)MikanLogLevel.Error, $"Malformed binary response: {e.Message}");
			}
			finally
			{
				if (binaryReader != null)
				{
					binaryReader.Dispose();
				}
			}
		}
	}
}