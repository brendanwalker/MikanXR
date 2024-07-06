using System.Collections.Generic;
using System.IO;

namespace MikanXR
{
	public class MikanStencilQuadInfo : MikanResponse
	{
		public int stencil_id { get; set; }
		public int parent_anchor_id { get; set; }
		public MikanTransform relative_transform { get; set; }
		public float quad_width { get; set; }
		public float quad_height { get; set; }
		public bool is_double_sided { get; set; }
		public bool is_disabled { get; set; }
		public string stencil_name { get; set; }

		public MikanStencilQuadInfo() : base(typeof(MikanStencilQuadInfo).Name) {}
	};

	public class MikanStencilBoxInfo : MikanResponse
	{
		public int stencil_id { get; set; }
		public int parent_anchor_id { get; set; }
		public MikanTransform relative_transform { get; set; }
		public float box_x_size { get; set; }
		public float box_y_size { get; set; }
		public float box_z_size { get; set; }
		public bool is_disabled { get; set; }
		public string stencil_name { get; set; }

		public MikanStencilBoxInfo() : base(typeof(MikanStencilBoxInfo).Name) {}
	};

	public class MikanStencilModelInfo : MikanResponse
	{
		public int stencil_id { get; set; }
		public int parent_anchor_id { get; set; }
		public MikanTransform relative_transform { get; set; } 
		public bool is_disabled { get; set; }
		public string stencil_name { get; set; }

		public MikanStencilModelInfo() : base(typeof(MikanStencilModelInfo).Name) {}
	};

	public class MikanStencilList : MikanResponse
	{
		public List<int> stencil_id_list { get; set; }

		public MikanStencilList() : base(typeof(MikanStencilList).Name) {}
	};

	public class MikanTriagulatedMesh
	{
		public MikanVector3f[] vertices
		{
			get; set;
		}
		public MikanVector3f[] normals
		{
			get; set;
		}
		public MikanVector2f[] texels
		{
			get; set;
		}
		public int[] indices
		{
			get; set;
		}

		public void ReadFromBinaryReader(BinaryReader inReader)
		{
			int vertexCount = inReader.ReadInt32();
			vertices = new MikanVector3f[vertexCount];
			for (int i = 0; i < vertexCount; i++)
			{
				vertices[i].ReadFromBinaryReader(inReader);
			}

			int normalCount = inReader.ReadInt32();
			normals = new MikanVector3f[normalCount];
			for (int i = 0; i < normalCount; i++)
			{
				normals[i].ReadFromBinaryReader(inReader);
			}

			int texelCount = inReader.ReadInt32();
			texels = new MikanVector2f[texelCount];
			for (int i = 0; i < texelCount; i++)
			{
				texels[i].ReadFromBinaryReader(inReader);
			}

			int indexCount = inReader.ReadInt32();
			indices = new int[indexCount];
			for (int i = 0; i < indexCount; i++)
			{
				indices[i] = inReader.ReadInt32();
			}
		}
	}

	public class MikanStencilModelRenderGeometry : MikanResponse
	{
		public MikanTriagulatedMesh[] meshes
		{
			get; set;
		}

		public MikanStencilModelRenderGeometry() : base(typeof(MikanStencilModelRenderGeometry).Name) { }
	}

	public class MikanStencilModeRenderGeometryFactory :
		MikanBinaryResponseFactory<MikanStencilModelRenderGeometry>
	{
		public override MikanResponse CreateResponse(
			int inRequestId,
			MikanResult inResultCode,
			string inResponseType,
			BinaryReader inReader)
		{
			var response =
				base.CreateResponse(inRequestId, inResultCode, inResponseType, inReader)
				as MikanStencilModelRenderGeometry;

			int meshCount = inReader.ReadInt32();
			response.meshes = new MikanTriagulatedMesh[meshCount];
			for (int i = 0; i < meshCount; i++)
			{
				var mesh = new MikanTriagulatedMesh();
				mesh.ReadFromBinaryReader(inReader);

				response.meshes[i] = mesh;
			}

			return response;
		}
	}

}