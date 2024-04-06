using SharpDX;
using SharpDX.D3DCompiler;
using SharpDX.Direct3D;
using SharpDX.DXGI;
using SharpDX.Windows;
using D3D11 = SharpDX.Direct3D11;
using System;
using System.Drawing;
using System.Linq;
using MikanXR;

namespace Mikan
{
	public class MikanCSharpDXTestApp : IDisposable
	{
		private MikanAPI mikanAPI;
		private float mikanReconnectTimeout = 0f;
		private DateTime lastUpdateTimestamp = DateTime.Now;
		private UInt64 lastReceivedVideoSourceFrame = 0;

		private RenderForm renderForm;
		private const int windowWidth = 1280;
		private const int windowHeight = 720;

		private D3D11.Device d3dDevice;
		private D3D11.DeviceContext d3dDeviceContext;
		private SwapChain swapChain;
		private D3D11.RenderTargetView renderTargetView;
		private D3D11.Texture2D g_renderTargetTexture;
		private Viewport viewport;
		private Matrix viewMatrix;
		private Matrix projectionMatrix;

		// Shaders
		private D3D11.VertexShader vertexShader;
		private D3D11.PixelShader pixelShader;
		private ShaderSignature inputSignature;
		private D3D11.InputLayout inputLayout;

		private D3D11.InputElement[] inputElements = new D3D11.InputElement[]
		{
			new D3D11.InputElement("POSITION", 0, Format.R32G32B32_Float, 0)
		};

		// Triangle vertices
		private Vector3[] vertices = new Vector3[] { new Vector3(-0.5f, 0.5f, 0.0f), new Vector3(0.5f, 0.5f, 0.0f), new Vector3(0.0f, -0.5f, 0.0f) };
		private D3D11.Buffer triangleVertexBuffer;


		public MikanCSharpDXTestApp()
		{
			mikanAPI = new MikanAPI();

			// Set window properties
			renderForm = new RenderForm("My first SharpDX game");
			renderForm.ClientSize = new Size(windowWidth, windowHeight);
			renderForm.AllowUserResizing = false;

			InitSwapChain(windowWidth, windowHeight);
			InitializeShaders();
			InitializeTriangle();
			InitializeMikan();
		}
		
		public void Run()
		{
			// Start render loop
			// Does not return until close event is received
			RenderLoop.Run(renderForm, RenderCallback);
		}

		private void RenderCallback()
		{
			DateTime now= DateTime.Now;
			float deltaTime = Math.Min(now.Subtract(lastUpdateTimestamp).Milliseconds / 1000.0f, .1f);

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
					mikanReconnectTimeout -= deltaTime;
				}

				// Just render the scene using the last applied camera pose
				Draw();
			}

