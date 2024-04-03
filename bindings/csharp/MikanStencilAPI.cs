using System.Collections;
using System.Runtime.InteropServices;
using System.Buffers;
using System.Text.Json;
using System.Threading;
using System.Threading.Tasks;

namespace MikanXR
{	
	public class MikanStencilAPI
	{
		private MikanRequestManager _requestManager;

		public MikanStencilAPI(MikanRequestManager requestManager) : _requestManager(requestManager) 
		{
			_requestManager.addResponseFactory<MikanStencilList>();
			_requestManager.addResponseFactory<MikanStencilQuad>();
			_requestManager.addResponseFactory<MikanStencilBox>();
			_requestManager.addResponseFactory<MikanStencilModel>();
		}

		public Task<MikanResponse> getStencilList() // returns MikanStencilList
		{
			return m_requestManager.sendRequest(nameof(getStencilList));
		}
		
		public Task<MikanResponse> getQuadStencil(int stencilId) // returns MikanStencilQuad
		{
			return m_requestManager.sendRequestWithPayload<int>(nameof(getQuadStencil), stencilId);
		}
		
		public Task<MikanResponse> getBoxStencil(int stencilId) // returns MikanStencilBox
		{
			return m_requestManager.sendRequestWithPayload<int>(nameof(getBoxStencil), stencilId);
		}
		
		public Task<MikanResponse> getModelStencil(int stencilId) // returns MikanStencilModel
		{
			return m_requestManager.sendRequestWithPayload<int>(nameof(getModelStencil), stencilId);
		}		
	}
}