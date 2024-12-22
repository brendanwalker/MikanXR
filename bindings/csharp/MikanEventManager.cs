using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using Newtonsoft.Json.Linq;

namespace MikanXR
{
	public class MikanEventManager
	{
		private static readonly string WEBSOCKET_CONNECT_EVENT = "connect";
		private static readonly string WEBSOCKET_DISCONNECT_EVENT = "disconnect";
		private static readonly string WEBSOCKET_ERROR_EVENT = "error";
		private static readonly string WEBSOCKET_PING_EVENT = "ping";
		private static readonly string WEBSOCKET_PONG_EVENT = "pong";

		private MikanCoreNative.NativeLogCallback _nativeLogCallback;
		private IntPtr _mikanContext = IntPtr.Zero;
		private Dictionary<long, Type> _eventTypeCache = null;

		public MikanEventManager(MikanCoreNative.NativeLogCallback logCallback)
		{
			_nativeLogCallback= logCallback;
			_eventTypeCache = new Dictionary<long, Type>();
		}

		public void Initialize(IntPtr mikanContext)
		{
			_mikanContext = mikanContext;

			// Build a map from ClassId to MikanEvent Type
			var eventTypes = from t in Assembly.GetExecutingAssembly().GetTypes()
					where t.IsClass && t.Namespace == "MikanXR" && typeof(MikanEvent).IsAssignableFrom(t)
					select t;
			eventTypes.ToList().ForEach(t =>
			{
				long classId = Utils.getMikanClassId(t);

				_eventTypeCache[classId] = t;
			});
		}

		public MikanAPIResult FetchNextEvent(out MikanEvent outEvent)
		{
			StringBuilder utf8Buffer = new StringBuilder(1024);
			UIntPtr utf8BufferSize = (UIntPtr)utf8Buffer.Capacity;

			outEvent= null;

			var result = (MikanAPIResult)MikanCoreNative.Mikan_FetchNextEvent(
				_mikanContext, utf8BufferSize, utf8Buffer, out UIntPtr utf8BytesWritten);
			if (result == MikanAPIResult.Success)
			{
				string utf8BufferString = utf8Buffer.ToString();
				
				outEvent = parseEventString(utf8BufferString);
				if (outEvent == null)
				{
					_nativeLogCallback((int)MikanLogLevel.Error, 
						"fetchNextEvent() - failed to parse event string: " + utf8BufferString);
					result = MikanAPIResult.MalformedResponse;
				}
			}
			
			return (MikanAPIResult)result;
		}

		private MikanEvent parseEventString(string utf8ResponseString)
		{
			MikanEvent mikanEvent= null;

			if (utf8ResponseString.StartsWith(WEBSOCKET_CONNECT_EVENT))
			{
				int clientVersion = MikanCoreNative.Mikan_GetClientAPIVersion();
				int serverVersion= 0;
				int minClientVersion= 0;

				string[] tokens = utf8ResponseString.Split(new char[] {':'});
				if (tokens.Length >= 3)
				{
					int.TryParse(tokens[1], out serverVersion);
					int.TryParse(tokens[2], out minClientVersion);
				}

				// Make sure the client version isn't too old
				if (clientVersion >= minClientVersion)
				{
					var connectEventPtr = new MikanConnectedEvent();
					connectEventPtr.serverVersion.version = serverVersion;
					connectEventPtr.minClientVersion.version = minClientVersion;

					mikanEvent = connectEventPtr;
				}
				else
				{
					// Disconnect since we have incompatible client
					// This will trigger an WEBSOCKET_DISCONNECT_EVENT
					MikanCoreNative.Mikan_Disconnect(
						_mikanContext,
						(ushort)MikanDisconnectCode.IncompatibleVersion,
						"Incompatible client version");
				}
			}
			else if (utf8ResponseString.StartsWith(WEBSOCKET_DISCONNECT_EVENT))
			{
				int disconnectCode = 0;
				string disconnectReason = "";

				string[] tokens = utf8ResponseString.Split(new char[] {':'});
				if (tokens.Length >= 3)
				{
					int.TryParse(tokens[1], out disconnectCode);
					disconnectReason = tokens[2];
				}

				var disconnectEventPtr = new MikanDisconnectedEvent();
				disconnectEventPtr.code = (MikanDisconnectCode)disconnectCode;
				disconnectEventPtr.reason= disconnectReason;
			}
			else if (utf8ResponseString.StartsWith(WEBSOCKET_ERROR_EVENT))
			{
				_nativeLogCallback(
					(int)MikanLogLevel.Error,
					"Received websocket ERROR: " + utf8ResponseString);
			}
			else if (utf8ResponseString.StartsWith(WEBSOCKET_PING_EVENT))
			{
				_nativeLogCallback((int)MikanLogLevel.Info, "Received websocket PING");
			}
			else if (utf8ResponseString.StartsWith(WEBSOCKET_PONG_EVENT))
			{
				_nativeLogCallback((int)MikanLogLevel.Info, "Received websocket PONG");
			}
			else
			{
				var root = JObject.Parse(utf8ResponseString);

				// Check if the "eventTypeName" and "eventTypeId" keys exist
				if (root.TryGetValue("eventTypeName", out JToken eventTypeNameElement) &&
					root.TryGetValue("eventTypeId", out JToken eventTypeIdElement))
				{
					// Check if the value of eventType keys
					if (eventTypeNameElement.Type == JTokenType.String &&
						eventTypeIdElement.Type == JTokenType.Integer)
					{
						// Get the string value of "eventTypeName"
						string eventTypeName = (string)eventTypeNameElement;
						// Get the integer value of "eventTypeId"
						long eventTypeId = (long)eventTypeIdElement;

						// Attempt to create the event object by class name
						if (_eventTypeCache.TryGetValue(eventTypeId, out Type eventType))
						{
							object eventObject = Activator.CreateInstance(eventType);

							// Deserialize the event object from the JSON string
							if (JsonDeserializer.deserializeFromJsonString(utf8ResponseString, eventObject, eventType))
							{
								mikanEvent = (MikanEvent)eventObject;
							}
							else
							{
								_nativeLogCallback(
									(int)MikanLogLevel.Error,
									"Failed to deserialize event object from JSON string: " + utf8ResponseString);
							}
						}
						else
						{
							_nativeLogCallback((int)MikanLogLevel.Error,
								"Unknown event type: " + eventTypeName +
								" (classId: " + eventTypeId + ")");
						}
					}
					else
					{
						_nativeLogCallback((int)MikanLogLevel.Error, "eventTypes not of expected types.");
					}
				}
				else
				{
					_nativeLogCallback((int)MikanLogLevel.Error, "eventType keys not found.");
				}
			}

			return mikanEvent;
		}
	}	
}