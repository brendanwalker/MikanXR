using System.Threading.Tasks;
using System.Collections.Generic;
using System;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System.IO;
using System.Runtime.InteropServices;
using System.Reflection;
using System.Linq;

namespace MikanXR
{
	public class MikanRequestManager
	{
		private MikanCoreNative.NativeLogCallback _nativeLogCallback;
		private MikanCoreNative.NativeTextResponseCallback _nativeTextResponseCallback;
		private MikanCoreNative.NativeBinaryResponseCallback _nativeBinaryResponseCallback;

		private IntPtr _mikanContext= IntPtr.Zero;
		private int m_nextRequestID= 0;
		private Dictionary<long, Type> _responseTypeCache = null;

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
			_responseTypeCache = new Dictionary<long, Type>();
		}

		public MikanAPIResult Initialize(IntPtr mikanContext)
		{
			MikanAPIResult result =
				(MikanAPIResult)MikanCoreNative.Mikan_SetTextResponseCallback(
					mikanContext, _nativeTextResponseCallback, IntPtr.Zero);
			if (result != MikanAPIResult.Success)
			{
				return result;
			}

			_mikanContext = mikanContext;

			// Build a map from ClassId to MikanResponse Type
			var eventTypes = from t in Assembly.GetExecutingAssembly().GetTypes()
							 where t.IsClass && t.Namespace == "MikanXR" && typeof(MikanResponse).IsAssignableFrom(t)
							 select t;
			eventTypes.ToList().ForEach(t =>
			{
				long classId = Utils.getMikanClassId(t);

				_responseTypeCache[classId] = t;
			});

			return MikanAPIResult.Success;
		}

		void InsertPendingRequest(PendingRequest pendingRequest)
		{
			lock (_pendingRequests)
			{
				_pendingRequests.Add(pendingRequest.requestId, pendingRequest);
			}
		}

		PendingRequest RemovePendingRequest(int requestId)
		{
			PendingRequest pendingRequest= null;

			lock(_pendingRequests)
			{
				if (_pendingRequests.TryGetValue(requestId, out pendingRequest))
				{
					_pendingRequests.Remove(requestId);
				}
			}

			return pendingRequest;
		}

		public MikanResponseFuture AddResponseHandler(int requestId, MikanAPIResult result)
		{
			TaskCompletionSource<MikanResponse> promise = new TaskCompletionSource<MikanResponse>();
			var future= new MikanResponseFuture(this, requestId, promise);

			if (result == MikanAPIResult.Success)
			{
				PendingRequest pendingRequest = new PendingRequest()
				{
					requestId = requestId,
					promise = promise
				};

				InsertPendingRequest(pendingRequest);
			}
			else
			{
				MikanResponse response = new MikanResponse()
				{
					responseTypeName = typeof(MikanResponse).Name,
					responseTypeId = MikanResponse.classId,
					requestId = requestId,
					resultCode = result
				};

				promise.SetResult(response);
			}

			return future;
		}

		public MikanAPIResult CancelRequest(int requestId)
		{
			PendingRequest existingRequest = RemovePendingRequest(requestId);

			return existingRequest != null ? MikanAPIResult.Success : MikanAPIResult.InvalidParam;
		}

		public MikanResponseFuture SendRequest(MikanRequest request)
		{
			// Stamp the request with the request type name and id
			Type requestType = request.GetType();
			request.requestTypeName = requestType.Name;
			request.requestTypeId = Utils.getMikanClassId(requestType);

			// Stamp the request with the next request id
			request.requestId= m_nextRequestID;
			m_nextRequestID++;

			// Serialize the request to a Json string
			string jsonRequestString= 
				JsonSerializer.serializeToJsonString(
					request, request.GetType());

			// Send the request string to Mikan
			MikanAPIResult result = 
				(MikanAPIResult)MikanCoreNative.Mikan_SendRequestJSON(
					_mikanContext, jsonRequestString);

			// Create a request handler
			return AddResponseHandler(request.requestId, result);
		}

		private void InternalTextResponseCallback(int requestId, string utf8ResponseString, IntPtr userData)
		{
			PendingRequest pendingRequest = RemovePendingRequest(requestId);

			if (pendingRequest != null)
			{
				MikanResponse response = ParseResponseString(utf8ResponseString);

				if (response == null)
				{
					response = new MikanResponse()
					{
						requestId = requestId,
						resultCode = MikanAPIResult.MalformedResponse
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
			if (root.TryGetValue("responseTypeName", out JToken responseTypeNameElement) &&
				root.TryGetValue("responseTypeId", out JToken responseTypeIdElement))
			{
				// Check if the value of "responseType" is a string
				if (responseTypeNameElement.Type == JTokenType.String && 
					responseTypeIdElement.Type == JTokenType.Integer)
				{
					// Get the string value of "responseType"
					string responseTypeName = (string)responseTypeNameElement;
					long responseTypeId = (long)responseTypeIdElement;

					// Attempt to create the response object by class name
					if (_responseTypeCache.TryGetValue(responseTypeId, out Type responseType))
					{
						object responseObject = Activator.CreateInstance(responseType);

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
						_nativeLogCallback((int)MikanLogLevel.Error,
							"Unknown response type: " + responseTypeName +
							" (classId: " + responseTypeId + ")");
					}
				}
				else
				{
					_nativeLogCallback((int)MikanLogLevel.Error, "responseTypes are not of expected types.");
				}
			}
			else
			{
				_nativeLogCallback((int)MikanLogLevel.Error, "responseType keys not found.");
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
				// Read the respose type id
				long responseTypeId = binaryReader.ReadInt64();

				// Read the response type name
				int requestTypeUTF8StringLength = binaryReader.ReadInt32();
				string responseTypeName =
					requestTypeUTF8StringLength > 0
					? System.Text.Encoding.UTF8.GetString(binaryReader.ReadBytes(requestTypeUTF8StringLength))
					: "";

				// Read the request ID
				int requestId = binaryReader.ReadInt32();

				// Read the result code
				MikanAPIResult resultCode = (MikanAPIResult)binaryReader.ReadInt32();

				// Look up the pending request
				PendingRequest pendingRequest = RemovePendingRequest(requestId);

				// Bail if the corresponding pending request is not found
				if (pendingRequest != null)
				{
					// Attempt to create the response object by class name
					MikanResponse response = null;
					if (_responseTypeCache.TryGetValue(responseTypeId, out Type responseType))
					{
						object responseObject = Activator.CreateInstance(responseType);

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
							responseTypeId = MikanResponse.classId,
							responseTypeName = typeof(MikanResponse).Name,
							requestId = requestId,
							resultCode = MikanAPIResult.MalformedResponse
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