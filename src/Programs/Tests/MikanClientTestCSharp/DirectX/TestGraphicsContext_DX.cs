using System;
using System.Runtime.InteropServices;
using SharpDX;
using SharpDX.D3DCompiler;
using SharpDX.Direct3D;
using SharpDX.DXGI;
using SharpDX.Windows;
using D3D11 = SharpDX.Direct3D11;
using MikanXR;

namespace Mikan
{
	public class TestGraphicsContext_DX : TestGraphicsContext
	{
		// Window dimensions
		private int _windowWidth;
		private int _windowHeight;

		// Direct3D11 Device and Swap Chain
		private D3D11.Device _d3dDevice;
		private D3D11.DeviceContext _d3dDeviceContext;
		private SwapChain _swapChain;
		private D3D11.Texture2D _defaultColorBuffer;
		private D3D11.RenderTargetView _defaultColorTargetView;
		private D3D11.Texture2D _defaultDepthBuffer;
		private D3D11.DepthStencilView _defaultFloatDepthView;

		// Cube Shader State
		private D3D11.VertexShader _cubeVertexShader;
		private D3D11.PixelShader _cubePixelShader;
		private ShaderSignature _cubeInputSignature;
		private D3D11.Buffer _cubeVertices;
		private D3D11.InputLayout _cubeInputLayout;
		private int _cubeVertexCount;
		private D3D11.Buffer _cubeConstantBuffer;

		// Depth Normalize Shader State
		private D3D11.VertexShader _depthNormalizeVertexShader;
		private D3D11.PixelShader _depthNormalizePixelShader;
		private D3D11.Buffer _depthNormalizeConstantBuffer;
		private D3D11.SamplerState _depthNormalizeSamplerState;

		// Depth Normalize Color Target (intermediate render target for depth visualization)
		private int _depthNormalizeTargetWidth = 0;
		private int _depthNormalizeTargetHeight = 0;
		private D3D11.Texture2D _depthNormalizeColorTargetTexture;
		private D3D11.RenderTargetView _depthNormalizeColorTargetView;
		private D3D11.ShaderResourceView _depthNormalizeColorTargetSRV;

		// Quad Texture Shader State
		private D3D11.VertexShader _quadTextureVertexShader;
		private D3D11.PixelShader _quadTexturePixelShader;
		private D3D11.SamplerState _quadTextureSamplerState;
		private D3D11.Buffer _quadVertices;
		private ShaderSignature _quadInputSignature;
		private D3D11.InputLayout _quadInputLayout;

		// Camera State
        private int _lastRenderedCameraId= -1;

		public TestGraphicsContext_DX(TestApp ownerApp) : base(ownerApp)
		{
		}

		// Graphics API Interface Implementation
		public override MikanClientGraphicsApi GetGraphicsApi()
		{
			return MikanClientGraphicsApi.Direct3D11;
		}

		public override IntPtr GetGraphicsDeviceInterface()
		{
			return _d3dDevice?.NativePointer ?? IntPtr.Zero;
		}

		public override TestCameraRenderTarget AllocateCameraRenderTarget(int cameraId)
		{
			return new TestCameraRenderTarget_DX(this, _d3dDevice, _d3dDeviceContext, cameraId);
		}

		public override bool Create(int windowWidth, int windowHeight)
		{
			_windowWidth = windowWidth;
			_windowHeight = windowHeight;

			try
			{
				// Create the render form (done by TestApp already, we just need the handle)
				// Initialize DirectX device and swap chain
				if (!InitializeSwapChain(windowWidth, windowHeight))
				{
					Console.WriteLine("ERROR: Failed to initialize swap chain");
					return false;
				}

				// Initialize shaders
				if (!InitializeCubeShader() ||
					!InitializeDepthNormalizeShader() ||
					!InitializeQuadTextureShader())
				{
					Console.WriteLine("ERROR: Failed to initialize shaders");
					return false;
				}

				// Initialize geometry
				InitializeCubeGeometry();
				InitializeQuadGeometry();

				return true;
			}
			catch (Exception e)
			{
				Console.WriteLine($"ERROR: Failed to create graphics context: {e.Message}");
				return false;
			}
		}

