// This file is auto generated. DO NOT EDIT.

export const CLASS_ID_MIKAN_MATRIX3D = -1079171085240853505n;
export const CLASS_ID_MIKAN_MATRIX4D = -1078012199984908336n;
export const CLASS_ID_MIKAN_MATRIX4F = -1078010000961651914n;
export const CLASS_ID_MIKAN_MATRIX4X3D = -3432518494297948957n;
export const CLASS_ID_MIKAN_QUATD = 1243674317020394682n;
export const CLASS_ID_MIKAN_QUATF = 1243672117997138260n;
export const CLASS_ID_MIKAN_ROTATOR3F = 1923080865110129363n;
export const CLASS_ID_MIKAN_TRANSFORM = 1003008018171028503n;
export const CLASS_ID_MIKAN_VECTOR2D = 4125209798540890896n;
export const CLASS_ID_MIKAN_VECTOR2F = 4125211997564147318n;
export const CLASS_ID_MIKAN_VECTOR2I = 4125224092192057639n;
export const CLASS_ID_MIKAN_VECTOR3D = 4126307111145656249n;
export const CLASS_ID_MIKAN_VECTOR3F = 4126304912122399827n;
export const CLASS_ID_MIKAN_VECTOR4D = 4123226279563976702n;
export const CLASS_ID_MIKAN_VECTOR4F = 4123224080540720280n;

export interface MikanMatrix3d {
  x0: number;
  x1: number;
  x2: number;
  y0: number;
  y1: number;
  y2: number;
  z0: number;
  z1: number;
  z2: number;
}

export interface MikanMatrix4d {
  x0: number;
  x1: number;
  x2: number;
  x3: number;
  y0: number;
  y1: number;
  y2: number;
  y3: number;
  z0: number;
  z1: number;
  z2: number;
  z3: number;
  w0: number;
  w1: number;
  w2: number;
  w3: number;
}

export interface MikanMatrix4f {
  x0: number;
  x1: number;
  x2: number;
  x3: number;
  y0: number;
  y1: number;
  y2: number;
  y3: number;
  z0: number;
  z1: number;
  z2: number;
  z3: number;
  w0: number;
  w1: number;
  w2: number;
  w3: number;
}

export interface MikanMatrix4x3d {
  x0: number;
  x1: number;
  x2: number;
  x3: number;
  y0: number;
  y1: number;
  y2: number;
  y3: number;
  z0: number;
  z1: number;
  z2: number;
  z3: number;
}

export interface MikanQuatd {
  w: number;
  x: number;
  y: number;
  z: number;
}

export interface MikanQuatf {
  w: number;
  x: number;
  y: number;
  z: number;
}

export interface MikanRotator3f {
  x_angle: number;
  y_angle: number;
  z_angle: number;
}

export interface MikanTransform {
  scale: MikanVector3f;
  rotation: MikanQuatf;
  position: MikanVector3f;
}

export interface MikanVector2d {
  x: number;
  y: number;
}

export interface MikanVector2f {
  x: number;
  y: number;
}

export interface MikanVector2i {
  x: number;
  y: number;
}

export interface MikanVector3d {
  x: number;
  y: number;
  z: number;
}

export interface MikanVector3f {
  x: number;
  y: number;
  z: number;
}

export interface MikanVector4d {
  x: number;
  y: number;
  z: number;
  w: number;
}

export interface MikanVector4f {
  x: number;
  y: number;
  z: number;
  w: number;
}

