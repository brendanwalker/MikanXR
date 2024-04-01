using System.Collections;
using System.Runtime.InteropServices;
using System.Buffers;
using System.Text.Json;
using System.Threading;
using System.Threading.Tasks;

namespace MikanXR
{
	struct MikanEvent
	{
		public string eventType { get; set; }
		
		public MikanEvent()
		{
			eventType= typeof(MikanEvent).Name;
		}

		public MikanEvent(string inEventType)
		{
			eventType= inEventType;
		}
	};
	
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
			_responseFactory = new Dictionary<string, IMikanEventFactory>();
			_pendingRequests = new Dictionary<int, PendingRequest>()
		}

		public void AddEventFactory<T>()
		{
			IMikanResponseFactory factory = new MikanEventFactory<T>();

			_responseFactory.Add(typeof(T).Name, factory);
		}

		private MikanResult FetchNextEvent(out MikanEvent)
		{
			StringBuilder utf8Buffer = new StringBuilder(1024);
			UIntPtr utf8BufferSize = (UIntPtr)utf8Buffer.Capacity;

			var result = (MikanResult)MikanCoreNative.Mikan_FetchNextEvent(utf8BufferSize, utf8Buffer, out UIntPtr utf8BytesWritten);
			if (result == MikanResult.Success)
			{
			}
			
			outUtf8Buffer = utf8Buffer.ToString();
			return (MikanResult)result;			
		}

		private MikanResponse parseEventString(string utf8ResponseString)
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