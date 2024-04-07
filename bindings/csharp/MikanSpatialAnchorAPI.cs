using System.Threading.Tasks;

namespace MikanXR
{	
	public class MikanSpatialAnchorAPI
	{
		private MikanRequestManager _requestManager;

		public MikanSpatialAnchorAPI(MikanRequestManager requestManager)
		{
			_requestManager= requestManager;
			_requestManager.AddResponseFactory<MikanSpatialAnchorList>();
			_requestManager.AddResponseFactory<MikanSpatialAnchorInfo>();
		}

		private static readonly string k_getSpatialAnchorList = "getSpatialAnchorList";
		public Task<MikanResponse> GetSpatialAnchorList() // returns MikanSpatialAnchorList
		{
			return _requestManager.SendRequest(k_getSpatialAnchorList);
		}
		
		private static readonly string k_getSpatialAnchorInfo = "getSpatialAnchorInfo";
		public Task<MikanResponse> getSpatialAnchorInfo(int anchorId) // returns MikanSpatialAnchorInfo
		{
			return _requestManager.SendRequestWithPayload<int>(k_getSpatialAnchorInfo, anchorId);
		}
		
		private static readonly string k_findSpatialAnchorInfoByName = "invokeScriptMessageHandler";
		public Task<MikanResponse> findSpatialAnchorInfoByName(string anchorName) // returns MikanSpatialAnchorInfo
		{
			return _requestManager.SendRequestWithPayload<string>(k_findSpatialAnchorInfoByName, anchorName);
		}
	}
}