		private bool InitializeSwapChain(int windowWidth, int windowHeight)
		{
			// Create swap chain description
			var swapChainDesc = new SwapChainDescription()
			{
				BufferCount = 1,
				ModeDescription = new ModeDescription(
					windowWidth,
					windowHeight,
					new Rational(60, 1),
					Format.B8G8R8A8_UNorm),
				IsWindowed = true,
				OutputHandle = _renderForm.Handle,
				SampleDescription = new SampleDescription(1, 0),
				SwapEffect = SwapEffect.Discard,
				Usage = Usage.RenderTargetOutput,
			};

			// Create device and swap chain
			D3D11.Device.CreateWithSwapChain(
				DriverType.Hardware,
				D3D11.DeviceCreationFlags.None,
				swapChainDesc,
				out _d3dDevice,
				out _swapChain);

			_d3dDeviceContext = _d3dDevice?.ImmediateContext;

			// Create render target view for back buffer
			CreateRenderTarget();

			return true;
		}

		private void CreateRenderTarget()
		{
			if (_swapChain == null) return;

			// Get the back buffer from the swap chain
			_defaultColorBuffer = _swapChain.GetBackBuffer<D3D11.Texture2D>(0);

			// Create a render target view for the back buffer
			_defaultColorTargetView = new D3D11.RenderTargetView(_d3dDevice, _defaultColorBuffer);

			// Create the depth buffer
			_defaultDepthBuffer = new D3D11.Texture2D(_d3dDevice, new D3D11.Texture2DDescription()
			{
				Format = Format.D32_Float_S8X24_UInt,
				ArraySize = 1,
				MipLevels = 1,
				Width = _windowWidth,
				Height = _windowHeight,
				SampleDescription = new SampleDescription(1, 0),
				Usage = D3D11.ResourceUsage.Default,
				BindFlags = D3D11.BindFlags.DepthStencil,
				CpuAccessFlags = D3D11.CpuAccessFlags.None,
				OptionFlags = D3D11.ResourceOptionFlags.None
			});

			// Create the depth buffer view
			_defaultFloatDepthView = new D3D11.DepthStencilView(_d3dDevice, _defaultDepthBuffer);

			// Setup the targets and viewport for rendering
			_d3dDeviceContext.Rasterizer.SetViewport(new Viewport(0, 0, _windowWidth, _windowHeight, 0.0f, 1.0f));
			_d3dDeviceContext.OutputMerger.SetTargets(_defaultFloatDepthView, _defaultColorTargetView);
		}

		private void CleanupRenderTarget()
		{
			Utilities.Dispose(ref _defaultFloatDepthView);
			Utilities.Dispose(ref _defaultDepthBuffer);
			Utilities.Dispose(ref _defaultColorTargetView);
			Utilities.Dispose(ref _defaultColorBuffer);
		}

		// Shader Initialization Methods

		[StructLayout(LayoutKind.Sequential)]
		private struct DepthNormalizeConstantBuffer
		{
			public float zNear;
			public float zFar;
			public float padding0;
			public float padding1;
		}

