using System.Collections;
using System.Runtime.InteropServices;
using System.Buffers;
using System.Text.Json;
using System.Threading;
using System.Threading.Tasks;

namespace MikanXR
{
	internal class IMikanEventFactory
	{
		virtual MikanEvent CreateEvent(string utfJsonString) = 0;
	}

	internal class MikanEventFactory<T> where T : MikanEvent, IMikanEventFactory
	{
		public override MikanEvent CreateEvent(string utfJsonString)
		{
			T response= JsonSerializer.Deserialize<T>(utfJsonString);

			return response;
		}
	}
	
	internal class MikanEventManager
	{
		private Dictionary<string, IMikanEventFactory> _eventFactories;
	
		public MikanEventManager(MikanCoreNative.NativeLogCallback logCallback)
		{
			_nativeLogCallback= logCallback;
			_eventFactories = new Dictionary<string, IMikanEventFactory>();
		}

		public void AddEventFactory<T>()
		{
			IMikanResponseFactory factory = new MikanEventFactory<T>();

			_eventFactories.Add(typeof(T).Name, factory);
		}

		private MikanResult FetchNextEvent(out MikanEvent outEvent)
		{
			StringBuilder utf8Buffer = new StringBuilder(1024);
			UIntPtr utf8BufferSize = (UIntPtr)utf8Buffer.Capacity;

			outEvent= null;

			var result = (MikanResult)MikanCoreNative.Mikan_FetchNextEvent(utf8BufferSize, utf8Buffer, out UIntPtr utf8BytesWritten);
			if (result == MikanResult.Success)
			{
				string utf8BufferString = utf8Buffer.ToString();
				
				outEvent = parseEvent(utf8BufferString);
				if (outEvent != null)
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