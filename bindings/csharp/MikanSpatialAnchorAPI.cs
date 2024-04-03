using System.Collections;
using System.Runtime.InteropServices;
using System.Buffers;
using System.Text.Json;
using System.Threading;
using System.Threading.Tasks;

namespace MikanXR
{	
	public class MikanSpatialAnchorAPI
	{
		private MikanRequestManager _requestManager;

		public MikanSpatialAnchorAPI(MikanRequestManager requestManager) : _requestManager(requestManager) 
		{
			_requestManager.addResponseFactory<MikanSpatialAnchorList>();
			_requestManager.addResponseFactory<MikanSpatialAnchorInfo>();
		}

		public Task<MikanResponse> getSpatialAnchorList() // returns MikanSpatialAnchorList
		{
			return _requestManager.sendRequest(nameof(getSpatialAnchorList));
		}
		
		public Task<MikanResponse> getSpatialAnchorInfo(int anchorId) // returns MikanSpatialAnchorInfo
		{
			return m_requestManager->sendRequestWithPayload<int>(nameof(getSpatialAnchorInfo), anchorId);
		}
		
		public Task<MikanResponse> findSpatialAnchorInfoByName(string anchorName) // returns MikanSpatialAnchorInfo
		{
			return m_requestManager->sendRequestWithPayload<string>(nameof(findSpatialAnchorInfoByName), anchorName);
		}
	}
}