using System.Collections;
using System.Runtime.InteropServices;
using System.Buffers;
using System.Text.Json;
using System.Threading;
using System.Threading.Tasks;

namespace MikanXR
{	
	public class MikanVideoSourceAPI
	{
		private MikanRequestManager _requestManager;

		public MikanVideoSourceAPI(MikanRequestManager requestManager) : _requestManager(requestManager) 
		{
			_requestManager.addResponseFactory<MikanVideoSourceIntrinsics>();
			_requestManager.addResponseFactory<MikanVideoSourceMode>();
			_requestManager.addResponseFactory<MikanVideoSourceAttachmentInfo>();			
		}

		public Task<MikanResponse> getVideoSourceIntrinsics() // returns MikanVideoSourceIntrinsics
		{
			return m_requestManager.sendRequest(nameof(getVideoSourceIntrinsics));
		}
		
		public Task<MikanResponse> getVideoSourceMode() // returns MikanVideoSourceMode
		{
			return m_requestManager.sendRequest(nameof(getVideoSourceMode));
		}
		
		public Task<MikanResponse> getVideoSourceAttachment() // returns MikanVideoSourceAttachmentInfo
		{
			return m_requestManager.sendRequest(nameof(getVideoSourceAttachment));
		}
	}
}