		private bool InitializeCubeShader()
		{
			string shaderCodeString = @"
				struct VS_INPUT
				{
					float4 pos : POSITION;
					float4 col : COLOR;
				};
				struct PS_INPUT
				{
					float4 pos : SV_POSITION;
					float4 col : COLOR;
				};

				float4x4 worldViewProj;

				PS_INPUT vs_main(VS_INPUT input)
				{
					PS_INPUT output = (PS_INPUT)0;
					output.pos = mul(input.pos, worldViewProj);
					output.col = input.col;
					return output;
				}

				float4 ps_main(PS_INPUT input) : SV_TARGET
				{
					return input.col;
				}";

			try
			{
				var vertexShaderByteCode = ShaderBytecode.Compile(shaderCodeString, "vs_main", "vs_4_0");
				if (vertexShaderByteCode.HasErrors)
				{
					Console.WriteLine($"ERROR: Cube vertex shader: {vertexShaderByteCode.Message}");
					return false;
				}
				_cubeVertexShader = new D3D11.VertexShader(_d3dDevice, vertexShaderByteCode);

				var pixelShaderByteCode = ShaderBytecode.Compile(shaderCodeString, "ps_main", "ps_4_0");
				if (pixelShaderByteCode.HasErrors)
				{
					Console.WriteLine($"ERROR: Cube pixel shader: {pixelShaderByteCode.Message}");
					return false;
				}
				_cubePixelShader = new D3D11.PixelShader(_d3dDevice, pixelShaderByteCode);

				_cubeInputSignature = ShaderSignature.GetInputSignature(vertexShaderByteCode);
				_cubeConstantBuffer = new D3D11.Buffer(_d3dDevice, Utilities.SizeOf<Matrix>(),
					D3D11.ResourceUsage.Default, D3D11.BindFlags.ConstantBuffer,
					D3D11.CpuAccessFlags.None, D3D11.ResourceOptionFlags.None, 0);

				return true;
			}
			catch (Exception e)
			{
				Console.WriteLine($"ERROR: InitializeCubeShader: {e.Message}");
				return false;
			}
		}

		private bool InitializeDepthNormalizeShader()
		{
			string shaderCodeString = @"
				Texture2D<float> InputTexture : register(t0);
				SamplerState samLinear : register(s0);

				cbuffer ConstantBuffer : register(b0)
				{
					float zNear;
					float zFar;
				};

				struct VS_INPUT
				{
					float3 position : POSITION;
					float2 texCoord : TEXCOORD;
				};

				struct PS_INPUT
				{
					float4 pos : SV_POSITION;
					float2 uv : TEXCOORD;
				};

				PS_INPUT vs_main(VS_INPUT input)
				{
					PS_INPUT output;
					output.pos = float4(input.position, 1.0f);
					output.uv = input.texCoord;
					return output;
				}

				float4 ps_main(PS_INPUT input) : SV_TARGET
				{
					float depth = InputTexture.Sample(samLinear, input.uv).r;
					float eyeDepth = zFar * zNear / ((zNear - zFar) * depth + zFar);
					float zNorm = (eyeDepth - zNear) / (zFar - zNear);
					return float4(zNorm, zNorm, zNorm, 1.0);
				}";

			try
			{
				var vertexShaderByteCode = ShaderBytecode.Compile(shaderCodeString, "vs_main", "vs_4_0");
				_depthNormalizeVertexShader = new D3D11.VertexShader(_d3dDevice, vertexShaderByteCode);

				var pixelShaderByteCode = ShaderBytecode.Compile(shaderCodeString, "ps_main", "ps_4_0");
				_depthNormalizePixelShader = new D3D11.PixelShader(_d3dDevice, pixelShaderByteCode);

				_depthNormalizeConstantBuffer = new D3D11.Buffer(_d3dDevice,
					Utilities.SizeOf<DepthNormalizeConstantBuffer>(),
					D3D11.ResourceUsage.Dynamic, D3D11.BindFlags.ConstantBuffer,
					D3D11.CpuAccessFlags.Write, D3D11.ResourceOptionFlags.None, 0);

				_depthNormalizeSamplerState = new D3D11.SamplerState(_d3dDevice,
					new D3D11.SamplerStateDescription()
					{
						Filter = D3D11.Filter.MinMagMipPoint,
						AddressU = D3D11.TextureAddressMode.Clamp,
						AddressV = D3D11.TextureAddressMode.Clamp,
						AddressW = D3D11.TextureAddressMode.Clamp
					});

				return true;
			}
			catch (Exception e)
			{
				Console.WriteLine($"ERROR: InitializeDepthNormalizeShader: {e.Message}");
				return false;
			}
		}

