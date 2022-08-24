/**
\file
*/

#ifndef __MIKAN_MATH_TYPES_H
#define __MIKAN_MATH_TYPES_H
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
typedef struct
{
    double x, y, z;
} MikanVector3d;

/// A 3D vector with float components.
typedef struct
{
    float x, y, z;
} MikanVector3f;

/// A 3-tuple of Euler angles with float components.
typedef struct
{
	float x_angle, y_angle, z_angle;
} MikanRotator3f;

/** A 4x4 matrix with float components
    storage is column major order:

    | x0 y0 z0 px |
    | x1 y1 z1 py |
    | x2 y2 z2 pz |
    |  0  0  0  1 |
 */
typedef struct
{
    float m[4][4];
} MikanMatrix4f;

/** A 3x3 matrix with double components
    storage is column major order:

    | x0 y0 z0 |
    | x1 y1 z1 |
    | x2 y2 z2 |
 */
typedef struct
{
    double m[3][3];
} MikanMatrix3d;

/** A 4x3 matrix with double components
    storage is column major order:

    | x0 y0 z0 px|
    | x1 y1 z1 py|
    | x2 y2 z2 pz|
 */
typedef struct
{
    double m[4][3];
} MikanMatrix4x3d;

/** A 4x4 matrix with double components
    storage is column major order:

    | x0 y0 z0 px |
    | x1 y1 z1 py |
    | x2 y2 z2 pz |
    |  0  0  0 1 |
 */
typedef struct
{
    double m[4][4];
} MikanMatrix4d;

/// A quaternion rotation.
typedef struct
{
    double w, x, y, z;
} MikanQuatd;

/**
@}
*/

//cut_after
#endif
