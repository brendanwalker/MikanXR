namespace MikanXR
{
	/// A 3D vector with double components.
	public struct MikanVector3d
	{
		public double x { get; set; }
		public double y { get; set; }
		public double z { get; set; }
	};

	/// A 2D vector with float components.
	public struct MikanVector2f
	{
		public float x { get; set; }
		public float y { get; set; }
	};

	/// A 3D vector with float components.
	public struct MikanVector3f
	{
		public float x { get; set; }
		public float y { get; set; }
		public float z { get; set; }
	};

	/// A 3-tuple of Euler angles with float components.
	public struct MikanRotator3f
	{
		public float x_angle { get; set; }
		public float y_angle { get; set; }
		public float z_angle { get; set; }
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
	};

	/// A double-precision quaternion rotation.
	public struct MikanQuatf
	{
		public float w { get; set; }
		public float x { get; set; }
		public float y { get; set; }
		public float z { get; set; }
	};

	/// A double-precision quaternion rotation.
	public struct MikanQuatd
	{
		public double w { get; set; }
		public double x { get; set; }
		public double y { get; set; }
		public double z { get; set; }
	};

	/// A single-precision Scale-Rotation-Translation transform.
	public struct MikanTransform
	{
		public MikanVector3f scale { get; set; }
		public MikanQuatf rotation { get; set; }
		public MikanVector3f position { get; set; }
	};	
}