		private bool InitializeQuadTextureShader()
		{
			string shaderCodeString = @"
				Texture2D<float4> InputTexture : register(t0);
				SamplerState samLinear : register(s0);

				struct VS_INPUT
				{
					float3 position : POSITION;
					float2 texCoord : TEXCOORD;
				};

				struct PS_INPUT
				{
					float4 pos : SV_POSITION;
					float2 uv : TEXCOORD;
				};

				PS_INPUT vs_main(VS_INPUT input)
				{
					PS_INPUT output;
					output.pos = float4(input.position, 1.0f);
					output.uv = input.texCoord;
					return output;
				}

				float4 ps_main(PS_INPUT input) : SV_TARGET
				{
					return InputTexture.Sample(samLinear, input.uv);
				}";

			try
			{
				var vertexShaderByteCode = ShaderBytecode.Compile(shaderCodeString, "vs_main", "vs_4_0");
				_quadTextureVertexShader = new D3D11.VertexShader(_d3dDevice, vertexShaderByteCode);

				var pixelShaderByteCode = ShaderBytecode.Compile(shaderCodeString, "ps_main", "ps_4_0");
				_quadTexturePixelShader = new D3D11.PixelShader(_d3dDevice, pixelShaderByteCode);

				_quadInputSignature = ShaderSignature.GetInputSignature(vertexShaderByteCode);

				_quadTextureSamplerState = new D3D11.SamplerState(_d3dDevice,
					new D3D11.SamplerStateDescription()
					{
						Filter = D3D11.Filter.MinMagMipLinear,
						AddressU = D3D11.TextureAddressMode.Clamp,
						AddressV = D3D11.TextureAddressMode.Clamp,
						AddressW = D3D11.TextureAddressMode.Clamp
					});

				return true;
			}
			catch (Exception e)
			{
				Console.WriteLine($"ERROR: InitializeQuadTextureShader: {e.Message}");
				return false;
			}
		}

        // Geometry Initialization Methods
        private const int kCubeVertexSize = (4 + 4) * sizeof(float); // position (float4) + color (float4)

        private void InitializeCubeGeometry()
		{
			_cubeInputLayout = new D3D11.InputLayout(_d3dDevice, _cubeInputSignature, new[]
			{
				new D3D11.InputElement("POSITION", 0, Format.R32G32B32A32_Float, 0, 0),
				new D3D11.InputElement("COLOR", 0, Format.R32G32B32A32_Float, 16, 0)
			});

			_cubeVertices = D3D11.Buffer.Create(_d3dDevice, D3D11.BindFlags.VertexBuffer, new[]
			{
				// Front (Red), Back (Green), Top (Blue), Bottom (Yellow), Left (Magenta), Right (Cyan)
				new Vector4(-1,-1,-1,1), new Vector4(1,0,0,1), new Vector4(-1,1,-1,1), new Vector4(1,0,0,1),
				new Vector4(1,1,-1,1), new Vector4(1,0,0,1), new Vector4(-1,-1,-1,1), new Vector4(1,0,0,1),
				new Vector4(1,1,-1,1), new Vector4(1,0,0,1), new Vector4(1,-1,-1,1), new Vector4(1,0,0,1),

				new Vector4(-1,-1,1,1), new Vector4(0,1,0,1), new Vector4(1,1,1,1), new Vector4(0,1,0,1),
				new Vector4(-1,1,1,1), new Vector4(0,1,0,1), new Vector4(-1,-1,1,1), new Vector4(0,1,0,1),
				new Vector4(1,-1,1,1), new Vector4(0,1,0,1), new Vector4(1,1,1,1), new Vector4(0,1,0,1),

				new Vector4(-1,1,-1,1), new Vector4(0,0,1,1), new Vector4(-1,1,1,1), new Vector4(0,0,1,1),
				new Vector4(1,1,1,1), new Vector4(0,0,1,1), new Vector4(-1,1,-1,1), new Vector4(0,0,1,1),
				new Vector4(1,1,1,1), new Vector4(0,0,1,1), new Vector4(1,1,-1,1), new Vector4(0,0,1,1),

				new Vector4(-1,-1,-1,1), new Vector4(1,1,0,1), new Vector4(1,-1,1,1), new Vector4(1,1,0,1),
				new Vector4(-1,-1,1,1), new Vector4(1,1,0,1), new Vector4(-1,-1,-1,1), new Vector4(1,1,0,1),
				new Vector4(1,-1,-1,1), new Vector4(1,1,0,1), new Vector4(1,-1,1,1), new Vector4(1,1,0,1),

				new Vector4(-1,-1,-1,1), new Vector4(1,0,1,1), new Vector4(-1,-1,1,1), new Vector4(1,0,1,1),
				new Vector4(-1,1,1,1), new Vector4(1,0,1,1), new Vector4(-1,-1,-1,1), new Vector4(1,0,1,1),
				new Vector4(-1,1,1,1), new Vector4(1,0,1,1), new Vector4(-1,1,-1,1), new Vector4(1,0,1,1),

				new Vector4(1,-1,-1,1), new Vector4(0,1,1,1), new Vector4(1,1,1,1), new Vector4(0,1,1,1),
				new Vector4(1,-1,1,1), new Vector4(0,1,1,1), new Vector4(1,-1,-1,1), new Vector4(0,1,1,1),
				new Vector4(1,1,-1,1), new Vector4(0,1,1,1), new Vector4(1,1,1,1), new Vector4(0,1,1,1),
			});

			_cubeVertexCount = 36;
		}

