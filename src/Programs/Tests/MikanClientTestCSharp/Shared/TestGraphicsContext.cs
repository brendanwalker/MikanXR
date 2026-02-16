using System;
using System.Collections.Generic;
using MikanXR;
using SharpDX.Windows;

namespace Mikan
{
	public abstract class TestGraphicsContext : IDisposable
	{
		protected TestApp _ownerApp;
		protected RenderForm _renderForm;
		protected Dictionary<int, TestCameraRenderTarget> _cameraRenderTargets;

		public TestApp OwnerApp => _ownerApp;

		protected TestGraphicsContext(TestApp ownerApp)
		{
			_ownerApp = ownerApp;
			_renderForm = ownerApp.RenderForm;
			_cameraRenderTargets = new Dictionary<int, TestCameraRenderTarget>();
		}

		// Camera Render Target Helpers
		public TestCameraRenderTarget GetCameraRenderTarget(int cameraId)
		{
			_cameraRenderTargets.TryGetValue(cameraId, out TestCameraRenderTarget renderTarget);
			return renderTarget;
		}

		public TestCameraRenderTarget GetOrAddCameraRenderTarget(MikanAPI mikanAPI, int cameraId)
		{
			if (!_cameraRenderTargets.TryGetValue(cameraId, out TestCameraRenderTarget renderTarget))
			{
				renderTarget = AllocateCameraRenderTarget(mikanAPI, cameraId);
				_cameraRenderTargets[cameraId] = renderTarget;
			}
			return renderTarget;
		}

		public void RemoveCameraRenderTarget(int cameraId)
		{
			if (_cameraRenderTargets.TryGetValue(cameraId, out TestCameraRenderTarget renderTarget))
			{
				renderTarget.Dispose();
				_cameraRenderTargets.Remove(cameraId);
			}
		}

		public void RemoveAllCameraRenderTargets()
		{
			foreach (var renderTarget in _cameraRenderTargets.Values)
			{
				renderTarget.Dispose();
			}
			_cameraRenderTargets.Clear();
		}

		// Abstract methods to be implemented by derived classes
		public abstract MikanClientGraphicsApi GetGraphicsApi();
		public abstract IntPtr GetGraphicsDeviceInterface();
		public abstract TestCameraRenderTarget AllocateCameraRenderTarget(MikanAPI mikanAPI, int cameraId);
		public abstract bool Create(int windowWidth, int windowHeight);
		public abstract void RenderMainTarget();
		public abstract bool RenderToCameraTarget(TestCameraRenderTarget cameraRenderTarget);
		public abstract void Dispose();
	}
}
