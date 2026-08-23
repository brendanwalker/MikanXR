using System;
using System.Collections.Generic;
using MikanXR;
using SharpDX;
using SharpDX.Windows;

namespace Mikan
{
	public abstract class TestGraphicsContext : IDisposable
	{
		protected TestApp _ownerApp;
		protected RenderForm _renderForm;
		protected Dictionary<int, TestCameraRenderTarget> _cameraRenderTargets;

		public TestApp OwnerApp => _ownerApp;
        public System.Drawing.Size WindowSize => _renderForm.ClientSize;

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

		public TestCameraRenderTarget GetOrAddCameraRenderTarget(int cameraId)
		{
			if (!_cameraRenderTargets.TryGetValue(cameraId, out TestCameraRenderTarget renderTarget))
			{
				renderTarget = AllocateCameraRenderTarget(cameraId);
				_cameraRenderTargets[cameraId] = renderTarget;
			}
			return renderTarget;
		}

		public void RemoveCameraRenderTarget(MikanAPI mikanAPI, int cameraId)
		{
			if (_cameraRenderTargets.TryGetValue(cameraId, out TestCameraRenderTarget renderTarget))
			{
				renderTarget.Dispose(mikanAPI);
				_cameraRenderTargets.Remove(cameraId);
			}
		}

		public void RemoveAllCameraRenderTargets(MikanAPI mikanAPI)
		{
			foreach (var renderTarget in _cameraRenderTargets.Values)
			{
				renderTarget.Dispose(mikanAPI);
			}
			_cameraRenderTargets.Clear();
		}

		// Abstract methods to be implemented by derived classes
		public abstract MikanClientGraphicsApi GetGraphicsApi();
		public abstract IntPtr GetGraphicsDeviceInterface();
		public abstract TestCameraRenderTarget AllocateCameraRenderTarget(int cameraId);
		public abstract bool Create(int windowWidth, int windowHeight);
		public abstract void RenderMainTarget();
		public abstract bool RenderToCameraTarget(TestCameraRenderTarget cameraRenderTarget);
		public abstract void Dispose();
	}
}
