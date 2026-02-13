using SharpDX;
using SharpDX.DXGI;
using D3D11 = SharpDX.Direct3D11;
using System;
using MikanXR;
using SharpDX.Direct3D11;
using SharpDX.Direct3D;

namespace Mikan
{
	class MikanCameraRenderTarget_DX : MikanCameraRenderTarget
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

		// Camera matrices
		private Matrix _projMatrix;
		private Matrix _viewMatrix;

		public D3D11.Texture2D ColorTexture => _colorTargetTexture;
		public D3D11.Texture2D DepthTexture => _floatDepthTargetTexture;

		public D3D11.RenderTargetView ColorTargetView => _colorTargetView;
		public D3D11.DepthStencilView DepthTargetView => _floatDepthTargetView;

		public D3D11.ShaderResourceView ColorTextureSRV => _colorTargetSRV;
		public D3D11.ShaderResourceView FloatDepthTextureSRV => _floatDepthTargetSRV;

		public Matrix ProjectionMatrix => _projMatrix;
		public Matrix ViewMatrix => _viewMatrix;

		public bool IsInitialized => _colorTargetTexture != null && _floatDepthTargetTexture != null;

		public MikanCameraRenderTarget_DX(
			MikanAPI mikanAPI,
			D3D11.Device d3dDevice,
			D3D11.DeviceContext d3dDeviceContext,
			int cameraId)
			: base(mikanAPI, cameraId)
		{
			_d3dDevice = d3dDevice;
			_d3dDeviceContext = d3dDeviceContext;
		}

		Format GetDepthResourceFormat(Format depthformat)
		{
			Format resformat = Format.Unknown;

			switch (depthformat)
			{
				case Format.D16_UNorm:
					resformat = Format.R16G16_Typeless;
					break;
				case Format.D24_UNorm_S8_UInt:
					resformat = Format.R24G8_Typeless;
					break;
				case Format.D32_Float:
					resformat = Format.R32_Typeless;
					break;
				case Format.D32_Float_S8X24_UInt:
					resformat = Format.R32G8X24_Typeless;
					break;
			}

			return resformat;
		}

		Format GetDepthSRVFormat(Format depthformat)
		{
			Format srvformat = Format.Unknown;

			switch (depthformat)
			{
				case Format.D16_UNorm:
					srvformat = Format.R16_Float;
					break;
				case Format.D24_UNorm_S8_UInt:
					srvformat = Format.R24_UNorm_X8_Typeless;
					break;
				case Format.D32_Float:
					srvformat = Format.R32_Float;
					break;
				case Format.D32_Float_S8X24_UInt:
					srvformat = Format.R32_Float_X8X24_Typeless;
					break;
			}

			return srvformat;
		}

		protected override bool CreateGraphicsAPIResources(int textureWidth, int textureHeight)
		{
			// Create the render target resources
			// -------

			// Create the color render target texture.
			_colorTargetTexture = new D3D11.Texture2D(
				_d3dDevice,
				new D3D11.Texture2DDescription()
				{
					Width = textureWidth,
					Height = textureHeight,
					MipLevels = 1,
					ArraySize = 1,
					Format = Format.B8G8R8A8_Typeless,
					SampleDescription = new SampleDescription(1, 0),
					Usage = D3D11.ResourceUsage.Default,
					BindFlags = D3D11.BindFlags.RenderTarget | D3D11.BindFlags.ShaderResource,
					CpuAccessFlags = D3D11.CpuAccessFlags.None,
					OptionFlags = D3D11.ResourceOptionFlags.None
				});

			// Create the render target view
			_colorTargetView = new D3D11.RenderTargetView(
				_d3dDevice,
				_colorTargetTexture,
				new RenderTargetViewDescription()
				{
					Format = Format.B8G8R8A8_UNorm,
					Dimension = RenderTargetViewDimension.Texture2D,
					Texture2D = new RenderTargetViewDescription.Texture2DResource() { MipSlice = 0 }
				});

			// Create the shader resource view
			_colorTargetSRV = new D3D11.ShaderResourceView(
				_d3dDevice,
				_colorTargetTexture,
				new ShaderResourceViewDescription()
				{
					Format = Format.B8G8R8A8_UNorm,
					Dimension = ShaderResourceViewDimension.Texture2D,
					Texture2D = new ShaderResourceViewDescription.Texture2DResource() { MipLevels = 1, MostDetailedMip = 0 }
				});

			// Create the depth resources
			// -------
			Format depthViewFormat = Format.D32_Float;
			Format resformat = GetDepthResourceFormat(depthViewFormat);
			Format srvformat = GetDepthSRVFormat(depthViewFormat);

			// Create the depth render target texture.
			_floatDepthTargetTexture = new D3D11.Texture2D(
				_d3dDevice,
				new D3D11.Texture2DDescription()
				{
					Width = textureWidth,
					Height = textureHeight,
					ArraySize = 1,
					BindFlags = D3D11.BindFlags.DepthStencil | D3D11.BindFlags.ShaderResource,
					CpuAccessFlags = D3D11.CpuAccessFlags.None,
					Format = resformat,
					MipLevels = 1,
					OptionFlags = D3D11.ResourceOptionFlags.None,
					SampleDescription = new SampleDescription(1, 0),
					Usage = D3D11.ResourceUsage.Default,
				});

			// Create the depth stencil view
			_floatDepthTargetView = new D3D11.DepthStencilView(
				_d3dDevice,
				_floatDepthTargetTexture,
				new DepthStencilViewDescription()
				{
					Format = depthViewFormat,
					Dimension = DepthStencilViewDimension.Texture2D,
					Texture2D = new DepthStencilViewDescription.Texture2DResource() { MipSlice = 0 }
				});

			// Create the shader resource view
			_floatDepthTargetSRV = new D3D11.ShaderResourceView(
				_d3dDevice,
				_floatDepthTargetTexture,
				new ShaderResourceViewDescription()
				{
					Format = srvformat,
					Dimension = ShaderResourceViewDimension.Texture2D,
					Texture2D = new ShaderResourceViewDescription.Texture2DResource() { MipLevels = 1, MostDetailedMip = 0 }
				});

			// Remember the size of the render target once created
			_width = textureWidth;
			_height = textureHeight;

			return true;
		}

		protected override void BindGraphicsAPIResource()
		{
			// Set the output render views
			_d3dDeviceContext.OutputMerger.SetTargets(_floatDepthTargetView, _colorTargetView);

			// Set the viewport dimensions
			_d3dDeviceContext.Rasterizer.SetViewport(
				new Viewport(
					0, 0,
					_colorTargetTexture.Description.Width, _colorTargetTexture.Description.Height));

			// Clear the screen
			_d3dDeviceContext.ClearRenderTargetView(_colorTargetView, new SharpDX.Color(0, 0, 0, 0));
			_d3dDeviceContext.ClearDepthStencilView(_floatDepthTargetView, DepthStencilClearFlags.Depth, 1.0f, 0);
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
			Vector3 cameraForward = new Vector3(newFrameEvent.camera_forward.x, newFrameEvent.camera_forward.y, -newFrameEvent.camera_forward.z);
			Vector3 cameraUp = new Vector3(newFrameEvent.camera_forward.x, newFrameEvent.camera_forward.y, -newFrameEvent.camera_up.z);
			Vector3 cameraPosition = new Vector3(newFrameEvent.camera_forward.x, newFrameEvent.camera_forward.y, -newFrameEvent.camera_position.z);

			Vector3 targetPosition = cameraPosition + cameraForward;

			_viewMatrix = Matrix.LookAtLH(cameraPosition, targetPosition, cameraUp);
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
