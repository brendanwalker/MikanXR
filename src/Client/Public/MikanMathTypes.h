/**
\file
*/
#pragma once

//cut_before

/**
\brief Geometrical data structures used by the Client API
\defgroup Mikan Math Data Types
\addtogroup MikanMath
@{
*/

//-- constants -----
/// Conversion factor to go from meters to centimeters
#define MIKAN_METERS_TO_CENTIMETERS  100.f

/// Conversion factor to go from centimeters to meters
#define MIKAN_CENTIMETERS_TO_METERS  0.01f

/// A 3D vector with double components.
struct MikanVector3d
{
	double x, y, z;
};

/// A 2D vector with float components.
struct MikanVector2f
{
	float x, y;
};

/// A 3D vector with float components.
struct MikanVector3f
{
    float x, y, z;
};

/// A 3-tuple of Euler angles with float components.
struct MikanRotator3f
{
	float x_angle, y_angle, z_angle;
};

/** A 4x4 matrix with float components
    storage is column major order:

    | x0 y0 z0 w0 |
    | x1 y1 z1 w1 |
    | x2 y2 z2 w2 |
    | x3 y3 z3 w3 |
 */
struct MikanMatrix4f
{
	float x0;
	float x1;
	float x2;
	float x3;
	float y0;
	float y1;
	float y2;
	float y3;
	float z0;
	float z1;
	float z2;
	float z3;
	float w0;
	float w1;
	float w2;
	float w3;
};

/** A 3x3 matrix with double components
    storage is column major order:

    | x0 y0 z0 |
    | x1 y1 z1 |
    | x2 y2 z2 |
 */
struct MikanMatrix3d
{
	double x0;
	double x1;
	double x2;
	double y0;
	double y1;
	double y2;
	double z0;
	double z1;
	double z2;
};

/** A 4x3 matrix with double components
    storage is column major order:

    | x0 y0 z0 w0|
    | x1 y1 z1 w1|
    | x2 y2 z2 w2|
 */
struct MikanMatrix4x3d
{
	double x0;
	double x1;
	double x2;
	double x3;
	double y0;
	double y1;
	double y2;
	double y3;
	double z0;
	double z1;
	double z2;
	double z3;
};

/** A 4x4 matrix with double components
    storage is column major order:

    | x0 y0 z0 w0 |
    | x1 y1 z1 w1 |
    | x2 y2 z2 w2 |
    | x3 y3 z3 w3 |
 */
struct MikanMatrix4d
{
	double x0;
	double x1;
	double x2;
	double x3;
	double y0;
	double y1;
	double y2;
	double y3;
	double z0;
	double z1;
	double z2;
	double z3;
	double w0;
	double w1;
	double w2;
	double w3;
};

/// A double-precision quaternion rotation.
struct MikanQuatf
{
	float w, x, y, z;
};

/// A double-precision quaternion rotation.
struct MikanQuatd
{
    double w, x, y, z;
};

/// A single-precision Scale-Rotation-Translation transform.
struct MikanTransform
{
	MikanVector3f scale;
	MikanQuatf rotation;
	MikanVector3f position;
};

/**
@}
*/

//cut_after