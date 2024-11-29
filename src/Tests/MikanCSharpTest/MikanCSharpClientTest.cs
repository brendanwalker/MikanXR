using SharpDX;
using SharpDX.D3DCompiler;
using SharpDX.Direct3D;
using SharpDX.DXGI;
using SharpDX.Windows;
using D3D11 = SharpDX.Direct3D11;
using System;
using System.Drawing;
using MikanXR;
using System.Diagnostics;
using SharpDX.Direct3D11;
using System.Runtime.InteropServices;

namespace Mikan
{
	class RenderTarget : IDisposable
	{
		// Direct3D11 Color Target
		private D3D11.Texture2D _colorTargetTexture;
		private D3D11.RenderTargetView _colorTargetView;
		private D3D11.ShaderResourceView _colorTargetSRV;

		// Direct3D11 Depth Target
		private D3D11.Texture2D _floatDepthTargetTexture;
		private D3D11.DepthStencilView _floatDepthTargetView;
		private D3D11.ShaderResourceView _floatDepthTargetSRV;

		public D3D11.Texture2D ColorTexture => _colorTargetTexture;
		public D3D11.Texture2D DepthTexture => _floatDepthTargetTexture;

		public D3D11.RenderTargetView ColorTargetView => _colorTargetView;
		public D3D11.DepthStencilView DepthTargetView => _floatDepthTargetView;

		public D3D11.ShaderResourceView ColorTextureSRV => _colorTargetSRV;
		public D3D11.ShaderResourceView FloatDepthTextureSRV => _floatDepthTargetSRV;

		public bool IsInitialized => _colorTargetTexture != null && _floatDepthTargetTexture != null;

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

