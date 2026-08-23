using System;
using SharpDX;
using SharpDX.DXGI;
using D3D11 = SharpDX.Direct3D11;
using MikanXR;

namespace Mikan
{
	public class TestCameraRenderTarget_DX : TestCameraRenderTarget
	{
		private D3D11.Device _d3dDevice;
		private D3D11.DeviceContext _d3dDeviceContext;

		// Direct3D11 Color Target
		private D3D11.Texture2D _colorTargetTexture;
		private D3D11.RenderTargetView _colorTargetView;
		private D3D11.ShaderResourceView _colorTargetSRV;

		// Direct3D11 Depth Target
		private D3D11.Texture2D _floatDepthTargetTexture;
		private D3D11.DepthStencilView _floatDepthTargetView;
		private D3D11.ShaderResourceView _floatDepthTargetSRV;

		// Camera matrices and vectors
		private Matrix _projMatrix;
		private Matrix _viewMatrix;
		private Vector3 _cameraPosition;
		private Vector3 _cameraForward;
		private Vector3 _cameraUp;
		private Vector3 _cameraRight;

		public D3D11.Texture2D ColorTexture => _colorTargetTexture;
		public D3D11.Texture2D DepthTexture => _floatDepthTargetTexture;
		public D3D11.RenderTargetView ColorTargetView => _colorTargetView;
		public D3D11.DepthStencilView DepthTargetView => _floatDepthTargetView;
		public D3D11.ShaderResourceView ColorTextureSRV => _colorTargetSRV;
		public D3D11.ShaderResourceView FloatDepthTextureSRV => _floatDepthTargetSRV;

		public Matrix ProjectionMatrix => _projMatrix;
		public Matrix ViewMatrix => _viewMatrix;
		public Matrix ViewProjectionMatrix => _viewMatrix * _projMatrix;

		public Vector3 CameraPosition => _cameraPosition;
		public Vector3 CameraForward => _cameraForward;
		public Vector3 CameraUp => _cameraUp;
		public Vector3 CameraRight => _cameraRight;

		public bool IsInitialized => _colorTargetTexture != null && _floatDepthTargetTexture != null;

		public TestCameraRenderTarget_DX(
			TestGraphicsContext ownerContext,
            D3D11.Device d3dDevice,
			D3D11.DeviceContext d3dDeviceContext,
			int cameraId)
			: base(ownerContext, cameraId)
		{
			_d3dDevice = d3dDevice;
			_d3dDeviceContext = d3dDeviceContext;

			// Initialize matrices
			_projMatrix = Matrix.Identity;
			_viewMatrix = Matrix.Identity;
			_cameraPosition = Vector3.Zero;
			_cameraForward = Vector3.ForwardLH;
			_cameraUp = Vector3.Up;
			_cameraRight = Vector3.Right;
		}

		protected override bool CreateGraphicsAPIResources(int textureWidth, int textureHeight)
		{
			try
			{
				// Create color render target using utility
				if (!TestDirectXUtils.CreateColorRenderTargetResources(
					_d3dDevice,
					textureWidth,
					textureHeight,
					out _colorTargetTexture,
					out _colorTargetView,
					out _colorTargetSRV))
				{
					Console.WriteLine("ERROR: Failed to create color render target resources");
					return false;
				}

				// Create depth render target using utility
				if (!TestDirectXUtils.CreateDepthRenderTargetResources(
					_d3dDevice,
					textureWidth,
					textureHeight,
					out _floatDepthTargetTexture,
					out _floatDepthTargetView,
					out _floatDepthTargetSRV))
				{
					Console.WriteLine("ERROR: Failed to create depth render target resources");
					return false;
				}

				// Remember the size of the render target once created
				_width = textureWidth;
				_height = textureHeight;

				return true;
			}
			catch (Exception e)
			{
				Console.WriteLine($"ERROR: Failed to create graphics API resources: {e.Message}");
				FreeGraphicsAPIResources();
				return false;
			}
		}

