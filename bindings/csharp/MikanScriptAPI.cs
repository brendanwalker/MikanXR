using System.Collections;
using System.Runtime.InteropServices;
using System.Buffers;
using System.Text.Json;
using System.Threading;
using System.Threading.Tasks;

namespace MikanXR
{	
	public class MikanScriptAPI
	{
		private MikanRequestManager _requestManager;
		
		public MikanScriptAPI(MikanRequestManager requestManager) : _requestManager(requestManager) {}

		private static readonly string k_sendScriptMessage = "invokeScriptMessageHandler";
		public Task<MikanResponse> sendScriptMessage(string mesg) // returns MikanResult
		{
			return _requestManager.sendRequestWithPayload<string>(k_sendScriptMessage, mesg);
		}
	}
}