using System.IO;

namespace MikanXR
{
	/// A 3D vector with double components.
	public struct MikanVector3d
	{
		public double x { get; set; }
		public double y { get; set; }
		public double z { get; set; }

		public void ReadFromBinaryReader(BinaryReader inReader)
		{
			x = inReader.ReadDouble();
			y = inReader.ReadDouble();
			z = inReader.ReadDouble();
		}
	};

	/// A 2D vector with float components.
	public struct MikanVector2f
	{
		public float x { get; set; }
		public float y { get; set; }

		public void ReadFromBinaryReader(BinaryReader inReader)
		{
			x = inReader.ReadSingle();
			y = inReader.ReadSingle();
		}
	};

	/// A 3D vector with float components.
	public struct MikanVector3f
	{
		public float x { get; set; }
		public float y { get; set; }
		public float z { get; set; }

		public void ReadFromBinaryReader(BinaryReader inReader)
		{
			x = inReader.ReadSingle();
			y = inReader.ReadSingle();
			z = inReader.ReadSingle();
		}
	};

	/// A 3-tuple of Euler angles with float components.
	public struct MikanRotator3f
	{
		public float x_angle { get; set; }
		public float y_angle { get; set; }
		public float z_angle { get; set; }

		public void ReadFromBinaryReader(BinaryReader inReader)
		{
			x_angle = inReader.ReadSingle();
			y_angle = inReader.ReadSingle();
			z_angle = inReader.ReadSingle();
		}
	};

	/** A 4x4 matrix with float components
		storage is column major order:

		| x0 y0 z0 w0 |
		| x1 y1 z1 w1 |
		| x2 y2 z2 w2 |
		| x3 y3 z3 w3 |
	 */
	public struct MikanMatrix4f
	{
		public float x0 { get; set; }
		public float x1 { get; set; }
		public float x2 { get; set; }
		public float x3 { get; set; }
		public float y0 { get; set; }
		public float y1 { get; set; }
		public float y2 { get; set; }
		public float y3 { get; set; }
		public float z0 { get; set; }
		public float z1 { get; set; }
		public float z2 { get; set; }
		public float z3 { get; set; }
		public float w0 { get; set; }
		public float w1 { get; set; }
		public float w2 { get; set; }
		public float w3 { get; set; }

		public void ReadFromBinaryReader(BinaryReader inReader)
		{
			x0 = inReader.ReadSingle();
			x1 = inReader.ReadSingle();
			x2 = inReader.ReadSingle();
			x3 = inReader.ReadSingle();
			y0 = inReader.ReadSingle();
			y1 = inReader.ReadSingle();
			y2 = inReader.ReadSingle();
			y3 = inReader.ReadSingle();
			z0 = inReader.ReadSingle();
			z1 = inReader.ReadSingle();
			z2 = inReader.ReadSingle();
			z3 = inReader.ReadSingle();
			w0 = inReader.ReadSingle();
			w1 = inReader.ReadSingle();
			w2 = inReader.ReadSingle();
			w3 = inReader.ReadSingle();
		}
	};

	/** A 3x3 matrix with double components
		storage is column major order:

		| x0 y0 z0 |
		| x1 y1 z1 |
		| x2 y2 z2 |
	 */
	public struct MikanMatrix3d
	{
		public double x0 { get; set; }
		public double x1 { get; set; }
		public double x2 { get; set; }
		public double y0 { get; set; }
		public double y1 { get; set; }
		public double y2 { get; set; }
		public double z0 { get; set; }
		public double z1 { get; set; }
		public double z2 { get; set; }

		public void ReadFromBinaryReader(BinaryReader inReader)
		{
			x0 = inReader.ReadDouble();
			x1 = inReader.ReadDouble();
			x2 = inReader.ReadDouble();
			y0 = inReader.ReadDouble();
			y1 = inReader.ReadDouble();
			y2 = inReader.ReadDouble();
			z0 = inReader.ReadDouble();
			z1 = inReader.ReadDouble();
			z2 = inReader.ReadDouble();
		}
	};