        [StructLayout(LayoutKind.Sequential)]
        internal struct QuadVertex
        {
            public Vector3 position;
            public Vector2 texture;
        }
        static int kQuadVertexSize = Marshal.SizeOf(typeof(QuadVertex));

        private void InitializeQuadGeometry()
		{
			_quadInputLayout = new D3D11.InputLayout(_d3dDevice, _quadInputSignature, new[]
			{
				new D3D11.InputElement("POSITION", 0, Format.R32G32B32_Float, 0, 0),
				new D3D11.InputElement("TEXCOORD", 0, Format.R32G32_Float, 12, 0)
			});

			_quadVertices = D3D11.Buffer.Create(_d3dDevice, D3D11.BindFlags.VertexBuffer, new[]
			{
                new QuadVertex() { position = new Vector3( 1.0f, -1.0f, 0.0f), texture = new Vector2(1.0f, 1.0f) },
                new QuadVertex() { position = new Vector3(-1.0f, -1.0f, 0.0f), texture = new Vector2(0.0f, 1.0f) },
                new QuadVertex() { position = new Vector3(-1.0f,  1.0f, 0.0f), texture = new Vector2(0.0f, 0.0f) },

                new QuadVertex() { position = new Vector3( 1.0f,  1.0f, 0.0f), texture = new Vector2(1.0f, 0.0f) },
                new QuadVertex() { position = new Vector3( 1.0f, -1.0f, 0.0f), texture = new Vector2(1.0f, 1.0f) },
                new QuadVertex() { position = new Vector3(-1.0f,  1.0f, 0.0f), texture = new Vector2(0.0f, 0.0f) },
            });
		}

		// Rendering Methods

		private void RenderCube(Matrix viewProj, Vector3 cubePosition, Vector3 cameraForward, Vector3 cameraUp, Vector3 cameraRight)
		{
			float time = _ownerApp.TimeSeconds;
			Vector3 cubeOffset = _ownerApp.CubeOffset;

			Vector3 position = cubePosition + cameraForward * cubeOffset.Z + cameraUp * cubeOffset.Y + cameraRight * cubeOffset.X;

			Matrix cubeXform =
				Matrix.Scaling(0.1f) *
				Matrix.RotationX(time) *
				Matrix.RotationY(time * 2.0f) *
				Matrix.RotationZ(time * 0.7f) *
				Matrix.Translation(position);

			Matrix mvp = Matrix.Transpose(cubeXform * viewProj);
			_d3dDeviceContext.UpdateSubresource(ref mvp, _cubeConstantBuffer);

			_d3dDeviceContext.InputAssembler.SetVertexBuffers(0, new D3D11.VertexBufferBinding(_cubeVertices, kCubeVertexSize, 0));
			_d3dDeviceContext.InputAssembler.InputLayout = _cubeInputLayout;
			_d3dDeviceContext.VertexShader.Set(_cubeVertexShader);
			_d3dDeviceContext.PixelShader.Set(_cubePixelShader);
			_d3dDeviceContext.VertexShader.SetConstantBuffer(0, _cubeConstantBuffer);
			_d3dDeviceContext.InputAssembler.PrimitiveTopology = PrimitiveTopology.TriangleList;
			_d3dDeviceContext.Draw(_cubeVertexCount, 0);
		}

