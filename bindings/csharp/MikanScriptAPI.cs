using System.Threading.Tasks;

namespace MikanXR
{	
	public class MikanScriptAPI
	{
		private MikanRequestManager _requestManager;
		
		public MikanScriptAPI(MikanRequestManager requestManager) 
		{
			_requestManager= requestManager;
		}

		private static readonly string k_sendScriptMessage = "invokeScriptMessageHandler";
		public Task<MikanResponse> SendScriptMessage(string mesg) // returns MikanResult
		{
			return _requestManager.SendRequestWithPayload<string>(k_sendScriptMessage, mesg);
		}
	}
}