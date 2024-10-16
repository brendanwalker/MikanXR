#pragma once

#include "MikanMathTypes.h"
#include "BinaryUtility.h"

inline void to_binary(BinaryWriter& writer, const MikanVector3d& v)
{
	to_binary(writer, v.x);
	to_binary(writer, v.y);
	to_binary(writer, v.z);
}
inline void from_binary(BinaryReader& reader, MikanVector3d& v)
{
	from_binary(reader, v.x);
	from_binary(reader, v.y);
	from_binary(reader, v.z);
}

inline void to_binary(BinaryWriter& writer, const MikanVector2f& v)
{
	to_binary(writer, v.x);
	to_binary(writer, v.y);
}
inline void from_binary(BinaryReader& reader, MikanVector2f& v)
{
	from_binary(reader, v.x);
	from_binary(reader, v.y);
}

inline void to_binary(BinaryWriter& writer, const MikanVector3f& v)
{
	to_binary(writer, v.x);
	to_binary(writer, v.y);
	to_binary(writer, v.z);
}
inline void from_binary(BinaryReader& reader, MikanVector3f& v)
{
	from_binary(reader, v.x);
	from_binary(reader, v.y);
	from_binary(reader, v.z);
}

inline void to_binary(BinaryWriter& writer, const MikanRotator3f& r)
{
	to_binary(writer, r.x_angle);
	to_binary(writer, r.y_angle);
	to_binary(writer, r.z_angle);
}
inline void from_binary(BinaryReader& reader, MikanRotator3f& r)
{
	from_binary(reader, r.x_angle);
	from_binary(reader, r.y_angle);
	from_binary(reader, r.z_angle);
}

inline void to_binary(BinaryWriter& writer, const MikanMatrix4f& m)
{
	to_binary(writer, m.x0);
	to_binary(writer, m.x1);
	to_binary(writer, m.x2);
	to_binary(writer, m.x3);
	to_binary(writer, m.y0);
	to_binary(writer, m.y1);
	to_binary(writer, m.y2);
	to_binary(writer, m.y3);
	to_binary(writer, m.z0);
	to_binary(writer, m.z1);
	to_binary(writer, m.z2);
	to_binary(writer, m.z3);
	to_binary(writer, m.w0);
	to_binary(writer, m.w1);
	to_binary(writer, m.w2);
	to_binary(writer, m.w3);
}
inline void from_binary(BinaryReader& reader, MikanMatrix4f& m)
{
	from_binary(reader, m.x0);
	from_binary(reader, m.x1);
	from_binary(reader, m.x2);
	from_binary(reader, m.x3);
	from_binary(reader, m.y0);
	from_binary(reader, m.y1);
	from_binary(reader, m.y2);
	from_binary(reader, m.y3);
	from_binary(reader, m.z0);
	from_binary(reader, m.z1);
	from_binary(reader, m.z2);
	from_binary(reader, m.z3);
	from_binary(reader, m.w0);
	from_binary(reader, m.w1);
	from_binary(reader, m.w2);
	from_binary(reader, m.w3);
}

inline void to_binary(BinaryWriter& writer, const MikanMatrix3d& m)
{
	to_binary(writer, m.x0);
	to_binary(writer, m.x1);
	to_binary(writer, m.x2);
	to_binary(writer, m.y0);
	to_binary(writer, m.y1);
	to_binary(writer, m.y2);
	to_binary(writer, m.z0);
	to_binary(writer, m.z1);
	to_binary(writer, m.z2);
}
inline void from_binary(BinaryReader& reader, MikanMatrix3d& m)
{
	from_binary(reader, m.x0);
	from_binary(reader, m.x1);
	from_binary(reader, m.x2);
	from_binary(reader, m.y0);
	from_binary(reader, m.y1);
	from_binary(reader, m.y2);
	from_binary(reader, m.z0);
	from_binary(reader, m.z1);
	from_binary(reader, m.z2);
}

inline void to_binary(BinaryWriter& writer, const MikanMatrix4x3d& m)
{
	to_binary(writer, m.x0);
	to_binary(writer, m.x1);
	to_binary(writer, m.x2);
	to_binary(writer, m.x3);
	to_binary(writer, m.y0);
	to_binary(writer, m.y1);
	to_binary(writer, m.y2);
	to_binary(writer, m.y3);
	to_binary(writer, m.z0);
	to_binary(writer, m.z1);
	to_binary(writer, m.z2);
	to_binary(writer, m.z3);
}
inline void from_binary(BinaryReader& reader, MikanMatrix4x3d& m)
{
	from_binary(reader, m.x0);
	from_binary(reader, m.x1);
	from_binary(reader, m.x2);
	from_binary(reader, m.x3);
	from_binary(reader, m.y0);
	from_binary(reader, m.y1);
	from_binary(reader, m.y2);
	from_binary(reader, m.y3);
	from_binary(reader, m.z0);
	from_binary(reader, m.z1);
	from_binary(reader, m.z2);
	from_binary(reader, m.z3);
}

inline void to_binary(BinaryWriter& writer, const MikanMatrix4d& m)
{
	to_binary(writer, m.x0);
	to_binary(writer, m.x1);
	to_binary(writer, m.x2);
	to_binary(writer, m.x3);
	to_binary(writer, m.y0);
	to_binary(writer, m.y1);
	to_binary(writer, m.y2);
	to_binary(writer, m.y3);
	to_binary(writer, m.z0);
	to_binary(writer, m.z1);
	to_binary(writer, m.z2);
	to_binary(writer, m.z3);
	to_binary(writer, m.w0);
	to_binary(writer, m.w1);
	to_binary(writer, m.w2);
	to_binary(writer, m.w3);
}
inline void from_binary(BinaryReader& reader, MikanMatrix4d& m)
{
	from_binary(reader, m.x0);
	from_binary(reader, m.x1);
	from_binary(reader, m.x2);
	from_binary(reader, m.x3);
	from_binary(reader, m.y0);
	from_binary(reader, m.y1);
	from_binary(reader, m.y2);
	from_binary(reader, m.y3);
	from_binary(reader, m.z0);
	from_binary(reader, m.z1);
	from_binary(reader, m.z2);
	from_binary(reader, m.z3);
	from_binary(reader, m.w0);
	from_binary(reader, m.w1);
	from_binary(reader, m.w2);
	from_binary(reader, m.w3);
}

inline void to_binary(BinaryWriter& writer, const MikanQuatf& q)
{
	to_binary(writer, q.w);
	to_binary(writer, q.x);
	to_binary(writer, q.y);
	to_binary(writer, q.z);
}
inline void from_binary(BinaryReader& reader, MikanQuatf& q)
{
	from_binary(reader, q.w);
	from_binary(reader, q.x);
	from_binary(reader, q.y);
	from_binary(reader, q.z);
}

inline void to_binary(BinaryWriter& writer, const MikanQuatd& q)
{
	to_binary(writer, q.w);
	to_binary(writer, q.x);
	to_binary(writer, q.y);
	to_binary(writer, q.z);
}
inline void from_binary(BinaryReader& reader, MikanQuatd& q)
{
	from_binary(reader, q.w);
	from_binary(reader, q.x);
	from_binary(reader, q.y);
	from_binary(reader, q.z);
}

inline void to_binary(BinaryWriter& writer, const MikanTransform& t)
{
	to_binary(writer, t.scale);
	to_binary(writer, t.rotation);
	to_binary(writer, t.position);
}
inline void from_binary(BinaryReader& reader, MikanTransform& t)
{
	from_binary(reader, t.scale);
	from_binary(reader, t.rotation);
	from_binary(reader, t.position);
}