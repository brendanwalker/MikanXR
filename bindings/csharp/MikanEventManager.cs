using System;
using System.Collections.Generic;
using System.Text;
using System.Text.Json;

namespace MikanXR
{
	public interface IMikanEventFactory
	{
		MikanEvent CreateEvent(string utfJsonString);
	}

	public class MikanEventFactory<T> : IMikanEventFactory where T : MikanEvent
	{
		public MikanEvent CreateEvent(string utfJsonString)
		{
			// Deserialize enumerations from strings rather than from integers
			var stringEnumConverter = new System.Text.Json.Serialization.JsonStringEnumConverter();
			JsonSerializerOptions opts = new JsonSerializerOptions();
			opts.Converters.Add(stringEnumConverter);

			T response= JsonSerializer.Deserialize<T>(utfJsonString, opts);

			return response;
		}
	}
	
	public class MikanEventManager
	{
		private MikanCoreNative.NativeLogCallback _nativeLogCallback;
		private Dictionary<string, IMikanEventFactory> _eventFactories;
	
		public MikanEventManager(MikanCoreNative.NativeLogCallback logCallback)
		{
			_nativeLogCallback= logCallback;
			_eventFactories = new Dictionary<string, IMikanEventFactory>();
		}

		public void AddEventFactory<T>() where T : MikanEvent
		{
			var factory = new MikanEventFactory<T>();

			_eventFactories.Add(typeof(T).Name, factory);
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
			MikanEvent mikanEvent = null;

			JsonDocument document = JsonDocument.Parse(utf8ResponseString);
			JsonElement root = document.RootElement;

			// Check if the key "eventType" exists
			if (root.TryGetProperty("eventType", out JsonElement eventTypeElement))
			{
				// Check if the value of "eventType" is a string
				if (eventTypeElement.ValueKind == JsonValueKind.String)
				{
					// Get the string value of "eventType"
					string eventType = eventTypeElement.GetString();
					
					if (_eventFactories.TryGetValue(eventType, out IMikanEventFactory factory))
					{
						mikanEvent = factory.CreateEvent(utf8ResponseString);
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

			// Dispose of the JsonDocument to free resources
			document.Dispose();

			return mikanEvent;
		}
	}	
}