		private void RenderColorTexture(D3D11.ShaderResourceView textureSRV)
		{
			_d3dDeviceContext.InputAssembler.SetVertexBuffers(0, new D3D11.VertexBufferBinding(_quadVertices, kQuadVertexSize, 0));
			_d3dDeviceContext.InputAssembler.InputLayout = _quadInputLayout;
			_d3dDeviceContext.VertexShader.Set(_quadTextureVertexShader);
			_d3dDeviceContext.PixelShader.Set(_quadTexturePixelShader);
			_d3dDeviceContext.PixelShader.SetShaderResource(0, textureSRV);
			_d3dDeviceContext.PixelShader.SetSampler(0, _quadTextureSamplerState);
			_d3dDeviceContext.InputAssembler.PrimitiveTopology = PrimitiveTopology.TriangleList;
			_d3dDeviceContext.Draw(6, 0);
		}

		private void RenderPackedDepthTexture(TestCameraRenderTarget_DX dxRenderTarget, MikanAPI mikanApi)
		{
			IntPtr packedDepthTextureHandle= mikanApi.GetCameraPackDepthTextureResourcePtr(dxRenderTarget.CameraId);

			if (packedDepthTextureHandle != IntPtr.Zero)
			{
				var packDepthSRV = new D3D11.ShaderResourceView(packedDepthTextureHandle);
				RenderColorTexture(packDepthSRV);
			}
		}

		private void RenderNormalizedDepthTexture(TestCameraRenderTarget_DX dxRenderTarget)
		{
			if (_depthNormalizeTargetWidth != dxRenderTarget.Width || _depthNormalizeTargetHeight != dxRenderTarget.Height)
			{
				Utilities.Dispose(ref _depthNormalizeColorTargetSRV);
				Utilities.Dispose(ref _depthNormalizeColorTargetView);
				Utilities.Dispose(ref _depthNormalizeColorTargetTexture);

				if (TestDirectXUtils.CreateColorRenderTargetResources(
						_d3dDevice, 
						dxRenderTarget.Width, 
						dxRenderTarget.Height,
						out _depthNormalizeColorTargetTexture, 
						out _depthNormalizeColorTargetView, 
						out _depthNormalizeColorTargetSRV))
				{
					_depthNormalizeTargetWidth = dxRenderTarget.Width;
					_depthNormalizeTargetHeight = dxRenderTarget.Height;
				}
			}

            // Bind the main render target view
            _d3dDeviceContext.OutputMerger.SetTargets(_depthNormalizeColorTargetView);

            // Assign the quad vertices
            _d3dDeviceContext.InputAssembler.SetVertexBuffers(0, new D3D11.VertexBufferBinding(_quadVertices, kQuadVertexSize, 0));
			_d3dDeviceContext.InputAssembler.InputLayout = _quadInputLayout;

            // Assign the normalized depth shader
            _d3dDeviceContext.VertexShader.Set(_depthNormalizeVertexShader);
			_d3dDeviceContext.PixelShader.Set(_depthNormalizePixelShader);

            // Set the texture
            _d3dDeviceContext.PixelShader.SetShaderResource(0, dxRenderTarget.FloatDepthTextureSRV);
			_d3dDeviceContext.PixelShader.SetSampler(0, _depthNormalizeSamplerState);

            // Update the depth normalization constants
            var depthConstants = new DepthNormalizeConstantBuffer { 
				zNear = dxRenderTarget.ZNear, 
				zFar = dxRenderTarget.ZFar 
			};

			_d3dDeviceContext.MapSubresource(_depthNormalizeConstantBuffer, D3D11.MapMode.WriteDiscard, D3D11.MapFlags.None, out DataStream stream);
			stream.Write(depthConstants);
			_d3dDeviceContext.UnmapSubresource(_depthNormalizeConstantBuffer, 0);
			_d3dDeviceContext.PixelShader.SetConstantBuffer(0, _depthNormalizeConstantBuffer);

            // Draw the quad
            _d3dDeviceContext.InputAssembler.PrimitiveTopology = PrimitiveTopology.TriangleList;
			_d3dDeviceContext.Draw(6, 0);
		}

