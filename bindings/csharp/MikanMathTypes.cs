namespace MikanXR
{
	/// A 3D vector with double components.
	public struct MikanVector3d
	{
		double x { get; set; }
		double y { get; set; }
		double z { get; set; }
	};

	/// A 2D vector with float components.
	public struct MikanVector2f
	{
		float x { get; set; }
		float y { get; set; }
	};

	/// A 3D vector with float components.
	public struct MikanVector3f
	{
		float x { get; set; }
		float y { get; set; }
		float z { get; set; }
	};

	/// A 3-tuple of Euler angles with float components.
	public struct MikanRotator3f
	{
		float x_angle { get; set; }
		float y_angle { get; set; }
		float z_angle { get; set; }
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
		float x0 { get; set; }
		float x1 { get; set; }
		float x2 { get; set; }
		float x3 { get; set; }
		float y0 { get; set; }
		float y1 { get; set; }
		float y2 { get; set; }
		float y3 { get; set; }
		float z0 { get; set; }
		float z1 { get; set; }
		float z2 { get; set; }
		float z3 { get; set; }
		float w0 { get; set; }
		float w1 { get; set; }
		float w2 { get; set; }
		float w3 { get; set; }
	};

	/** A 3x3 matrix with double components
		storage is column major order:

		| x0 y0 z0 |
		| x1 y1 z1 |
		| x2 y2 z2 |
	 */
	public struct MikanMatrix3d
	{
		double x0 { get; set; }
		double x1 { get; set; }
		double x2 { get; set; }
		double y0 { get; set; }
		double y1 { get; set; }
		double y2 { get; set; }
		double z0 { get; set; }
		double z1 { get; set; }
		double z2 { get; set; }
	};

	/** A 4x3 matrix with double components
		storage is column major order:

		| x0 y0 z0 w0|
		| x1 y1 z1 w1|
		| x2 y2 z2 w2|
	 */
	public struct MikanMatrix4x3d
	{
		double x0 { get; set; }
		double x1 { get; set; }
		double x2 { get; set; }
		double x3 { get; set; }
		double y0 { get; set; }
		double y1 { get; set; }
		double y2 { get; set; }
		double y3 { get; set; }
		double z0 { get; set; }
		double z1 { get; set; }
		double z2 { get; set; }
		double z3 { get; set; }
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
		double x0 { get; set; }
		double x1 { get; set; }
		double x2 { get; set; }
		double x3 { get; set; }
		double y0 { get; set; }
		double y1 { get; set; }
		double y2 { get; set; }
		double y3 { get; set; }
		double z0 { get; set; }
		double z1 { get; set; }
		double z2 { get; set; }
		double z3 { get; set; }
		double w0 { get; set; }
		double w1 { get; set; }
		double w2 { get; set; }
		double w3 { get; set; }
	};

	/// A double-precision quaternion rotation.
	public struct MikanQuatf
	{
		float w { get; set; }
		float x { get; set; }
		float y { get; set; }
		float z { get; set; }
	};

	/// A double-precision quaternion rotation.
	public struct MikanQuatd
	{
		double w { get; set; }
		double x { get; set; }
		double y { get; set; }
		double z { get; set; }
	};

	/// A single-precision Scale-Rotation-Translation transform.
	public struct MikanTransform
	{
		MikanVector3f scale { get; set; }
		MikanQuatf rotation { get; set; }
		MikanVector3f position { get; set; }
	};	
}