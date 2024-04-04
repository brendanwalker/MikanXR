using System.Threading.Tasks;
using System.Text.Json;
using System.Collections.Generic;
using System;

namespace MikanXR
{
	public class MikanResponse
	{
		public string responseType { get; set; }
		public int requestId { get; set; }
		public MikanResult resultCode { get; set; }
		
		public MikanResponse()
		{
			responseType= typeof(MikanResponse).Name;
			requestId= MikanXR.Constants.INVALID_MIKAN_ID;
			resultCode= MikanXR.MikanResult.Success;
		}
		public MikanResponse(string inResponseType)
		{
			responseType= inResponseType;
			requestId= MikanXR.Constants.INVALID_MIKAN_ID;
			resultCode= MikanXR.MikanResult.Success;
		}		
	};

	public interface IMikanResponseFactory
	{
		MikanResponse CreateResponse(string utfJsonString);
	}

	public class MikanResponseFactory<T> : IMikanResponseFactory where T : MikanResponse
	{
		public MikanResponse CreateResponse(string utfJsonString)
		{
			T response= JsonSerializer.Deserialize<T>(utfJsonString);

			return response;
		}
	}
	
	public class MikanRequestManager
	{
		private MikanCoreNative.NativeLogCallback _nativeLogCallback;
		private MikanCoreNative.NativeResponseCallback _nativeResponseCallback;
		private Dictionary<string, IMikanResponseFactory> _responseFactory;

		private class PendingRequest
		{
			public int requestId;
			public TaskCompletionSource<MikanResponse> promise;
		};
		private Dictionary<int, PendingRequest> _pendingRequests;
		
		public MikanRequestManager(MikanCoreNative.NativeLogCallback logCallback)
		{
			_nativeLogCallback= logCallback;
			_nativeResponseCallback = new MikanCoreNative.NativeResponseCallback(InternalResponseCallback);
			_responseFactory = new Dictionary<string, IMikanResponseFactory>();
			_pendingRequests = new Dictionary<int, PendingRequest>();
		}

		public MikanResult Initialize()
		{
			MikanResult result = 
				(MikanResult)MikanCoreNative.Mikan_SetResponseCallback(
					_nativeResponseCallback, IntPtr.Zero);
			if (result != MikanResult.Success)
			{
				return result;
			}

			return MikanResult.Success;
		}

		public void AddResponseFactory<T>() where T : MikanResponse
		{
			var factory = new MikanResponseFactory<T>();

			_responseFactory.Add(typeof(T).Name, factory);
		}

		public Task<MikanResponse> AddResponseHandler(int requestId, MikanResult result)
		{
			TaskCompletionSource<MikanResponse> promise = new TaskCompletionSource<MikanResponse>();

            if (result == MikanResult.Success)
            {
                PendingRequest pendingRequest = new PendingRequest() { 
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
			string payloadString = JsonSerializer.Serialize(payload);
			
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

		private void InternalResponseCallback(int requestId, string utf8ResponseString, IntPtr userData)
		{
			PendingRequest pendingRequest= null;

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

			JsonDocument document = JsonDocument.Parse(utf8ResponseString);
			JsonElement root = document.RootElement;

			// Check if the key "responseType" exists
			if (root.TryGetProperty("responseType", out JsonElement responseTypeElement))
			{
				// Check if the value of "responseType" is a string
				if (responseTypeElement.ValueKind == JsonValueKind.String)
				{
					// Get the string value of "responseType"
					string responseType = responseTypeElement.GetString();
					
					if (_responseFactory.TryGetValue(responseType, out IMikanResponseFactory factory))
					{
						response = factory.CreateResponse(utf8ResponseString);
					}
					else
					{
						_nativeLogCallback((int)MikanLogLevel.Error, "Unknown response type: " + responseType);
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

			// Dispose of the JsonDocument to free resources
			document.Dispose();

			return response;
		}
	}	
}