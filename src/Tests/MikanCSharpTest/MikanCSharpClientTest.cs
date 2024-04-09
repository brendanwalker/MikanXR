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

namespace Mikan
{
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
		private D3D11.RenderTargetView defaultRenderTargetView;
		private D3D11.Texture2D defaultBackBuffer;
		private D3D11.Texture2D defaultDepthBuffer;
		private D3D11.DepthStencilView defaultDepthView;

		// Direct3D11 Render Target
		private D3D11.Texture2D renderTargetTexture;
		private D3D11.RenderTargetView renderTargetView;
		private D3D11.ShaderResourceView renderTargetSRV;

		// Direct3D11 Depth Target
		private D3D11.Texture2D depthTargetTexture;
		private D3D11.DepthStencilView depthTargetView;
		private D3D11.ShaderResourceView depthTargetSRV;

		// Cube Shader Data
		private D3D11.VertexShader cubeVertexShader;
		private D3D11.PixelShader cubePixelShader;
		private ShaderSignature inputSignature;
		private D3D11.Buffer vertices;
		private int vertexCount;
		private D3D11.Buffer cubeShaderContantBuffer;
		private Matrix viewMatrix;
		private Matrix projectionMatrix;

		// Depth Pack Shader Data
		private D3D11.VertexShader depthPackVertexShader;
		private D3D11.PixelShader depthPackPixelShader;

		// Mikan
		private MikanAPI mikanAPI;
		private float mikanReconnectTimeout = 0f;
		private Stopwatch clock;
		private long lastUpdateTimestamp= 0;
		private float time = 0f;
		private UInt64 lastReceivedVideoSourceFrame = 0;


		public MikanCSharpDXTestApp()
		{
			clock = new Stopwatch();
			mikanAPI = new MikanAPI();

			// Set window properties
			renderForm = new RenderForm("Mikan C# D3D11 Test");
			renderForm.ClientSize = new Size(windowWidth, windowHeight);
			renderForm.AllowUserResizing = false;
		}