	/** A 4x3 matrix with double components
		storage is column major order:

		| x0 y0 z0 w0|
		| x1 y1 z1 w1|
		| x2 y2 z2 w2|
	 */
	public struct MikanMatrix4x3d
	{
		public double x0 { get; set; }
		public double x1 { get; set; }
		public double x2 { get; set; }
		public double x3 { get; set; }
		public double y0 { get; set; }
		public double y1 { get; set; }
		public double y2 { get; set; }
		public double y3 { get; set; }
		public double z0 { get; set; }
		public double z1 { get; set; }
		public double z2 { get; set; }
		public double z3 { get; set; }

		public void ReadFromBinaryReader(BinaryReader inReader)
		{
			x0 = inReader.ReadDouble();
			x1 = inReader.ReadDouble();
			x2 = inReader.ReadDouble();
			x3 = inReader.ReadDouble();
			y0 = inReader.ReadDouble();
			y1 = inReader.ReadDouble();
			y2 = inReader.ReadDouble();
			y3 = inReader.ReadDouble();
			z0 = inReader.ReadDouble();
			z1 = inReader.ReadDouble();
			z2 = inReader.ReadDouble();
			z3 = inReader.ReadDouble();
		}
	};

	/** A 4x4 matrix with double components
		storage is column major order:

		| x0 y0 z0 w0 |
		| x1 y1 z1 w1 |
		| x2 y2 z2 w2 |
		| x3 y3 z3 w3 |
	 */
	public struct MikanMatrix4d
	{
		public double x0 { get; set; }
		public double x1 { get; set; }
		public double x2 { get; set; }
		public double x3 { get; set; }
		public double y0 { get; set; }
		public double y1 { get; set; }
		public double y2 { get; set; }
		public double y3 { get; set; }
		public double z0 { get; set; }
		public double z1 { get; set; }
		public double z2 { get; set; }
		public double z3 { get; set; }
		public double w0 { get; set; }
		public double w1 { get; set; }
		public double w2 { get; set; }
		public double w3 { get; set; }

		public void ReadFromBinaryReader(BinaryReader inReader)
		{
			x0 = inReader.ReadDouble();
			x1 = inReader.ReadDouble();
			x2 = inReader.ReadDouble();
			x3 = inReader.ReadDouble();
			y0 = inReader.ReadDouble();
			y1 = inReader.ReadDouble();
			y2 = inReader.ReadDouble();
			y3 = inReader.ReadDouble();
			z0 = inReader.ReadDouble();
			z1 = inReader.ReadDouble();
			z2 = inReader.ReadDouble();
			z3 = inReader.ReadDouble();
			w0 = inReader.ReadDouble();
			w1 = inReader.ReadDouble();
			w2 = inReader.ReadDouble();
			w3 = inReader.ReadDouble();
		}
	};

	/// A double-precision quaternion rotation.
	public struct MikanQuatf
	{
		public float w { get; set; }
		public float x { get; set; }
		public float y { get; set; }
		public float z { get; set; }

		public void ReadFromBinaryReader(BinaryReader inReader)
		{
			w = inReader.ReadSingle();
			x = inReader.ReadSingle();
			y = inReader.ReadSingle();
			z = inReader.ReadSingle();
		}
	};

	/// A double-precision quaternion rotation.
	public struct MikanQuatd
	{
		public double w { get; set; }
		public double x { get; set; }
		public double y { get; set; }
		public double z { get; set; }

		public void ReadFromBinaryReader(BinaryReader inReader)
		{
			w = inReader.ReadDouble();
			x = inReader.ReadDouble();
			y = inReader.ReadDouble();
			z = inReader.ReadDouble();
		}
	};

	/// A single-precision Scale-Rotation-Translation transform.
	public struct MikanTransform
	{
		public MikanVector3f scale { get; set; }
		public MikanQuatf rotation { get; set; }
		public MikanVector3f position { get; set; }

		public void ReadFromBinaryReader(BinaryReader inReader)
		{
			scale.ReadFromBinaryReader(inReader);
			rotation.ReadFromBinaryReader(inReader);
			position.ReadFromBinaryReader(inReader);
		}
	};	
}