using System.Threading.Tasks;

namespace MikanXR
{	
	public class MikanVRDeviceAPI
	{
		private MikanRequestManager _requestManager;

		public MikanVRDeviceAPI(MikanRequestManager requestManager)
		{
			_requestManager = requestManager;
			_requestManager.AddResponseFactory<MikanVRDeviceList>();
			_requestManager.AddResponseFactory<MikanVRDeviceInfo>();
		}

		private static readonly string k_getVRDeviceList = "getVRDeviceList";
		public Task<MikanResponse> getVRDeviceList() // returns MikanVRDeviceList
		{
			return _requestManager.SendRequest(k_getVRDeviceList);
		}
		
		private static readonly string k_getVRDeviceInfo = "getVRDeviceInfo";
		public Task<MikanResponse> getVRDeviceInfo(int deviceId) // returns MikanVRDeviceInfo
		{
			return _requestManager.SendRequestWithPayload<int>(k_getVRDeviceInfo, deviceId);
		}
		
		private static readonly string k_subscribeToVRDevicePoseUpdates = "subscribeToVRDevicePoseUpdates";
		public Task<MikanResponse> subscribeToVRDevicePoseUpdates(int deviceId) // returns MikanResponse
		{
			return _requestManager.SendRequestWithPayload<int>(k_subscribeToVRDevicePoseUpdates, deviceId);
		}
		
		private static readonly string k_unsubscribeFromVRDevicePoseUpdates = "unsubscribeFromVRDevicePoseUpdates";
		public Task<MikanResponse> unsubscribeFromVRDevicePoseUpdates(int deviceId) // returns MikanResponse
		{
			return _requestManager.SendRequestWithPayload<int>(k_unsubscribeFromVRDevicePoseUpdates, deviceId);
		}		
	}
}