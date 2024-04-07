using System.Threading.Tasks;

namespace MikanXR
{	
	public class MikanVideoSourceAPI
	{
		private MikanRequestManager _requestManager;

		public MikanVideoSourceAPI(MikanRequestManager requestManager)
		{
			_requestManager= requestManager;
			_requestManager.AddResponseFactory<MikanVideoSourceIntrinsics>();
			_requestManager.AddResponseFactory<MikanVideoSourceMode>();
			_requestManager.AddResponseFactory<MikanVideoSourceAttachmentInfo>();
		}

		private static readonly string k_getVideoSourceIntrinsics = "getVideoSourceIntrinsics";
		public Task<MikanResponse> GetVideoSourceIntrinsics() // returns MikanVideoSourceIntrinsics
		{
			return _requestManager.SendRequest(k_getVideoSourceIntrinsics);
		}
		
		private static readonly string k_getVideoSourceMode = "getVideoSourceMode";
		public Task<MikanResponse> GetVideoSourceMode() // returns MikanVideoSourceMode
		{
			return _requestManager.SendRequest(k_getVideoSourceMode);
		}
		
		private static readonly string k_getVideoSourceAttachment = "getVideoSourceAttachment";
		public Task<MikanResponse> GetVideoSourceAttachment() // returns MikanVideoSourceAttachmentInfo
		{
			return _requestManager.SendRequest(k_getVideoSourceAttachment);
		}
	}
}