		public bool Initialize()
		{
			bool bSuccess;

			try
			{
				InitSwapChain(windowWidth, windowHeight);

				bSuccess= InitializeCubeShader() && InitializeDepthPackShader();
				if (bSuccess)
				{
					InitializeGeometry();

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
			Utilities.Dispose(ref inputSignature);
			Utilities.Dispose(ref cubeVertexShader);
			Utilities.Dispose(ref cubePixelShader);
			Utilities.Dispose(ref defaultBackBuffer);
			Utilities.Dispose(ref defaultRenderTargetView);
			Utilities.Dispose(ref defaultDepthBuffer);
			Utilities.Dispose(ref defaultDepthView);
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
			Utilities.Dispose(ref defaultBackBuffer);
			Utilities.Dispose(ref defaultRenderTargetView);
			Utilities.Dispose(ref defaultDepthBuffer);
			Utilities.Dispose(ref defaultDepthView);

			if (swapChain != null)
			{
				// Resize the back buffer
				swapChain.ResizeBuffers(1, newWidth, newHeight, Format.B8G8R8A8_UNorm, SwapChainFlags.None);

				// Get the back buffer from the swap chain
				defaultBackBuffer = swapChain.GetBackBuffer<D3D11.Texture2D>(0);

				// Create a render target view for the back buffer
				defaultRenderTargetView = new D3D11.RenderTargetView(d3dDevice, defaultBackBuffer);

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
				defaultDepthView = new D3D11.DepthStencilView(d3dDevice, defaultDepthBuffer);

				// Setup the targets and viewport for rendering
				d3dDeviceContext.Rasterizer.SetViewport(
					new Viewport(0, 0, windowWidth, windowHeight, 0.0f, 1.0f));
				d3dDeviceContext.OutputMerger.SetTargets(defaultDepthView, defaultRenderTargetView);
			}
		}

		private void FreeFrameBuffer()
		{
			Utilities.Dispose(ref renderTargetSRV);
			Utilities.Dispose(ref renderTargetView);
			Utilities.Dispose(ref renderTargetTexture);
			Utilities.Dispose(ref depthTargetSRV);
			Utilities.Dispose(ref depthTargetView);
			Utilities.Dispose(ref depthTargetTexture);
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

		private void CreateFrameBuffer(int newWidth, int newHeight)
		{
			// Create the render target resources
			// -------

			// Create the render target texture.
			renderTargetTexture = new D3D11.Texture2D(
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
			renderTargetView = new D3D11.RenderTargetView(
				d3dDevice, 
				renderTargetTexture, 
				new RenderTargetViewDescription() { 
					Format = Format.B8G8R8A8_UNorm, 
					Dimension = RenderTargetViewDimension.Texture2D,
					Texture2D = new RenderTargetViewDescription.Texture2DResource() { MipSlice = 0 }
				});

			// Create the shader resource view
			renderTargetSRV = new D3D11.ShaderResourceView(
				d3dDevice, 
				renderTargetTexture,
				new ShaderResourceViewDescription() { 
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
			depthTargetTexture = new D3D11.Texture2D(
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
			depthTargetView = new D3D11.DepthStencilView(
				d3dDevice,
				depthTargetTexture,
				new DepthStencilViewDescription()
				{
					Format = depthViewFormat,
					Dimension = DepthStencilViewDimension.Texture2D,
					Texture2D = new DepthStencilViewDescription.Texture2DResource() { MipSlice = 0 }
				});

			// Create the shader resource view
			depthTargetSRV = new D3D11.ShaderResourceView(
				d3dDevice,
				depthTargetTexture,
				new ShaderResourceViewDescription()
				{
					Format = srvformat,
					Dimension = ShaderResourceViewDimension.Texture2D,
					Texture2D = new ShaderResourceViewDescription.Texture2DResource() { MipLevels = 1, MostDetailedMip = 0 }
				});
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
			inputSignature = ShaderSignature.GetInputSignature(vertexShaderByteCode);

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
			projectionMatrix = Matrix.PerspectiveFovLH(MathUtil.PiOverFour, windowWidth / (float)windowHeight, 0.1f, 100.0f);
			viewMatrix = Matrix.LookAtLH(new Vector3(0, 0, -10), new Vector3(0, 0, 0), new Vector3(0, 1, 0));

			return true;
		}

		private bool InitializeDepthPackShader()
		{
			string shaderCodeString = @"
				Texture2D<float> InputTexture : register(t0);
				SamplerState samLinear : register(s0);

				struct PS_INPUT
				{
					float4 pos : SV_POSITION;
					float2 uv : TEXCOORD;
				};

				PS_INPUT vs_main(uint id : SV_VertexId)
				{
					PS_INPUT output = (PS_INPUT)0;

					output.uv = float2(id % 2, (id % 4) >> 1);
					output.pos = float4((output.uv.x - 0.5f) * 2, -(output.uv.y - 0.5f) * 2, 0, 1);

					return output;
				}

				float4 ps_main(PS_INPUT input) : SV_TARGET
				{
					// https://aras-p.info/blog/2009/07/30/encoding-floats-to-rgba-the-final/
					float floatValue = InputTexture.Sample(samLinear, input.uv).r;
					float4 encodedValue = float4(1.0, 255.0, 65025.0, 16581375.0) * floatValue;
					encodedValue = frac(encodedValue);
					encodedValue -= encodedValue.yzww * float4(1.0/255.0,1.0/255.0,1.0/255.0,0.0);

					return encodedValue;
				}";

			// Compile the vertex shader code
			var vertexShaderByteCode = ShaderBytecode.Compile(shaderCodeString, "vs_main", "vs_4_0", ShaderFlags.Debug);
			if (vertexShaderByteCode.HasErrors)
			{
				Log(MikanLogLevel.Fatal, vertexShaderByteCode.Message);
				return false;
			}
			depthPackVertexShader = new D3D11.VertexShader(d3dDevice, vertexShaderByteCode);

			// Compile the pixel shader code
			var pixelShaderByteCode = ShaderBytecode.Compile(shaderCodeString, "ps_main", "ps_4_0", ShaderFlags.Debug);
			if (pixelShaderByteCode.HasErrors)
			{
				Log(MikanLogLevel.Fatal, pixelShaderByteCode.Message);
				return false;
			}
			depthPackPixelShader = new D3D11.PixelShader(d3dDevice, pixelShaderByteCode);

			return true;
		}

		private void InitializeGeometry()
		{
			// Create the input layout from the input signature and the input elements
			var inputLayout = new InputLayout(d3dDevice, inputSignature, new[]
				{
					new InputElement("POSITION", 0, Format.R32G32B32A32_Float, 0, 0),
					new InputElement("COLOR", 0, Format.R32G32B32A32_Float, 16, 0)
				});

			// Instantiate Vertex buiffer from vertex data
			vertices = D3D11.Buffer.Create(d3dDevice, BindFlags.VertexBuffer, new[]
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
			vertexCount= 36;

			d3dDeviceContext.InputAssembler.InputLayout = inputLayout;
			d3dDeviceContext.InputAssembler.SetVertexBuffers(
				0,
				new VertexBufferBinding(vertices, Utilities.SizeOf<Vector4>() * 2, 0));
			d3dDeviceContext.InputAssembler.PrimitiveTopology = PrimitiveTopology.TriangleList;
		}

		private	bool InitializeMikan()
		{
			mikanAPI.SetLogCallback(this.Log);

			if (mikanAPI.Initialize(MikanLogLevel.Info) != MikanResult.Success)
			{
				return false;
			}

			if (mikanAPI.SetClientInfo(new MikanClientInfo() { 
				engineName = "MikanXR Test", 
				engineVersion = "1.0.0",
				applicationName = "MikanXR C# Test App",
				applicationVersion = "1.0.0",
				graphicsAPI = MikanClientGraphicsApi.Direct3D11,
				supportsRGB24 = true}) != MikanResult.Success)
			{
				return false;
			}

			if (mikanAPI.SetGraphicsDeviceInterface(
				MikanClientGraphicsApi.Direct3D11, 
				d3dDevice.NativePointer) != MikanResult.Success)
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

		private void UpdateLoop()
		{
			long now = clock.ElapsedMilliseconds;
			float deltaMilliseconds = now - lastUpdateTimestamp;
			float deltaSecondsClamped = Math.Min(deltaMilliseconds / 1000.0f, .1f);

			// Update timekeeping
			time += deltaSecondsClamped;
			lastUpdateTimestamp = now;

			if (mikanAPI.GetIsConnected())
			{
				while (mikanAPI.FetchNextEvent(out MikanEvent nextEvent) == MikanResult.Success)
				{
					if (nextEvent is MikanConnectedEvent)
					{
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
					if (mikanAPI.Connect() != MikanResult.Success || !mikanAPI.GetIsConnected())
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

			await mikanAPI.FreeRenderTargetBuffers();

			MikanResponse response = await mikanAPI.VideoSourceAPI.GetVideoSourceMode();
			if (response.resultCode == MikanResult.Success)
			{
				var mode = response as MikanVideoSourceMode;

				var desc = new MikanRenderTargetDescriptor()
				{
					width = (uint)mode.resolution_x,
					height = (uint)mode.resolution_y,
					color_buffer_type = MikanColorBufferType.BGRA32,
					depth_buffer_type = MikanDepthBufferType.NODEPTH,
					graphicsAPI = MikanClientGraphicsApi.Direct3D11
				};

				// Tell the server to allocate new render target buffers
				await mikanAPI.AllocateRenderTargetBuffers(ref desc);

				// Create a new frame buffer to render to
				CreateFrameBuffer(mode.resolution_x, mode.resolution_y);
			}
		}

		async void UpdateCameraProjectionMatrix()
		{
			var response = await mikanAPI.VideoSourceAPI.GetVideoSourceIntrinsics();
			if (response.resultCode == MikanResult.Success)
			{
				var videoSourceIntrinsics = response as MikanVideoSourceIntrinsics;

				var monoIntrinsics = videoSourceIntrinsics.mono;
				float videoSourcePixelWidth = (float)monoIntrinsics.pixel_width;
				float videoSourcePixelHeight = (float)monoIntrinsics.pixel_height;

				projectionMatrix = Matrix.PerspectiveFovLH(
					MathUtil.DegreesToRadians((float)monoIntrinsics.vfov),
					videoSourcePixelWidth / videoSourcePixelHeight,
					(float)monoIntrinsics.znear,
					(float)monoIntrinsics.zfar);
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
			if (renderTargetTexture != null)
			{
				var frameRendered = new MikanClientFrameRendered()
				{
					frame_index = newFrameEvent.frame
				};
				
				mikanAPI.PublishRenderTargetTexture(renderTargetTexture.NativePointer, ref frameRendered);
			}

			// Remember the frame index of the last frame we published
			lastReceivedVideoSourceFrame = newFrameEvent.frame;
		}

		void SetCameraPose(
			MikanVector3f mikanForward,
			MikanVector3f mikanUp,
			MikanVector3f mikanPosition)
		{
			var cameraForward = MikanVector3fToSharpDXVector3(mikanForward);
			var cameraUp = MikanVector3fToSharpDXVector3(mikanUp);
			var cameraPosition = MikanVector3fToSharpDXVector3(mikanPosition);
			var targetPosition = cameraPosition + cameraForward;

			// Apply the camera pose received
			Matrix.LookAtLH(
				ref cameraPosition,
				ref targetPosition,
				ref cameraUp,
				out viewMatrix);
		}

		private void Draw()
		{
			// Assign the cube shader
			d3dDeviceContext.VertexShader.SetConstantBuffer(0, cubeShaderContantBuffer);
			d3dDeviceContext.VertexShader.Set(cubeVertexShader);
			d3dDeviceContext.PixelShader.Set(cubePixelShader);

			// Update the model view projection matrix used to render the geometry
			var viewProj = Matrix.Multiply(viewMatrix, projectionMatrix);
			var modelViewProj =
				Matrix.RotationX(time) *
				Matrix.RotationY(time * 2) *
				Matrix.RotationZ(time * .7f) *
				viewProj;
			modelViewProj.Transpose();
			d3dDeviceContext.UpdateSubresource(ref modelViewProj, cubeShaderContantBuffer);

			// Draw to the render/depth target texture
			if (renderTargetView != null && depthTargetView != null)
			{
				// Set the render/depth target to the render/depth target texture
				d3dDeviceContext.OutputMerger.SetTargets(depthTargetView, renderTargetView);

				// Clear the depth/render textures
				d3dDeviceContext.ClearDepthStencilView(depthTargetView, DepthStencilClearFlags.Depth, 1.0f, 0);
				d3dDeviceContext.ClearRenderTargetView(renderTargetView, new SharpDX.Color(32, 103, 178, 0));

				// Draw the triangle
				d3dDeviceContext.Draw(vertexCount, 0);

				// Restore back to the default render target
				d3dDeviceContext.OutputMerger.SetTargets(defaultDepthView, defaultRenderTargetView);
			}

			// Clear the screen
			d3dDeviceContext.ClearDepthStencilView(defaultDepthView, DepthStencilClearFlags.Depth, 1.0f, 0);
			d3dDeviceContext.ClearRenderTargetView(defaultRenderTargetView, new SharpDX.Color(32, 103, 178, 0));

			// Draw the triangle
			d3dDeviceContext.Draw(vertexCount, 0);

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