		public override void RenderMainTarget()
		{
			_d3dDeviceContext.ClearRenderTargetView(_defaultColorTargetView, new Color(0, 0, 255, 255));
			_d3dDeviceContext.ClearDepthStencilView(_defaultFloatDepthView, D3D11.DepthStencilClearFlags.Depth, 1.0f, 0);

			TestCameraRenderTarget renderTarget = GetCameraRenderTarget(_lastRenderedCameraId);

			if (renderTarget != null)
			{
				var dxRenderTarget = (TestCameraRenderTarget_DX)renderTarget;
				TestRenderMode renderMode = _ownerApp.RenderMode;

				_d3dDeviceContext.OutputMerger.SetTargets(_defaultFloatDepthView, _defaultColorTargetView);
				_d3dDeviceContext.Rasterizer.SetViewport(new Viewport(0, 0, _windowWidth, _windowHeight));

				switch (renderMode)
				{
					case TestRenderMode.Color:
						RenderColorTexture(dxRenderTarget.ColorTextureSRV);
						break;
					case TestRenderMode.DepthNormalize:
						RenderColorTexture(_depthNormalizeColorTargetSRV);
						break;
					case TestRenderMode.PackedDepth:
						RenderPackedDepthTexture(dxRenderTarget, _ownerApp.MikanAPI);
						break;
				}
			}

			_swapChain.Present(1, PresentFlags.None);
		}

		public override bool RenderToCameraTarget(TestCameraRenderTarget cameraRenderTarget)
		{
			var dxRenderTarget = (TestCameraRenderTarget_DX)cameraRenderTarget;
			Matrix viewProj = dxRenderTarget.ViewProjectionMatrix;

			RenderCube(viewProj, dxRenderTarget.CameraPosition, dxRenderTarget.CameraForward,
				dxRenderTarget.CameraUp, dxRenderTarget.CameraRight);

            // If we're in depth normalize mode, also render the normalized depth texture for this camera.
            if (_ownerApp.RenderMode == TestRenderMode.DepthNormalize)
			{
				RenderNormalizedDepthTexture(dxRenderTarget);
            }

            // Remember the last rendered camera ID
            // We will render this camera in renderMainTarget
            _lastRenderedCameraId = cameraRenderTarget.CameraId;

			return true;
		}

		public override void Dispose()
		{
			RemoveAllCameraRenderTargets(null);

			Utilities.Dispose(ref _cubeConstantBuffer);
			Utilities.Dispose(ref _cubeInputLayout);
			Utilities.Dispose(ref _cubeVertices);
			Utilities.Dispose(ref _cubeInputSignature);
			Utilities.Dispose(ref _cubePixelShader);
			Utilities.Dispose(ref _cubeVertexShader);

			Utilities.Dispose(ref _depthNormalizeSamplerState);
			Utilities.Dispose(ref _depthNormalizeConstantBuffer);
			Utilities.Dispose(ref _depthNormalizePixelShader);
			Utilities.Dispose(ref _depthNormalizeVertexShader);

			Utilities.Dispose(ref _depthNormalizeColorTargetSRV);
			Utilities.Dispose(ref _depthNormalizeColorTargetView);
			Utilities.Dispose(ref _depthNormalizeColorTargetTexture);

			Utilities.Dispose(ref _quadInputLayout);
			Utilities.Dispose(ref _quadVertices);
			Utilities.Dispose(ref _quadTextureSamplerState);
			Utilities.Dispose(ref _quadInputSignature);
			Utilities.Dispose(ref _quadTexturePixelShader);
			Utilities.Dispose(ref _quadTextureVertexShader);

			CleanupRenderTarget();

			Utilities.Dispose(ref _swapChain);
			Utilities.Dispose(ref _d3dDeviceContext);
			Utilities.Dispose(ref _d3dDevice);

			_renderForm?.Dispose();
		}
	}
}