		public void Initialize(D3D11.Device d3dDevice, int newWidth, int newHeight)
		{
			// Create the render target resources
			// -------

			// Create the render target texture.
			_colorTargetTexture = new D3D11.Texture2D(
				d3dDevice,
				new D3D11.Texture2DDescription()
				{
					Width = newWidth,
					Height = newHeight,
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
				d3dDevice,
				_colorTargetTexture,
				new RenderTargetViewDescription()
				{
					Format = Format.B8G8R8A8_UNorm,
					Dimension = RenderTargetViewDimension.Texture2D,
					Texture2D = new RenderTargetViewDescription.Texture2DResource() { MipSlice = 0 }
				});

			// Create the shader resource view
			_colorTargetSRV = new D3D11.ShaderResourceView(
				d3dDevice,
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

			// Create the render target texture.
			_floatDepthTargetTexture = new D3D11.Texture2D(
				d3dDevice,
				new D3D11.Texture2DDescription()
				{
					Width = newWidth,
					Height = newHeight,
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
				d3dDevice,
				_floatDepthTargetTexture,
				new DepthStencilViewDescription()
				{
					Format = depthViewFormat,
					Dimension = DepthStencilViewDimension.Texture2D,
					Texture2D = new DepthStencilViewDescription.Texture2DResource() { MipSlice = 0 }
				});

			// Create the shader resource view
			_floatDepthTargetSRV = new D3D11.ShaderResourceView(
				d3dDevice,
				_floatDepthTargetTexture,
				new ShaderResourceViewDescription()
				{
					Format = srvformat,
					Dimension = ShaderResourceViewDimension.Texture2D,
					Texture2D = new ShaderResourceViewDescription.Texture2DResource() { MipLevels = 1, MostDetailedMip = 0 }
				});
		}

		public void Dispose()
		{
			Utilities.Dispose(ref _colorTargetSRV);
			Utilities.Dispose(ref _colorTargetView);
			Utilities.Dispose(ref _colorTargetTexture);

			Utilities.Dispose(ref _floatDepthTargetSRV);
			Utilities.Dispose(ref _floatDepthTargetView);
			Utilities.Dispose(ref _floatDepthTargetTexture);
		}

		public void Bind(D3D11.DeviceContext d3dDeviceContext)
		{
			// Set the output render views
			d3dDeviceContext.OutputMerger.SetTargets(_floatDepthTargetView, _colorTargetView);

			// Set the viewport dimensions
			d3dDeviceContext.Rasterizer.SetViewport(
				new Viewport(
					0, 0, 
					_colorTargetTexture.Description.Width, _colorTargetTexture.Description.Height)); // min z, max z

			// Clear the screen
			d3dDeviceContext.ClearRenderTargetView(_colorTargetView, new SharpDX.Color(0, 0, 0, 0));
			d3dDeviceContext.ClearDepthStencilView(_floatDepthTargetView, DepthStencilClearFlags.Depth, 1.0f, 0);
		}
	}

	public class MikanCSharpDXTestApp : IDisposable
	{
		// Window
		private RenderForm renderForm;
		private const int windowWidth = 1280;
		private const int windowHeight = 720;

		// Direct3D11 SwapChain 
		private D3D11.Device d3dDevice;
		private D3D11.DeviceContext d3dDeviceContext;
		private SwapChain swapChain;
		private D3D11.Texture2D defaultColorBuffer;
		private D3D11.RenderTargetView defaultColorTargetView;
		private D3D11.Texture2D defaultDepthBuffer;
		private D3D11.DepthStencilView defaultFloatDepthView;

		// Render Targets
		private RenderTarget renderTarget;
		private RenderTarget depthDebugRenderTarget;

		// Cube Shader Data
		private D3D11.VertexShader cubeVertexShader;
		private D3D11.PixelShader cubePixelShader;
		private ShaderSignature cubeInputSignature;
		private D3D11.Buffer cubeVertices;
		private D3D11.InputLayout cubeInputLayout;
		private int cubeVertexCount;
		private D3D11.Buffer cubeShaderContantBuffer;
		private Vector3 cameraPosition;
		private Vector3 cameraForward;
		private Vector3 cameraUp;
		private Vector3 cameraRight;
		private Matrix viewMatrix;
		private Matrix projectionMatrix;

		// Depth Normalize Shader Data
		private D3D11.VertexShader depthNormalizeVertexShader;
		private D3D11.PixelShader depthNormalizePixelShader;
		private D3D11.Buffer depthNormalizeContantBuffer;
		private DepthNormalizeConstantBuffer depthNormalConstants;

		// Pack Depth Texture SRV
		private IntPtr packDepthTextureResourcePtr;
		private D3D11.ShaderResourceView packDepthTargetSRV;

		// Full Screen Texture Shader Data
		private D3D11.VertexShader quadVertexShader;
		private D3D11.PixelShader quadPixelShader;
		private D3D11.SamplerState quadTextureSamplerState;
		private D3D11.Buffer quadVertices;
		private ShaderSignature quadInputSignature;
		private D3D11.InputLayout quadInputLayout;

		// Mikan
		private MikanAPI mikanAPI;
		private float mikanReconnectTimeout = 0f;
		private Stopwatch clock;
		private long lastUpdateTimestamp= 0;
		private float time = 0f;
		private UInt64 lastReceivedVideoSourceFrame = 0;

		public Vector3 cubeOffset = new Vector3(0, 0, 10);

		public MikanCSharpDXTestApp()
		{
			clock = new Stopwatch();
			mikanAPI = new MikanAPI();

			renderTarget = new RenderTarget();
			depthDebugRenderTarget = new RenderTarget();

			// Set window properties
			renderForm = new RenderForm("Mikan C# D3D11 Test");
			renderForm.ClientSize = new Size(windowWidth, windowHeight);
			renderForm.AllowUserResizing = false;
			renderForm.KeyPress += (sender, e) => { 
				switch (e.KeyChar)
				{
				case '1':
					DrawDepthMode = RenderMode.Color;

					break;
				case '2':
					DrawDepthMode = RenderMode.DepthNormalize;
					break;
				case '3':
					DrawDepthMode = RenderMode.PackDepth;
					break;
				case 'w':
					cubeOffset.Z += 0.1f;
					break;
				case 's':
					cubeOffset.Z -= 0.1f;
					break;
				case 'a':
					cubeOffset.X -= 0.1f;
					break;
				case 'd':
					cubeOffset.X += 0.1f;
					break;
				case 'q':
					cubeOffset.Y += 0.1f;
					break;
				case 'e':
					cubeOffset.Y -= 0.1f;
					break;
				}
			};
		}

		public bool Initialize()
		{
			bool bSuccess;

			try
			{
				InitSwapChain(windowWidth, windowHeight);

				bSuccess= 
					InitializeCubeShader() && 
					InitializeDepthNormalizeShader() &&
					InitializeQuadTextureShader();
				if (bSuccess)
				{
					InitializeCubeGeometry();
					InitializeQuadGeometry();

					bSuccess= InitializeMikan();
				}
			}
			catch (SharpDXException e)
			{
				Log(MikanLogLevel.Fatal, e.Message);
				bSuccess= false;
			}

			return bSuccess;
		}

		public void Dispose()
		{
			Utilities.Dispose(ref mikanAPI);

			Utilities.Dispose(ref cubeInputSignature);
			Utilities.Dispose(ref cubeVertexShader);
			Utilities.Dispose(ref cubePixelShader);
			Utilities.Dispose(ref cubeInputSignature);
			Utilities.Dispose(ref cubeInputLayout);
			Utilities.Dispose(ref cubeVertices);

			Utilities.Dispose(ref depthNormalizeVertexShader);
			Utilities.Dispose(ref depthNormalizePixelShader);

			Utilities.Dispose(ref quadPixelShader);
			Utilities.Dispose(ref quadVertexShader);
			Utilities.Dispose(ref quadInputSignature);
			Utilities.Dispose(ref quadInputLayout);
			Utilities.Dispose(ref quadVertices);

			Utilities.Dispose(ref defaultColorBuffer);
			Utilities.Dispose(ref defaultColorTargetView);
			Utilities.Dispose(ref defaultDepthBuffer);
			Utilities.Dispose(ref defaultFloatDepthView);

			Utilities.Dispose(ref renderTarget);
			Utilities.Dispose(ref depthDebugRenderTarget);

			Utilities.Dispose(ref swapChain);
			Utilities.Dispose(ref d3dDevice);
			Utilities.Dispose(ref d3dDeviceContext);
			Utilities.Dispose(ref renderForm);
		}

		private void InitSwapChain(int textureWidth, int textureHeight)
		{
			// Descriptor for the swap chain
			SwapChainDescription swapChainDesc = new SwapChainDescription()
			{
				BufferCount = 1,
				ModeDescription = 
					new ModeDescription(windowWidth, windowHeight, 
						new Rational(60, 1), Format.B8G8R8A8_UNorm),
				IsWindowed = true,
				OutputHandle = renderForm.Handle,
				SampleDescription = new SampleDescription(1, 0),
				SwapEffect = SwapEffect.Discard,
				Usage = Usage.RenderTargetOutput,
			};

			// Create device and swap chain
			D3D11.Device.CreateWithSwapChain(
				DriverType.Hardware, 
				D3D11.DeviceCreationFlags.None, 
				swapChainDesc, 
				out d3dDevice, 
				out swapChain);
			d3dDeviceContext = d3dDevice?.ImmediateContext ?? null;

			// Create render target view for back buffer
			ResizeSwapChainRenderBuffers(windowWidth, windowHeight);
		}

		private void ResizeSwapChainRenderBuffers(int newWidth, int newHeight)
		{
			Utilities.Dispose(ref defaultColorBuffer);
			Utilities.Dispose(ref defaultColorTargetView);
			Utilities.Dispose(ref defaultDepthBuffer);
			Utilities.Dispose(ref defaultFloatDepthView);

			if (swapChain != null)
			{
				// Resize the back buffer
				swapChain.ResizeBuffers(1, newWidth, newHeight, Format.B8G8R8A8_UNorm, SwapChainFlags.None);

				// Get the back buffer from the swap chain
				defaultColorBuffer = swapChain.GetBackBuffer<D3D11.Texture2D>(0);

				// Create a render target view for the back buffer
				defaultColorTargetView = new D3D11.RenderTargetView(d3dDevice, defaultColorBuffer);

				// Create the depth buffer
				defaultDepthBuffer = new D3D11.Texture2D(d3dDevice, new D3D11.Texture2DDescription()
				{
					Format = Format.D32_Float_S8X24_UInt,
					ArraySize = 1,
					MipLevels = 1,
					Width = newWidth,
					Height = newHeight,
					SampleDescription = new SampleDescription(1, 0),
					Usage = D3D11.ResourceUsage.Default,
					BindFlags = D3D11.BindFlags.DepthStencil,
					CpuAccessFlags = D3D11.CpuAccessFlags.None,
					OptionFlags = D3D11.ResourceOptionFlags.None
				});

				// Create the depth buffer view
				defaultFloatDepthView = new D3D11.DepthStencilView(d3dDevice, defaultDepthBuffer);

				// Setup the targets and viewport for rendering
				depthNormalConstants = new DepthNormalizeConstantBuffer() { zNear= 0f, zFar= 1f };
				d3dDeviceContext.Rasterizer.SetViewport(
					new Viewport(0, 0, windowWidth, windowHeight, 0.0f, 1.0f));
				d3dDeviceContext.OutputMerger.SetTargets(defaultFloatDepthView, defaultColorTargetView);
			}
		}

		private void FreeFrameBuffer()
		{
			renderTarget.Dispose();
			depthDebugRenderTarget.Dispose();
		}

		private void CreateFrameBuffer(int newWidth, int newHeight)
		{
			renderTarget.Initialize(d3dDevice, newWidth, newHeight);
			depthDebugRenderTarget.Initialize(d3dDevice, newWidth, newHeight);
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

			// Compile the vertex shader code
			var vertexShaderByteCode = ShaderBytecode.Compile(shaderCodeString, "vs_main", "vs_4_0", ShaderFlags.Debug);
			if (vertexShaderByteCode.HasErrors)
			{
				Log(MikanLogLevel.Fatal, vertexShaderByteCode.Message);
				return false;
			}
			cubeVertexShader = new D3D11.VertexShader(d3dDevice, vertexShaderByteCode);

			// Compile the pixel shader code
			var pixelShaderByteCode = ShaderBytecode.Compile(shaderCodeString, "ps_main", "ps_4_0", ShaderFlags.Debug);
			if (pixelShaderByteCode.HasErrors)
			{
				Log(MikanLogLevel.Fatal, pixelShaderByteCode.Message);
				return false;
			}
			cubePixelShader = new D3D11.PixelShader(d3dDevice, pixelShaderByteCode);

			// Read input signature from shader code
			cubeInputSignature = ShaderSignature.GetInputSignature(vertexShaderByteCode);

			// Create Constant Buffer
			cubeShaderContantBuffer = new D3D11.Buffer(
				d3dDevice, 
				Utilities.SizeOf<Matrix>(), 
				ResourceUsage.Default, 
				BindFlags.ConstantBuffer, 
				CpuAccessFlags.None, 
				ResourceOptionFlags.None, 
				0);

			// Setup default camera and projection matrices
			depthNormalConstants.zNear= 0.1f;
			depthNormalConstants.zFar= 100.0f;
			projectionMatrix = 
				Matrix.PerspectiveFovLH(
					MathUtil.PiOverFour, windowWidth / (float)windowHeight, 
					depthNormalConstants.zNear, 
					depthNormalConstants.zFar);

			// Compute the initial view matrix
			SetCameraPose(new Vector3(0, 0, 1), new Vector3(0, 1, 0), new Vector3(0, 0, -10));

			return true;
		}

		[StructLayout(LayoutKind.Sequential)]
		internal struct DepthNormalizeConstantBuffer
		{
			public float zNear;
			public float zFar;
			public float padding0;
			public float padding1;
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

			// Compile the vertex shader code
			var vertexShaderByteCode = ShaderBytecode.Compile(shaderCodeString, "vs_main", "vs_4_0", ShaderFlags.Debug);
			if (vertexShaderByteCode.HasErrors)
			{
				Log(MikanLogLevel.Fatal, vertexShaderByteCode.Message);
				return false;
			}
			depthNormalizeVertexShader = new D3D11.VertexShader(d3dDevice, vertexShaderByteCode);

			// Compile the pixel shader code
			var pixelShaderByteCode = ShaderBytecode.Compile(shaderCodeString, "ps_main", "ps_4_0", ShaderFlags.Debug);
			if (pixelShaderByteCode.HasErrors)
			{
				Log(MikanLogLevel.Fatal, pixelShaderByteCode.Message);
				return false;
			}
			depthNormalizePixelShader = new D3D11.PixelShader(d3dDevice, pixelShaderByteCode);

			// Create Constant Buffer
			depthNormalizeContantBuffer = new D3D11.Buffer(
				d3dDevice,
				Utilities.SizeOf<DepthNormalizeConstantBuffer>(),
				ResourceUsage.Dynamic,
				BindFlags.ConstantBuffer,
				CpuAccessFlags.Write,
				ResourceOptionFlags.None,
				0);

			return true;
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

			// Compile the vertex shader code
			var vertexShaderByteCode = ShaderBytecode.Compile(shaderCodeString, "vs_main", "vs_4_0", ShaderFlags.Debug);
			if (vertexShaderByteCode.HasErrors)
			{
				Log(MikanLogLevel.Fatal, vertexShaderByteCode.Message);
				return false;
			}
			quadVertexShader = new D3D11.VertexShader(d3dDevice, vertexShaderByteCode);

			// Compile the pixel shader code
			var pixelShaderByteCode = ShaderBytecode.Compile(shaderCodeString, "ps_main", "ps_4_0", ShaderFlags.Debug);
			if (pixelShaderByteCode.HasErrors)
			{
				Log(MikanLogLevel.Fatal, pixelShaderByteCode.Message);
				return false;
			}
			quadPixelShader = new D3D11.PixelShader(d3dDevice, pixelShaderByteCode);

			// Read input signature from shader code
			quadInputSignature = ShaderSignature.GetInputSignature(vertexShaderByteCode);

			quadTextureSamplerState = new D3D11.SamplerState(d3dDevice, new D3D11.SamplerStateDescription()
			{
				Filter = Filter.MinMagMipLinear,
				AddressU = TextureAddressMode.Clamp,
				AddressV = TextureAddressMode.Clamp,
				AddressW = TextureAddressMode.Clamp,
				BorderColor = SharpDX.Color.Black,
				ComparisonFunction = Comparison.Never,
				MaximumAnisotropy = 1,
				MipLodBias = 0,
				MinimumLod = 0,
				MaximumLod = float.MaxValue
			});

			return true;
		}

		private void InitializeCubeGeometry()
		{
			// Create the input layout from the input signature and the input elements
			cubeInputLayout = new InputLayout(d3dDevice, cubeInputSignature, new[]
				{
					new InputElement("POSITION", 0, Format.R32G32B32A32_Float, 0, 0),
					new InputElement("COLOR", 0, Format.R32G32B32A32_Float, 16, 0)
				});

			// Instantiate Vertex buffer from vertex data
			cubeVertices = D3D11.Buffer.Create(d3dDevice, BindFlags.VertexBuffer, new[]
			{
				new Vector4(-1.0f, -1.0f, -1.0f, 1.0f), new Vector4(1.0f, 0.0f, 0.0f, 1.0f), // Front
                new Vector4(-1.0f,  1.0f, -1.0f, 1.0f), new Vector4(1.0f, 0.0f, 0.0f, 1.0f),
				new Vector4( 1.0f,  1.0f, -1.0f, 1.0f), new Vector4(1.0f, 0.0f, 0.0f, 1.0f),
				new Vector4(-1.0f, -1.0f, -1.0f, 1.0f), new Vector4(1.0f, 0.0f, 0.0f, 1.0f),
				new Vector4( 1.0f,  1.0f, -1.0f, 1.0f), new Vector4(1.0f, 0.0f, 0.0f, 1.0f),
				new Vector4( 1.0f, -1.0f, -1.0f, 1.0f), new Vector4(1.0f, 0.0f, 0.0f, 1.0f),

				new Vector4(-1.0f, -1.0f,  1.0f, 1.0f), new Vector4(0.0f, 1.0f, 0.0f, 1.0f), // BACK
                new Vector4( 1.0f,  1.0f,  1.0f, 1.0f), new Vector4(0.0f, 1.0f, 0.0f, 1.0f),
				new Vector4(-1.0f,  1.0f,  1.0f, 1.0f), new Vector4(0.0f, 1.0f, 0.0f, 1.0f),
				new Vector4(-1.0f, -1.0f,  1.0f, 1.0f), new Vector4(0.0f, 1.0f, 0.0f, 1.0f),
				new Vector4( 1.0f, -1.0f,  1.0f, 1.0f), new Vector4(0.0f, 1.0f, 0.0f, 1.0f),
				new Vector4( 1.0f,  1.0f,  1.0f, 1.0f), new Vector4(0.0f, 1.0f, 0.0f, 1.0f),

				new Vector4(-1.0f, 1.0f, -1.0f,  1.0f), new Vector4(0.0f, 0.0f, 1.0f, 1.0f), // Top
                new Vector4(-1.0f, 1.0f,  1.0f,  1.0f), new Vector4(0.0f, 0.0f, 1.0f, 1.0f),
				new Vector4( 1.0f, 1.0f,  1.0f,  1.0f), new Vector4(0.0f, 0.0f, 1.0f, 1.0f),
				new Vector4(-1.0f, 1.0f, -1.0f,  1.0f), new Vector4(0.0f, 0.0f, 1.0f, 1.0f),
				new Vector4( 1.0f, 1.0f,  1.0f,  1.0f), new Vector4(0.0f, 0.0f, 1.0f, 1.0f),
				new Vector4( 1.0f, 1.0f, -1.0f,  1.0f), new Vector4(0.0f, 0.0f, 1.0f, 1.0f),

				new Vector4(-1.0f,-1.0f, -1.0f,  1.0f), new Vector4(1.0f, 1.0f, 0.0f, 1.0f), // Bottom
                new Vector4( 1.0f,-1.0f,  1.0f,  1.0f), new Vector4(1.0f, 1.0f, 0.0f, 1.0f),
				new Vector4(-1.0f,-1.0f,  1.0f,  1.0f), new Vector4(1.0f, 1.0f, 0.0f, 1.0f),
				new Vector4(-1.0f,-1.0f, -1.0f,  1.0f), new Vector4(1.0f, 1.0f, 0.0f, 1.0f),
				new Vector4( 1.0f,-1.0f, -1.0f,  1.0f), new Vector4(1.0f, 1.0f, 0.0f, 1.0f),
				new Vector4( 1.0f,-1.0f,  1.0f,  1.0f), new Vector4(1.0f, 1.0f, 0.0f, 1.0f),

				new Vector4(-1.0f, -1.0f, -1.0f, 1.0f), new Vector4(1.0f, 0.0f, 1.0f, 1.0f), // Left
                new Vector4(-1.0f, -1.0f,  1.0f, 1.0f), new Vector4(1.0f, 0.0f, 1.0f, 1.0f),
				new Vector4(-1.0f,  1.0f,  1.0f, 1.0f), new Vector4(1.0f, 0.0f, 1.0f, 1.0f),
				new Vector4(-1.0f, -1.0f, -1.0f, 1.0f), new Vector4(1.0f, 0.0f, 1.0f, 1.0f),
				new Vector4(-1.0f,  1.0f,  1.0f, 1.0f), new Vector4(1.0f, 0.0f, 1.0f, 1.0f),
				new Vector4(-1.0f,  1.0f, -1.0f, 1.0f), new Vector4(1.0f, 0.0f, 1.0f, 1.0f),

				new Vector4( 1.0f, -1.0f, -1.0f, 1.0f), new Vector4(0.0f, 1.0f, 1.0f, 1.0f), // Right
                new Vector4( 1.0f,  1.0f,  1.0f, 1.0f), new Vector4(0.0f, 1.0f, 1.0f, 1.0f),
				new Vector4( 1.0f, -1.0f,  1.0f, 1.0f), new Vector4(0.0f, 1.0f, 1.0f, 1.0f),
				new Vector4( 1.0f, -1.0f, -1.0f, 1.0f), new Vector4(0.0f, 1.0f, 1.0f, 1.0f),
				new Vector4( 1.0f,  1.0f, -1.0f, 1.0f), new Vector4(0.0f, 1.0f, 1.0f, 1.0f),
				new Vector4( 1.0f,  1.0f,  1.0f, 1.0f), new Vector4(0.0f, 1.0f, 1.0f, 1.0f),
			});
			cubeVertexCount= 36;
		}

		[StructLayout(LayoutKind.Sequential)]
		internal struct QuadVertex
		{
			public Vector3 position;
			public Vector2 texture;
		}
		static int kQuadVertexSize= Marshal.SizeOf(typeof(QuadVertex));

		private void InitializeQuadGeometry()
		{
			// Create the input layout from the input signature and the input elements
			quadInputLayout = new InputLayout(d3dDevice, quadInputSignature, new[]
				{
					new InputElement("POSITION", 0, Format.R32G32B32_Float, 0, 0),
					new InputElement("TEXCOORD", 0, Format.R32G32_Float, 12, 0)
				});

			// Instantiate Vertex buffer from vertex data
			quadVertices = D3D11.Buffer.Create(d3dDevice, BindFlags.VertexBuffer, new[]
			{
				new QuadVertex() { position = new Vector3( 1.0f, -1.0f, 0.0f), texture = new Vector2(1.0f, 1.0f) },
				new QuadVertex() { position = new Vector3(-1.0f, -1.0f, 0.0f), texture = new Vector2(0.0f, 1.0f) },
				new QuadVertex() { position = new Vector3(-1.0f,  1.0f, 0.0f), texture = new Vector2(0.0f, 0.0f) },

				new QuadVertex() { position = new Vector3( 1.0f,  1.0f, 0.0f), texture = new Vector2(1.0f, 0.0f) },
				new QuadVertex() { position = new Vector3( 1.0f, -1.0f, 0.0f), texture = new Vector2(1.0f, 1.0f) },
				new QuadVertex() { position = new Vector3(-1.0f,  1.0f, 0.0f), texture = new Vector2(0.0f, 0.0f) },
			});
		}

		private	bool InitializeMikan()
		{
			mikanAPI.SetLogCallback(this.Log);

			if (mikanAPI.Initialize(MikanLogLevel.Info) != MikanAPIResult.Success)
			{
				return false;
			}

			if (mikanAPI.SetGraphicsDeviceInterface(
					MikanClientGraphicsApi.Direct3D11, 
					d3dDevice.NativePointer) != MikanAPIResult.Success)
			{
				return false;
			}

			return true;
		}

		public void Run()
		{
			clock.Start();

			// Start render loop
			// Does not return until close event is received
			RenderLoop.Run(renderForm, UpdateLoop);

			clock.Stop();
		}

		private async void UpdateLoop()
		{
			long now = clock.ElapsedMilliseconds;
			float deltaMilliseconds = now - lastUpdateTimestamp;
			float deltaSecondsClamped = Math.Min(deltaMilliseconds / 1000.0f, .1f);

			// Update timekeeping
			time += deltaSecondsClamped;
			lastUpdateTimestamp = now;

			if (mikanAPI.GetIsConnected())
			{
				while (mikanAPI.FetchNextEvent(out MikanEvent nextEvent) == MikanAPIResult.Success)
				{
					if (nextEvent is MikanConnectedEvent)
					{
						var clientInfo = mikanAPI.AllocateClientInfo();
						clientInfo.engineName = "MikanXR Test";
						clientInfo.engineVersion = "1.0.0";
						clientInfo.applicationName = "MikanXR C# Test App";
						clientInfo.applicationVersion = "1.0.0";
						clientInfo.graphicsAPI = MikanClientGraphicsApi.Direct3D11;
						clientInfo.supportsRGBA32 = true;
						clientInfo.supportsDepth = true;

						var initClientRequest = new InitClientRequest()
						{
							clientInfo = clientInfo
						};

						await mikanAPI.SendRequest(initClientRequest);

						ReallocateRenderBuffers();
						UpdateCameraProjectionMatrix();
					}
					else if (nextEvent is MikanVideoSourceOpenedEvent)
					{
						ReallocateRenderBuffers();
						UpdateCameraProjectionMatrix();
					}
					else if (nextEvent is MikanVideoSourceNewFrameEvent)
					{
						var newFrameEvent = nextEvent as MikanVideoSourceNewFrameEvent;

						ProcessNewVideoSourceFrame(newFrameEvent);
					}
					else if (nextEvent is MikanVideoSourceModeChangedEvent ||
							nextEvent is MikanVideoSourceIntrinsicsChangedEvent)
					{
						ReallocateRenderBuffers();
						UpdateCameraProjectionMatrix();
					}
				}
			}
			else
			{
				if (mikanReconnectTimeout <= 0f)
				{
					if (mikanAPI.Connect() != MikanAPIResult.Success || !mikanAPI.GetIsConnected())
					{
						// Timeout before trying to reconnect
						mikanReconnectTimeout = 1f;
					}
				}
				else
				{
					mikanReconnectTimeout -= deltaSecondsClamped;
				}

				// Just render the scene using the last applied camera pose
				Draw();
			}
		}

		private void Log(MikanLogLevel logLevel, string logMessage)
		{
			switch (logLevel)
			{
			case MikanLogLevel.Debug:
				Console.WriteLine($"DEBUG: {logMessage}");
				break;
			case MikanLogLevel.Info:
				Console.WriteLine($"INFO: {logMessage}");
				break;
			case MikanLogLevel.Warning:
				Console.WriteLine($"WARNING: {logMessage}");
				break;
			case MikanLogLevel.Error:
				Console.WriteLine($"ERROR: {logMessage}");
				break;
			case MikanLogLevel.Fatal:
				Console.WriteLine($"FATAL: {logMessage}");
				break;
			default:
				Console.WriteLine($"UNKNOWN: {logMessage}");
				break;
			}
		}

		async void ReallocateRenderBuffers()
		{
			FreeFrameBuffer();

			await mikanAPI.SendRequest(new FreeRenderTargetTextures());

			MikanResponse response = await mikanAPI.SendRequest(new GetVideoSourceMode());
			if (response.resultCode == MikanAPIResult.Success)
			{
				var mode = response as MikanVideoSourceModeResponse;

				var desc = new MikanRenderTargetDescriptor()
				{
					width = (uint)mode.resolution_x,
					height = (uint)mode.resolution_y,
					color_buffer_type = MikanColorBufferType.BGRA32,
					depth_buffer_type = MikanDepthBufferType.FLOAT_DEVICE_DEPTH,
					graphicsAPI = MikanClientGraphicsApi.Direct3D11
				};

				// Tell the server to allocate new render target buffers
				await mikanAPI.SendRequest(new AllocateRenderTargetTextures() { descriptor = desc });

				// Create a new frame buffer to render to
				CreateFrameBuffer(mode.resolution_x, mode.resolution_y);
			}
		}

		async void UpdateCameraProjectionMatrix()
		{
			var response = await mikanAPI.SendRequest(new GetVideoSourceIntrinsics());
			if (response.resultCode == MikanAPIResult.Success)
			{
				var videoSourceIntrinsics = response as MikanVideoSourceIntrinsicsResponse;
				var intrinsics = videoSourceIntrinsics.intrinsics.intrinsics_ptr;

				var monoIntrinsics = intrinsics.Instance as MikanMonoIntrinsics;
				float videoSourcePixelWidth = (float)monoIntrinsics.pixel_width;
				float videoSourcePixelHeight = (float)monoIntrinsics.pixel_height;

				depthNormalConstants.zNear = (float)monoIntrinsics.znear;
				depthNormalConstants.zFar = (float)monoIntrinsics.zfar;
				projectionMatrix = Matrix.PerspectiveFovLH(
					MathUtil.DegreesToRadians((float)monoIntrinsics.vfov),
					videoSourcePixelWidth / videoSourcePixelHeight,
					depthNormalConstants.zNear,
					depthNormalConstants.zFar);
			}
		}

		Vector3 MikanVector3fToSharpDXVector3(MikanVector3f v)
		{
			return new Vector3(v.x, v.y, -v.z);
		}

		void ProcessNewVideoSourceFrame(MikanVideoSourceNewFrameEvent newFrameEvent)
		{
			if (newFrameEvent.frame == lastReceivedVideoSourceFrame)
				return;

			// Apply the camera pose received
			SetCameraPose(
				newFrameEvent.cameraForward,
				newFrameEvent.cameraUp,
				newFrameEvent.cameraPosition);

			// Render out a new frame
			Draw();

			// Publish the updated render target to Mikan
			if (renderTarget.ColorTexture != null)
			{			
				mikanAPI.SendRequest(new WriteColorRenderTargetTexture() {
					apiColorTexturePtr = renderTarget.ColorTexture.NativePointer 
				});
				mikanAPI.SendRequest(new WriteDepthRenderTargetTexture() {
					apiDepthTexturePtr= renderTarget.DepthTexture.NativePointer,
					zNear= depthNormalConstants.zNear,
					zFar = depthNormalConstants.zFar
				});
				mikanAPI.SendRequest(new PublishRenderTargetTextures()
				{
					frameIndex= newFrameEvent.frame,
				});

				// Update the pack depth texture resource pointer, if any
				IntPtr packDepthPtr= mikanAPI.GetPackDepthTextureResourcePtr();
				if (packDepthPtr != packDepthTextureResourcePtr)
				{
					packDepthTextureResourcePtr= packDepthPtr;
					packDepthTargetSRV= new D3D11.ShaderResourceView(packDepthTextureResourcePtr);
				}
			}

			// Remember the frame index of the last frame we published
			lastReceivedVideoSourceFrame = newFrameEvent.frame;
		}

		void SetCameraPose(
			MikanVector3f mikanForward,
			MikanVector3f mikanUp,
			MikanVector3f mikanPosition)
		{
			var newCamForward = MikanVector3fToSharpDXVector3(mikanForward);
			var newCamUp = MikanVector3fToSharpDXVector3(mikanUp);
			var newCamPos = MikanVector3fToSharpDXVector3(mikanPosition);

			SetCameraPose(newCamForward, newCamUp, newCamPos);
		}

		void SetCameraPose(
			Vector3 inCameraForward,
			Vector3 inCameraUp,
			Vector3 inCameraPosition)
		{
			cameraForward = inCameraForward;
			cameraUp = inCameraUp;
			cameraRight = Vector3.Cross(inCameraUp, inCameraForward);
			cameraRight.Normalize();

			cameraPosition = inCameraPosition;
			var targetPosition = cameraPosition + cameraForward;

			// Apply the camera pose received
			Matrix.LookAtLH(
				ref cameraPosition,
				ref targetPosition,
				ref cameraUp,
				out viewMatrix);
		}

		private void RenderCubeToTarget(
			D3D11.RenderTargetView inColorTargetView,
			D3D11.DepthStencilView inDepthTargetView)
		{
			// Set the output render views
			d3dDeviceContext.OutputMerger.SetTargets(inDepthTargetView, inColorTargetView);

			// Clear the screen
			d3dDeviceContext.ClearRenderTargetView(inColorTargetView, new SharpDX.Color(32, 103, 178, 0));
			d3dDeviceContext.ClearDepthStencilView(inDepthTargetView, DepthStencilClearFlags.Depth, 1.0f, 0);

			// Assign the cube vertices
			d3dDeviceContext.InputAssembler.SetVertexBuffers(
				0,
				new VertexBufferBinding(cubeVertices, Utilities.SizeOf<Vector4>() * 2, 0));

			// Assign the cube shader
			d3dDeviceContext.InputAssembler.InputLayout = cubeInputLayout;
			d3dDeviceContext.VertexShader.Set(cubeVertexShader);
			d3dDeviceContext.PixelShader.Set(cubePixelShader);

			// Update the model view projection matrix used to render the cube geometry
			var viewProj = Matrix.Multiply(viewMatrix, projectionMatrix);
			var cubePostion = 
				cameraPosition + 
				cameraForward*cubeOffset.Z +
				cameraUp*cubeOffset.Y +
				cameraRight*cubeOffset.X;
			var cubeModelViewProj =
				Matrix.Scaling(0.1f) *
				Matrix.RotationX(time) *
				Matrix.RotationY(time * 2) *
				Matrix.RotationZ(time * .7f) *
				Matrix.Translation(cubePostion) *
				viewProj;
			cubeModelViewProj.Transpose();
			d3dDeviceContext.UpdateSubresource(ref cubeModelViewProj, cubeShaderContantBuffer);

			// Set the constants buffer
			d3dDeviceContext.VertexShader.SetConstantBuffer(0, cubeShaderContantBuffer);

			// Draw the cube
			d3dDeviceContext.InputAssembler.PrimitiveTopology = PrimitiveTopology.TriangleList;
			d3dDeviceContext.Draw(cubeVertexCount, 0);
		}

		private void RenderNormalizedDepthTexture()
		{
			// Set the output render views
			depthDebugRenderTarget.Bind(d3dDeviceContext);

			// Assign the quad vertices
			d3dDeviceContext.InputAssembler.SetVertexBuffers(
				0,
				new VertexBufferBinding(quadVertices, kQuadVertexSize, 0));

			// Assign the depth shader
			d3dDeviceContext.InputAssembler.InputLayout = quadInputLayout;
			d3dDeviceContext.VertexShader.Set(depthNormalizeVertexShader);
			d3dDeviceContext.PixelShader.Set(depthNormalizePixelShader);

			// Set the texture
			d3dDeviceContext.PixelShader.SetShaderResource(0, renderTarget.FloatDepthTextureSRV);
			d3dDeviceContext.PixelShader.SetSampler(0, quadTextureSamplerState);

			// Update the depth normalization constants
			d3dDeviceContext.MapSubresource(depthNormalizeContantBuffer, MapMode.WriteDiscard, D3D11.MapFlags.None, out DataStream mappedResource);
			mappedResource.Write(depthNormalConstants);
			d3dDeviceContext.UnmapSubresource(depthNormalizeContantBuffer, 0);
			d3dDeviceContext.PixelShader.SetConstantBuffer(0, depthNormalizeContantBuffer);

			// Draw the cube
			d3dDeviceContext.InputAssembler.PrimitiveTopology = PrimitiveTopology.TriangleList;
			d3dDeviceContext.Draw(6, 0);
		}

		private void RenderColorTexture(ShaderResourceView textureSRV)
		{
			// Set the output render views
			d3dDeviceContext.OutputMerger.SetTargets(defaultFloatDepthView, defaultColorTargetView);

			// Clear the screen
			d3dDeviceContext.ClearRenderTargetView(defaultColorTargetView, new SharpDX.Color(0, 0, 0, 0));
			d3dDeviceContext.ClearDepthStencilView(defaultFloatDepthView, DepthStencilClearFlags.Depth, 1.0f, 0);

			// Assign the quad vertices
			d3dDeviceContext.InputAssembler.SetVertexBuffers(
				0,
				new VertexBufferBinding(quadVertices, kQuadVertexSize, 0));

			// Assign the quad shader
			d3dDeviceContext.InputAssembler.InputLayout = quadInputLayout;
			d3dDeviceContext.VertexShader.Set(quadVertexShader);
			d3dDeviceContext.PixelShader.Set(quadPixelShader);

			// Set the texture
			d3dDeviceContext.PixelShader.SetShaderResource(0, textureSRV);
			d3dDeviceContext.PixelShader.SetSampler(0, quadTextureSamplerState);

			// Draw the cube
			d3dDeviceContext.InputAssembler.PrimitiveTopology = PrimitiveTopology.TriangleList;
			d3dDeviceContext.Draw(6, 0);
		}

		enum RenderMode
		{
			Color,
			DepthNormalize,
			PackDepth
		}
		static RenderMode DrawDepthMode = RenderMode.Color;

		private void Draw()
		{
			if (renderTarget.IsInitialized && depthDebugRenderTarget.IsInitialized)
			{
				// Draw the cube to the RGBA color and float depth textures
				RenderCubeToTarget(renderTarget.ColorTargetView, renderTarget.DepthTargetView);

				// Render the depth RGBA packed texture
				RenderNormalizedDepthTexture();

				// Set the viewport dimensions
				d3dDeviceContext.Rasterizer.SetViewport(new Viewport(0, 0, windowWidth, windowHeight));

				// Render a result texture to the screen
				switch (DrawDepthMode)
				{
				case RenderMode.Color:
					RenderColorTexture(renderTarget.ColorTextureSRV);
					break;
				case RenderMode.DepthNormalize:
					RenderColorTexture(depthDebugRenderTarget.ColorTextureSRV);
					break;
				case RenderMode.PackDepth:
					if (packDepthTargetSRV != null)
					{
						RenderColorTexture(packDepthTargetSRV);
					}
					break;
				}
			}
			else
			{
				// Draw the cube to the default render target
				RenderCubeToTarget(defaultColorTargetView, defaultFloatDepthView);
			}

			// Swap front and back buffer
			swapChain.Present(1, PresentFlags.None);
		}

		[STAThread]
		static void Main(string[] args)
		{
			using (var app = new MikanCSharpDXTestApp())
			{
				if (app.Initialize())
				{
					app.Run();
				}
			}
		}
	}
}