			lastUpdateTimestamp = now;
		}

		private void InitSwapChain(int textureWidth, int textureHeight)
		{
			ModeDescription backBufferDesc = 
				new ModeDescription(windowWidth, windowHeight, new Rational(60, 1), Format.R8G8B8A8_Typeless);

			// Descriptor for the swap chain
			SwapChainDescription swapChainDesc = new SwapChainDescription()
			{
				ModeDescription = backBufferDesc,
				SampleDescription = new SampleDescription(1, 0),
				Usage = Usage.RenderTargetOutput,
				BufferCount = 1,
				OutputHandle = renderForm.Handle,
				IsWindowed = true
			};

			// Create device and swap chain
			D3D11.Device.CreateWithSwapChain(DriverType.Hardware, D3D11.DeviceCreationFlags.None, swapChainDesc, out d3dDevice, out swapChain);
			d3dDeviceContext = d3dDevice.ImmediateContext;

			viewport = new Viewport(0, 0, windowWidth, windowHeight);
			d3dDeviceContext.Rasterizer.SetViewport(viewport);

			// Setup default camera and projection matrices
			projectionMatrix = Matrix.PerspectiveFovLH(MathUtil.PiOverFour, windowWidth / (float)windowHeight, 0.1f, 100.0f);
			viewMatrix = Matrix.LookAtLH(new Vector3(0, 0, -1), new Vector3(0, 0, 0), new Vector3(0, 1, 0));

			// Create render target view for back buffer
			using (D3D11.Texture2D backBuffer = swapChain.GetBackBuffer<D3D11.Texture2D>(0))
			{
				renderTargetView = new D3D11.RenderTargetView(d3dDevice, backBuffer);

				if (renderTargetView.Resource != null)
				{
					g_renderTargetTexture = renderTargetView.Resource.QueryInterfaceOrNull<D3D11.Texture2D>();
				}
			}
		}

		private void ResizeRenderTargetView(int textureWidth, int textureHeight)
		{
			if (renderTargetView != null)
			{
				renderTargetView.Dispose();
				renderTargetView= null;
			}

			if (swapChain != null)
			{
				swapChain.ResizeBuffers(1, textureWidth, textureHeight, Format.R8G8B8A8_Typeless, SwapChainFlags.None);

				using (D3D11.Texture2D backBuffer = swapChain.GetBackBuffer<D3D11.Texture2D>(0))
				{
					renderTargetView = new D3D11.RenderTargetView(d3dDevice, backBuffer);

					if (renderTargetView.Resource != null)
					{
						g_renderTargetTexture = renderTargetView.Resource.QueryInterfaceOrNull<D3D11.Texture2D>();
					}
				}
			}
		}

		private void InitializeShaders()
		{
			// Compile the vertex shader code
			using (var vertexShaderByteCode = ShaderBytecode.CompileFromFile("vertexShader.hlsl", "main", "vs_4_0", ShaderFlags.Debug))
			{
				// Read input signature from shader code
				inputSignature = ShaderSignature.GetInputSignature(vertexShaderByteCode);

				vertexShader = new D3D11.VertexShader(d3dDevice, vertexShaderByteCode);
			}

			// Compile the pixel shader code
			using (var pixelShaderByteCode = ShaderBytecode.CompileFromFile("pixelShader.hlsl", "main", "ps_4_0", ShaderFlags.Debug))
			{
				pixelShader = new D3D11.PixelShader(d3dDevice, pixelShaderByteCode);
			}

			// Set as current vertex and pixel shaders
			d3dDeviceContext.VertexShader.Set(vertexShader);
			d3dDeviceContext.PixelShader.Set(pixelShader);

			d3dDeviceContext.InputAssembler.PrimitiveTopology = PrimitiveTopology.TriangleList;

			// Create the input layout from the input signature and the input elements
			inputLayout = new D3D11.InputLayout(d3dDevice, inputSignature, inputElements);

			// Set input layout to use
			d3dDeviceContext.InputAssembler.InputLayout = inputLayout;
		}

		private void InitializeTriangle()
		{
			// Create a vertex buffer, and use our array with vertices as data
			triangleVertexBuffer = D3D11.Buffer.Create<Vector3>(d3dDevice, D3D11.BindFlags.VertexBuffer, vertices);
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
				ResizeRenderTargetView(mode.resolution_x, mode.resolution_y);
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

			// Publish the new video frame back to Mikan
			if (g_renderTargetTexture != null)
			{
				var frameRendered = new MikanClientFrameRendered()
				{
					frame_index = newFrameEvent.frame
				};
				
				mikanAPI.PublishRenderTargetTexture(g_renderTargetTexture.NativePointer, ref frameRendered);
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
			// Set back buffer as current render target view
			d3dDeviceContext.OutputMerger.SetRenderTargets(renderTargetView);

			// Clear the screen
			d3dDeviceContext.ClearRenderTargetView(renderTargetView, new SharpDX.Color(32, 103, 178));

			// Set vertex buffer
			d3dDeviceContext.InputAssembler.SetVertexBuffers(0, new D3D11.VertexBufferBinding(triangleVertexBuffer, Utilities.SizeOf<Vector3>(), 0));

			// Draw the triangle
			d3dDeviceContext.Draw(vertices.Count(), 0);

			// Swap front and back buffer
			swapChain.Present(1, PresentFlags.None);
		}

		public void Dispose()
		{
			mikanAPI.Dispose();
			inputLayout.Dispose();
			inputSignature.Dispose();
			triangleVertexBuffer.Dispose();
			vertexShader.Dispose();
			pixelShader.Dispose();

			if (g_renderTargetTexture != null)
			{
				g_renderTargetTexture.Dispose();
				g_renderTargetTexture = null;
			}

			renderTargetView.Dispose();
			swapChain.Dispose();
			d3dDevice.Dispose();
			d3dDeviceContext.Dispose();
			renderForm.Dispose();
		}

		[STAThread]
		static void Main(string[] args)
		{
			using (var app = new MikanCSharpDXTestApp())
			{
				app.Run();
			}
		}
	}
}