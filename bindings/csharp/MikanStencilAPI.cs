using System.Threading.Tasks;

namespace MikanXR
{	
	public class MikanStencilAPI
	{
		private MikanRequestManager _requestManager;

		public MikanStencilAPI(MikanRequestManager requestManager)
		{
			_requestManager= requestManager;
			_requestManager.AddTextResponseFactory<MikanStencilList>();
			_requestManager.AddTextResponseFactory<MikanStencilQuadInfo>();
			_requestManager.AddTextResponseFactory<MikanStencilBoxInfo>();
			_requestManager.AddTextResponseFactory<MikanStencilModelInfo>();
			_requestManager.AddBinaryResponseFactory<
				MikanStencilModeRenderGeometryFactory,
				MikanStencilModelRenderGeometry>();
		}

		private static readonly string k_getQuadStencilList = "getQuadStencilList";
		public Task<MikanResponse> getQuadStencilList() // returns MikanStencilList
		{
			return _requestManager.SendRequest(k_getQuadStencilList);
		}

		private static readonly string k_getQuadStencil = "getQuadStencil";
		public Task<MikanResponse> getQuadStencil(int stencilId) // returns MikanStencilQuad
		{
			return _requestManager.SendRequestWithPayload<int>(k_getQuadStencil, stencilId);
		}

		private static readonly string k_getBoxStencilList = "getBoxStencilList";
		public Task<MikanResponse> getBoxStencilList() // returns MikanStencilList
		{
			return _requestManager.SendRequest(k_getBoxStencilList);
		}

		private static readonly string k_getBoxStencil = "getBoxStencil";
		public Task<MikanResponse> getBoxStencil(int stencilId) // returns MikanStencilBox
		{
			return _requestManager.SendRequestWithPayload<int>(k_getBoxStencil, stencilId);
		}

		private static readonly string k_getModelStencilList = "getModelStencilList";
		public Task<MikanResponse> getModelStencilList() // returns MikanStencilList
		{
			return _requestManager.SendRequest(k_getModelStencilList);
		}

		private static readonly string k_getModelStencil = "getModelStencil";
		public Task<MikanResponse> getModelStencil(int stencilId) // returns MikanStencilModel
		{
			return _requestManager.SendRequestWithPayload<int>(k_getModelStencil, stencilId);
		}

		private static readonly string k_getModelStencilRenderGeometry = "getModelStencilRenderGeometry";
		public Task<MikanResponse> getModelStencilRenderGeometry(int stencilId) // returns MikanStencilModelMesh
		{
			return _requestManager.SendRequestWithPayload<int>(k_getModelStencilRenderGeometry, stencilId);
		}
	}
}