		protected override void BindGraphicsAPIResource()
		{
			// Set the output render views
			_d3dDeviceContext.OutputMerger.SetTargets(_floatDepthTargetView, _colorTargetView);

			// Set the viewport dimensions
			_d3dDeviceContext.Rasterizer.SetViewport(
				new Viewport(0, 0, _width, _height));

			// Clear the screen
			_d3dDeviceContext.ClearRenderTargetView(_colorTargetView, new Color(0, 0, 0, 0));
			_d3dDeviceContext.ClearDepthStencilView(_floatDepthTargetView, D3D11.DepthStencilClearFlags.Depth, 1.0f, 0);
		}

		protected override void UnbindGraphicsAPIResource()
		{
			// Unbind render targets
			_d3dDeviceContext.OutputMerger.SetTargets((D3D11.DepthStencilView)null, (D3D11.RenderTargetView)null);
		}

		protected override void FreeGraphicsAPIResources()
		{
			_width = 0;
			_height = 0;

			Utilities.Dispose(ref _colorTargetSRV);
			Utilities.Dispose(ref _colorTargetView);
			Utilities.Dispose(ref _colorTargetTexture);

			Utilities.Dispose(ref _floatDepthTargetSRV);
			Utilities.Dispose(ref _floatDepthTargetView);
			Utilities.Dispose(ref _floatDepthTargetTexture);
		}

		protected override IntPtr GetGraphicsApiColorTexturePtr()
		{
			return _colorTargetTexture?.NativePointer ?? IntPtr.Zero;
		}

		protected override IntPtr GetGraphicsApiDepthTexturePtr()
		{
			return _floatDepthTargetTexture?.NativePointer ?? IntPtr.Zero;
		}

		protected override void UpdateCameraViewMatrix(MikanCameraNewFrameEvent newFrameEvent)
		{
			// Convert from Mikan coordinate system (right-handed, Y-up, Z-forward)
			// to DirectX coordinate system (left-handed, Y-up, Z-forward)
			_cameraPosition = new Vector3(
				newFrameEvent.camera_position.x,
				newFrameEvent.camera_position.y,
				-newFrameEvent.camera_position.z);

			_cameraForward = new Vector3(
				newFrameEvent.camera_forward.x,
				newFrameEvent.camera_forward.y,
				-newFrameEvent.camera_forward.z);

			_cameraUp = new Vector3(
				newFrameEvent.camera_up.x,
				newFrameEvent.camera_up.y,
				-newFrameEvent.camera_up.z);

			// Compute right vector from forward and up
			_cameraRight = Vector3.Cross(_cameraUp, _cameraForward);
			_cameraRight.Normalize();

			Vector3 targetPosition = _cameraPosition + _cameraForward;

			_viewMatrix = Matrix.LookAtLH(_cameraPosition, targetPosition, _cameraUp);
			_hasValidViewMatrix = true;
		}

		protected override void UpdateCameraProjectionMatrix(MikanCameraNewFrameEvent newFrameEvent)
		{
			float fx = (float)newFrameEvent.focal_length.x;
			float fy = (float)newFrameEvent.focal_length.y;
			float cx = (float)newFrameEvent.principal_point.x;
			float cy = (float)newFrameEvent.principal_point.y;
			float width = newFrameEvent.pixel_size.x;
			float height = newFrameEvent.pixel_size.y;
			float zNear = (float)newFrameEvent.z_bounds.x;
			float zFar = (float)newFrameEvent.z_bounds.y;

			// Convert camera intrinsics to DirectX projection matrix
			float left = -cx / fx * zNear;
			float right = (width - cx) / fx * zNear;
			float bottom = -(height - cy) / fy * zNear;
			float top = cy / fy * zNear;

			_projMatrix = Matrix.PerspectiveOffCenterLH(left, right, bottom, top, zNear, zFar);
			_hasValidProjMatrix = true;
		}
	}
}
