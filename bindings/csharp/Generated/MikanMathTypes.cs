// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	// Structs
	public struct MikanVector3d
	{
		public double x { get; set; }
		public double y { get; set; }
		public double z { get; set; }
	};

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

	public struct MikanVector2f
	{
		public float x { get; set; }
		public float y { get; set; }
	};

	public struct MikanTransform
	{
		public MikanVector3f scale { get; set; }
		public MikanQuatf rotation { get; set; }
		public MikanVector3f position { get; set; }
	};

	public struct MikanVector3f
	{
		public float x { get; set; }
		public float y { get; set; }
		public float z { get; set; }
	};

	public struct MikanRotator3f
	{
		public float x_angle { get; set; }
		public float y_angle { get; set; }
		public float z_angle { get; set; }
	};

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

	public struct MikanQuatf
	{
		public float w { get; set; }
		public float x { get; set; }
		public float y { get; set; }
		public float z { get; set; }
	};

	public struct MikanQuatd
	{
		public double w { get; set; }
		public double x { get; set; }
		public double y { get; set; }
		public double z { get; set; }
	};

}
