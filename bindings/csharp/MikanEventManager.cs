using System;
using System.Text;
using Newtonsoft.Json.Linq;

namespace MikanXR
{
	public class MikanEventManager
	{
		private MikanCoreNative.NativeLogCallback _nativeLogCallback;
	
		public MikanEventManager(MikanCoreNative.NativeLogCallback logCallback)
		{
			_nativeLogCallback= logCallback;
		}

		public MikanResult FetchNextEvent(out MikanEvent outEvent)
		{
			StringBuilder utf8Buffer = new StringBuilder(1024);
			UIntPtr utf8BufferSize = (UIntPtr)utf8Buffer.Capacity;

			outEvent= null;

			var result = (MikanResult)MikanCoreNative.Mikan_FetchNextEvent(
				utf8BufferSize, utf8Buffer, out UIntPtr utf8BytesWritten);
			if (result == MikanResult.Success)
			{
				string utf8BufferString = utf8Buffer.ToString();
				
				outEvent = parseEventString(utf8BufferString);
				if (outEvent == null)
				{
					_nativeLogCallback((int)MikanLogLevel.Error, 
						"fetchNextEvent() - failed to parse event string: " + utf8BufferString);
					result = MikanResult.MalformedResponse;
				}
			}
			
			return (MikanResult)result;
		}

		private MikanEvent parseEventString(string utf8ResponseString)
		{
			MikanEvent mikanEvent= null;

			var root = JObject.Parse(utf8ResponseString);

			// Check if the key "eventType" exists
			if (root.TryGetValue("eventType", out JToken eventTypeElement))
			{
				// Check if the value of "eventType" is a string
				if (eventTypeElement.Type == JTokenType.String)
				{
					// Get the string value of "eventType"
					string eventTypeName = (string)eventTypeElement;

					// Attempt to create the event object by class name
					object eventObject= Utils.allocateMikanTypeByName(eventTypeName, out Type eventType);
					if (eventObject != null)
					{
						// Deserialize the event object from the JSON string
						if (JsonDeserializer.deserializeFromJsonString(utf8ResponseString, eventObject, eventType))
						{
							mikanEvent= (MikanEvent)eventObject;
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
						_nativeLogCallback((int)MikanLogLevel.Error, "Unknown event type: " + eventType);
					}
				}
				else
				{
					_nativeLogCallback((int)MikanLogLevel.Error, "eventType is not a string.");
				}
			}
			else
			{
				_nativeLogCallback((int)MikanLogLevel.Error, "eventType key not found.");
			}

			return mikanEvent;
		}
	}	
}