using System.Collections;
using System.Runtime.InteropServices;
using System.Buffers;
using System.Text.Json;
using System.Threading;
using System.Threading.Tasks;

namespace MikanXR
{	
	public class MikanVRDeviceAPI
	{
		private MikanRequestManager _requestManager;

		public MikanVRDeviceAPI(MikanRequestManager requestManager) : _requestManager(requestManager) {
			_requestManager.addResponseFactory<MikanVRDeviceList>();
			_requestManager.addResponseFactory<MikanVRDeviceInfo>();
		}

		public Task<MikanResponse> getVRDeviceList() // returns MikanVRDeviceList
		{
			return m_requestManager.sendRequest(nameof(getVRDeviceList));
		}
		
		public Task<MikanResponse> getVRDeviceInfo(int deviceId) // returns MikanVRDeviceInfo
		{
			return m_requestManager.sendRequestWithPayload<int>(nameof(getVRDeviceInfo), deviceId);
		}
		
		public Task<MikanResponse> subscribeToVRDevicePoseUpdates(int deviceId) // returns MikanResponse
		{
			return m_requestManager.sendRequestWithPayload<int>(nameof(subscribeToVRDevicePoseUpdates), deviceId);
		}
		
		public Task<MikanResponse> unsubscribeFromVRDevicePoseUpdates(int deviceId) // returns MikanResponse
		{
			return m_requestManager.sendRequestWithPayload<int>(nameof(unsubscribeFromVRDevicePoseUpdates), deviceId);
		}		
	}
}