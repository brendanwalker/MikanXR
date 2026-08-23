using System;
using SharpDX;
using SharpDX.D3DCompiler;
using SharpDX.DXGI;
using D3D11 = SharpDX.Direct3D11;

namespace Mikan
{
	public static class TestDirectXUtils
	{
		// Depth format conversion utilities
		public static Format GetDepthResourceFormat(Format depthFormat)
		{
			switch (depthFormat)
			{
				case Format.D16_UNorm:
					return Format.R16_Typeless;
				case Format.D24_UNorm_S8_UInt:
					return Format.R24G8_Typeless;
				case Format.D32_Float:
					return Format.R32_Typeless;
				case Format.D32_Float_S8X24_UInt:
					return Format.R32G8X24_Typeless;
				default:
					return Format.Unknown;
			}
		}

		public static Format GetDepthSRVFormat(Format depthFormat)
		{
			switch (depthFormat)
			{
				case Format.D16_UNorm:
					return Format.R16_Float;
				case Format.D24_UNorm_S8_UInt:
					return Format.R24_UNorm_X8_Typeless;
				case Format.D32_Float:
					return Format.R32_Float;
				case Format.D32_Float_S8X24_UInt:
					return Format.R32_Float_X8X24_Typeless;
				default:
					return Format.Unknown;
			}
		}

		// Create color render target resources
		public static bool CreateColorRenderTargetResources(
			D3D11.Device device,
			int width,
			int height,
			out D3D11.Texture2D colorTexture,
			out D3D11.RenderTargetView colorRTV,
			out D3D11.ShaderResourceView colorSRV)
		{
			colorTexture = null;
			colorRTV = null;
			colorSRV = null;

			try
			{
				// Create the render target texture
				colorTexture = new D3D11.Texture2D(
					device,
					new D3D11.Texture2DDescription()
					{
						Width = width,
						Height = height,
						MipLevels = 1,
						ArraySize = 1,
						Format = Format.B8G8R8A8_Typeless,
						SampleDescription = new SampleDescription(1, 0),
						Usage = D3D11.ResourceUsage.Default,
						BindFlags = D3D11.BindFlags.RenderTarget | D3D11.BindFlags.ShaderResource,
						CpuAccessFlags = D3D11.CpuAccessFlags.None,
						OptionFlags = D3D11.ResourceOptionFlags.Shared
					});

				// Create the render target view
				colorRTV = new D3D11.RenderTargetView(
					device,
					colorTexture,
					new D3D11.RenderTargetViewDescription()
					{
						Format = Format.B8G8R8A8_UNorm,
						Dimension = D3D11.RenderTargetViewDimension.Texture2D,
						Texture2D = new D3D11.RenderTargetViewDescription.Texture2DResource()
						{
							MipSlice = 0
						}
					});

				// Create the shader resource view
				colorSRV = new D3D11.ShaderResourceView(
					device,
					colorTexture,
					new D3D11.ShaderResourceViewDescription()
					{
						Format = Format.B8G8R8A8_UNorm,
						Dimension = SharpDX.Direct3D.ShaderResourceViewDimension.Texture2D,
						Texture2D = new D3D11.ShaderResourceViewDescription.Texture2DResource()
						{
							MipLevels = 1,
							MostDetailedMip = 0
						}
					});

				return true;
			}
			catch (Exception e)
			{
				Console.WriteLine($"ERROR: Failed to create color render target: {e.Message}");
				colorTexture?.Dispose();
				colorRTV?.Dispose();
				colorSRV?.Dispose();
				return false;
			}
		}

		// Create depth render target resources
		public static bool CreateDepthRenderTargetResources(
			D3D11.Device device,
			int width,
			int height,
			out D3D11.Texture2D depthTexture,
			out D3D11.DepthStencilView depthDSV,
			out D3D11.ShaderResourceView depthSRV)
		{
			depthTexture = null;
			depthDSV = null;
			depthSRV = null;

			Format depthFormat = Format.D32_Float;
			Format resourceFormat = GetDepthResourceFormat(depthFormat);
			Format srvFormat = GetDepthSRVFormat(depthFormat);

			try
			{
				// Create the depth target texture
				depthTexture = new D3D11.Texture2D(
					device,
					new D3D11.Texture2DDescription()
					{
						Width = width,
						Height = height,
						MipLevels = 1,
						ArraySize = 1,
						Format = resourceFormat,
						SampleDescription = new SampleDescription(1, 0),
						Usage = D3D11.ResourceUsage.Default,
						BindFlags = D3D11.BindFlags.DepthStencil | D3D11.BindFlags.ShaderResource,
						CpuAccessFlags = D3D11.CpuAccessFlags.None,
						OptionFlags = D3D11.ResourceOptionFlags.Shared
					});

				// Create the depth stencil view
				depthDSV = new D3D11.DepthStencilView(
					device,
					depthTexture,
					new D3D11.DepthStencilViewDescription()
					{
						Format = depthFormat,
						Dimension = D3D11.DepthStencilViewDimension.Texture2D,
						Texture2D = new D3D11.DepthStencilViewDescription.Texture2DResource()
						{
							MipSlice = 0
						}
					});

				// Create the shader resource view
				depthSRV = new D3D11.ShaderResourceView(
					device,
					depthTexture,
					new D3D11.ShaderResourceViewDescription()
					{
						Format = srvFormat,
						Dimension = SharpDX.Direct3D.ShaderResourceViewDimension.Texture2D,
						Texture2D = new D3D11.ShaderResourceViewDescription.Texture2DResource()
						{
							MipLevels = 1,
							MostDetailedMip = 0
						}
					});

				return true;
			}
			catch (Exception e)
			{
				Console.WriteLine($"ERROR: Failed to create depth render target: {e.Message}");
				depthTexture?.Dispose();
				depthDSV?.Dispose();
				depthSRV?.Dispose();
				return false;
			}
		}

		// Compile shader from string
		public static bool CompileShaderFromString(
			string shaderCode,
			string entryPoint,
			string shaderModel,
			out ShaderBytecode bytecode)
		{
			bytecode = null;

			try
			{
				var result = ShaderBytecode.Compile(
					shaderCode,
					entryPoint,
					shaderModel,
					ShaderFlags.None,
					EffectFlags.None);

				if (result.HasErrors)
				{
					Console.WriteLine($"ERROR: Shader compilation failed: {result.Message}");
					return false;
				}

				bytecode = result.Bytecode;
				return true;
			}
			catch (Exception e)
			{
				Console.WriteLine($"ERROR: Shader compilation exception: {e.Message}");
				return false;
			}
		}
	}
}
