/*
  bmath.hpp - v1.0 - public domain math library

  by Blat Blatnik

  NO WARRANTY IMPLIED - USE AT YOUR OWN RISK

  For licence information see end of file.

  Primarily of interest to game developers and others who only need
  2D, 3D, and 4D vector/matrix math and nothing more. This library 
  is intended to be a lightweight competitor to GLM. Unlike GLM this 
  library does not go out of its way to provide GLSL equivalent 
  functionality - only the commonly used stuff is provided, things like 
  sin(vec4) are not. Just like GLM, this is a header-only library.

  REQUIRES C++11 for exp2/log2 and constexpr.

  This library provides:
  + 2D, 3D and 4D vectors, 2x2, 3x3, 4x4 matrices, generic to any type
  + quaternions, generic to any type
  + full suite of vector operators (unary: + - ~ ! binary: + - * / % & | ^ << >>)
  + some matrix and quaternion operators (+ - * /)
  + most GLSL vector and matrix functions (but not all)
  + some color conversion functions
  + transform matrix building functions (perspective, translate, rotate, lookAt ..)
  + constexpr where possible

  This library does NOT provide:
  - non-square matrices
  - vectors of arbitrary size
  - "1D vectors"
  - dual quaternions
  - 16-bit floats
  - bit-twiddling math (bitCount, findLSB, bitfieldInsert)
  - trigonometric functions for vectors (sin, cos, ..)
  - packing function (packDouble2x32, ..)
  - arbitrary vector swizzles
  - complete set of operators for matrices and quaternions
  - functions which are FORCED to be inlined (with something like __forceinline for example)
  - SIMD optimization

  All types and most functions are implemented as templates in order to reduce
  code duplication, although typedefs are provided for float, double, int, uint,
  and bool vectors, float and double matrices and quaternions.

  Matrices are stored in a *COLUMN MAJOR* order to keep compatible with OpenGL.

  For view/projection matrix building functions, both a left-handed and a right-handed
  version are provided (*LH and *RH) so you can use whichever one suits your needs.

  By default, the projection matrix building functions assume that depth values
  outside of **[-1, 1]** are clipped - like they are in OpenGL. If you use DirectX or
  Vulkan, then you can 
  #define B_DEPTH_CLIP_ZERO_TO_ONE 
  before including this file to make the library produce projection matrices suitable 
  for clipping depth values outside of [0, 1].

  Also by default all types and functions are defined in the global namespace. You can
  #define B_MATH_NAMESPACE [some-namespace]   
  before including this file to place everything in a namespace of your choice.
*/

#pragma once
#ifndef B_MATH
#define B_MATH

#ifdef B_MATH_NAMESPACE
#define B_MATH_BEGIN namespace B_MATH_NAMESPACE {
#define B_MATH_END }
#else
#define B_MATH_BEGIN 
#define B_MATH_END
#endif

#include <cmath>

#if !defined(B_DEPTH_CLIP_ZERO_TO_ONE) && !defined(B_DEPTH_CLIP_MINUS_ONE_TO_ONE)
#	define B_DEPTH_CLIP_MINUS_ONE_TO_ONE
#endif

B_MATH_BEGIN

constexpr float  pi   = 3.141592741f;
constexpr double pi64 = 3.141592653589793;

/*
 * --- Struct Declarations ---
 */

template<class T, int N>        struct vector;
template<class T, int C, int R> struct matrix;
template<class T>               struct quaternion;

typedef unsigned int      uint;
typedef vector<float,  2> vec2;
typedef vector<float,  3> vec3;
typedef vector<float,  4> vec4;
typedef vector<double, 2> dvec2;
typedef vector<double, 3> dvec3;
typedef vector<double, 4> dvec4;
typedef vector<int,    2> ivec2;
typedef vector<int,    3> ivec3;
typedef vector<int,    4> ivec4;
typedef vector<uint,   2> uvec2;
typedef vector<uint,   3> uvec3;
typedef vector<uint,   4> uvec4;
typedef vector<bool,   2> bvec2;
typedef vector<bool,   3> bvec3;
typedef vector<bool,   4> bvec4;
typedef matrix<float,  2, 2> mat2;
typedef matrix<float,  3, 3> mat3;
typedef matrix<float,  4, 4> mat4;
typedef matrix<double, 2, 2> dmat2;
typedef matrix<double, 3, 3> dmat3;
typedef matrix<double, 4, 4> dmat4;
typedef quaternion<float>  quat;
typedef quaternion<double> dquat;

/*
 * --- Struct Definitions ---
 */

template<class T> 
struct vector<T, 2> {

	union {
		struct { T x, y; };
		struct { T r, g; };
		struct { T u, v; };
		T elem[2];
	};

	vector() = default;
	constexpr vector(const vector &v) = default;

	template<class T1, class T2> 
	inline constexpr vector(T1 x, T2 y)
		: x(T(x)), y(T(y)) {}

	template<class T1> 
	inline constexpr explicit vector(T1 xy)
		: vector(xy, xy) {}
	
	template<class T1> 
	inline constexpr explicit vector(vector<T1, 2> xy)
		: vector(xy.x, xy.y) {}
	
	template<class T1> 
	inline constexpr explicit vector(vector<T1, 3> xy)
		: vector(xy.x, xy.y) {}

	template<class T1> 
	inline constexpr explicit vector(vector<T1, 4> xy)
		: vector(xy.x, xy.y) {}

	inline T &operator[](int index) {
		return elem[index];
	}
	inline constexpr const T &operator[](int index) const {
		return elem[index];
	}
};

template<class T> 
struct vector<T, 3> {

	union {
		struct { T x, y, z; };
		struct { T r, g, b; };
		vector<T, 2> xy;
		vector<T, 2> rg;
		T elem[3];
	};

	vector() = default;
	constexpr vector(const vector &v) = default;

	template<class T1, class T2, class T3> 
	inline constexpr vector(T1 x, T2 y, T3 z)
		: x(T(x)), y(T(y)), z(T(z)) {}
	
	template<class T1, class T2> 
	inline constexpr vector(vector<T1, 2> xy, T2 z)
		: vector(xy.x, xy.y, z) {}
	
	template<class T1, class T2> 
	inline constexpr vector(T1 x, vector<T2, 2> yz)
		: vector(x, yz.x, yz.y) {}
	
	template<class T1> 
	inline constexpr explicit vector(T1 xyz)
		: vector(xyz, xyz, xyz) {}
	
	template<class T1> 
	inline constexpr explicit vector(vector<T1, 2> xy)
		: vector(xy.x, T(0)) {}
	
	template<class T1> 
	inline constexpr explicit vector(vector<T1, 3> xyz)
		: vector(xyz.x, xyz.y, xyz.z) {}
	
	template<class T1> 
	inline constexpr explicit vector(vector<T1, 4> xyz)
		: vector(xyz.x, xyz.y, xyz.z) {}

	inline T &operator[](int index) {
		return elem[index];
	}
	inline constexpr const T &operator[](int index) const {
		return elem[index];
	}
};

template<class T> 
struct vector<T, 4> {

	union {
		struct { T x, y, z, w; };
		struct { T r, g, b, a; };
		vector<T, 3> xyz;
		vector<T, 3> rgb;
		struct {
			vector<T, 2> xy;
			vector<T, 2> zw;
		};
		struct {
			vector<T, 2> rg;
			vector<T, 2> ba;
		};
		T elem[4];
	};

	vector() = default;
	constexpr vector(const vector &v) = default;

	template<class T1, class T2, class T3, class T4> 
	inline constexpr vector(T1 x, T2 y, T3 z, T4 w)
		: x(T(x)), y(T(y)), z(T(z)), w(T(w)) {}
	
	template<class T1, class T2, class T3> 
	inline constexpr vector(vector<T1, 2> xy, T2 z, T3 w)
		: vector(xy.x, z, w) {}
	
	template<class T1, class T2, class T3> 
	inline constexpr vector(T1 x, vector<T2, 2> yz, T3 w)
		: vector(x, yz.x, yz.y, w) {}
	
	template<class T1, class T2, class T3> 
	inline constexpr vector(T1 x, T2 y, vector<T3, 2> zw)
		: vector(x, y, zw.x, zw.y) {}
	
	template<class T1, class T2> 
	inline constexpr vector(vector<T1, 2> xy, vector<T2, 2> zw)
		: vector(xy.x, xy.y, zw.x, zw.y) {}
	
	template<class T1, class T2> 
	inline constexpr vector(vector<T1, 3> xyz, T2 w)
		: vector(xyz.x, xyz.y, xyz.z, w) {}
	
	template<class T1, class T2> 
	inline constexpr vector(T1 x, vector<T2, 3> yzw)
		: vector(x, yzw.x, yzw.y, yzw.z) {}
	
	template<class T1> 
	inline constexpr explicit vector(T1 xyzw)
		: vector(xyzw, xyzw, xyzw, xyzw) {}
	
	template<class T1> 
	inline constexpr explicit vector(vector<T1, 2> xy)
		: vector(xy.x, xy.y, T(0), T(0)) {}
	
	template<class T1> 
	inline constexpr explicit vector(vector<T1, 3> xyz)
		: vector(xyz.x, xyz.y, xyz.z, T(0)) {}
	
	template<class T1> 
	inline constexpr explicit vector(vector<T1, 4> xyzw)
		: vector(xyzw.x, xyzw.y, xyzw.z, xyzw.w) {}

	inline T &operator[](int index) {
		return elem[index];
	}
	inline constexpr const T &operator[](int index) const {
		return elem[index];
	}
};

template<class T> 
struct matrix<T, 2, 2> {

	vector<T, 2> col[2];

	matrix() = default;
	constexpr matrix(const matrix &m) = default;

	template<
		class X1, class Y1,
		class X2, class Y2>
	inline constexpr matrix(
		X1 x1, Y1 y1,
		X2 x2, Y2 y2)
		: col{
			vector<T, 2>(x1, y1),
			vector<T, 2>(x2, y2) } {}

	template<
		class T1,
		class T2>
	inline constexpr matrix(
		vector<T1, 2> col1,
		vector<T2, 2> col2)
		: col{ col1, col2 } {}

	template<class T1> 
	inline constexpr explicit matrix(matrix<T1, 2, 2> m)
		: matrix(m.col[0], m.col[1]) {}

	template<class T1> 
	inline constexpr explicit matrix(matrix<T1, 3, 3> m)
		: matrix(m.col[0].xy, m.col[1].xy) {}

	template<class T1> 
	inline constexpr explicit matrix(matrix<T1, 4, 4> m)
		: matrix(m.col[0].xy, m.col[1].xy) {}

	template<class T1> 
	inline constexpr explicit matrix(vector<T1, 2> diag)
		: matrix(
			diag.x, T( 0 ), 
			T( 0 ), diag.y) {}
	
	template<class T1> 
	inline constexpr explicit matrix(T1 diag)
		: matrix(
			diag, T(0),
			T(0), diag) {}

	inline vector<T, 2> &operator[](int index) {
		return col[index];
	}
	inline constexpr const vector<T, 2> &operator[](int index) const {
		return col[index];
	}
};

template<class T> 
struct matrix<T, 3, 3> {

	vector<T, 3> col[3];

	matrix() = default;
	constexpr matrix(const matrix &m) = default;

	template<
		class X1, class Y1, class Z1,
		class X2, class Y2, class Z2,
		class X3, class Y3, class Z3>
	inline constexpr matrix(
		X1 x1, Y1 y1, Z1 z1,
		X2 x2, Y2 y2, Z2 z2,
		X3 x3, Y3 y3, Z3 z3)
		: col{
			vector<T, 3>(x1, y1, z1),
			vector<T, 3>(x2, y2, z2),
			vector<T, 3>(x3, y3, z3)} {}

	template<
		class T1,
		class T2,
		class T3>
	inline constexpr matrix(
		vector<T1, 3> col1,
		vector<T2, 3> col2,
		vector<T3, 3> col3)
		: col{ 
			vector<T, 3>(col1), 
			vector<T, 3>(col2),
			vector<T, 3>(col3) } {}

	template<class T1> 
	inline constexpr explicit matrix(matrix<T1, 2, 2> m)
		: matrix(
			vector<T, 3>(m.col[0], T(0)),
			vector<T, 3>(m.col[1], T(0)),
			vector<T, 3>(T(0),T(0),T(1))) {}
	
	template<class T1> 
	inline constexpr explicit matrix(matrix<T1, 3, 3> m)
		: matrix(m.col[0], m.col[1], m.col[2]) {}

	
	template<class T1> 
	inline constexpr explicit matrix(matrix<T1, 4, 4> m)
		: matrix(m.col[0].xyz, m.col[1].xyz, m.col[2].xyz) {}
	
	template<class T1> 
	inline constexpr explicit matrix(vector<T1, 3> diag)
		: matrix(
			diag.x, T( 0 ), T( 0 ),
			T( 0 ), diag.y, T( 0 ),
			T( 0 ), T( 0 ), diag.z) {}
	
	template<class T1> 
	inline constexpr explicit matrix(T1 diag)
		: matrix(
			diag, T(0), T(0),
			T(0), diag, T(0),
			T(0), T(0), diag) {}

	inline vector<T, 3> &operator[](int index) {
		return col[index];
	}
	inline constexpr const vector<T, 3> &operator[](int index) const {
		return col[index];
	}
};

template<class T> 
struct matrix<T, 4, 4> {

	vector<T, 4> col[4];

	matrix() = default;
	constexpr matrix(const matrix &m) = default;
	
	template<
		class X1, class Y1, class Z1, class W1,
		class X2, class Y2, class Z2, class W2,
		class X3, class Y3, class Z3, class W3, 
		class X4, class Y4, class Z4, class W4>
	inline constexpr matrix(
		X1 x1, Y1 y1, Z1 z1, W1 w1,
		X2 x2, Y2 y2, Z2 z2, W2 w2,
		X3 x3, Y3 y3, Z3 z3, W3 w3,
		X4 x4, Y4 y4, Z4 z4, W4 w4)
		: col{
			vector<T, 4>(x1, y1, z1, w1),
			vector<T, 4>(x2, y2, z2, w2),
			vector<T, 4>(x3, y3, z3, w3),
			vector<T, 4>(x4, y4, z4, w4) } {}

	template<
		class T1,
		class T2,
		class T3,
		class T4>
	inline constexpr matrix(
		vector<T1, 4> col1,
		vector<T2, 4> col2,
		vector<T3, 4> col3,
		vector<T4, 4> col4)
		: col{ 
			vector<T, 4>(col1),
			vector<T, 4>(col2),
			vector<T, 4>(col3),
			vector<T, 4>(col4) } {}

	template<class T1> 
	inline constexpr explicit matrix(matrix<T1, 2, 2> m)
		: matrix(
			vector<T, 4>(m.col[0], T(0),T(0)),
			vector<T, 4>(m.col[1], T(0),T(0)),
			vector<T, 4>(T(0),T(0),T(1),T(0)),
			vector<T, 4>(T(0),T(0),T(0),T(1))) {}

	template<class T1> 
	inline constexpr explicit matrix(matrix<T1, 3, 3> m)
		: matrix(
			vector<T, 4>(m.col[0], T(0)),
			vector<T, 4>(m.col[1], T(0)),
			vector<T, 4>(m.col[2], T(0)),
			vector<T, 4>(T(0), T(0), T(0), T(1))) {}

	template<class T1> 
	inline constexpr explicit matrix(matrix<T1, 4, 4> m)
		: matrix(
			vector<T, 4>(m.col[0]),
			vector<T, 4>(m.col[1]),
			vector<T, 4>(m.col[2]),
			vector<T, 4>(m.col[3])) {}

	template<class T1> 
	inline constexpr explicit matrix(vector<T1, 4> diag)
		: matrix(
			diag.x, T( 0 ), T( 0 ), T( 0 ),
			T( 0 ), diag.y, T( 0 ), T( 0 ),
			T( 0 ), T( 0 ), diag.z, T( 0 ),
			T( 0 ), T( 0 ), T( 0 ), diag.w) {}

	template<class T1> 
	inline constexpr explicit matrix(T1 diag)
		: matrix(
			diag, T(0), T(0), T(0),
			T(0), diag, T(0), T(0),
			T(0), T(0), diag, T(0),
			T(0), T(0), T(0), diag) {}

	inline vector<T, 4> &operator[](int index) {
		return col[index];
	}
	inline constexpr const vector<T, 4> &operator[](int index) const {
		return col[index];
	}
};

template<class T> 
struct quaternion {

	union {
		struct { T x, y, z, w; };
		vector<T, 3> xyz;
		vector<T, 4> xyzw;
		T elem[4];
	};

	quaternion() = default;
	constexpr quaternion(const quaternion &q) = default;

	template<class T1, class T2, class T3, class T4> 
	inline constexpr quaternion(T1 x, T2 y, T3 z, T4 w)
		: x(T(x)), y(T(y)), z(T(z)), w(T(w)) {}
	
	template<class T1, class T2> 
	inline constexpr quaternion(vector<T1, 3> xyz, T2 w)
		: quaternion(xyz.x, xyz.y, xyz.z, w) {}
	
	template<class T1> 
	inline constexpr explicit quaternion(vector<T1, 4> xyzw)
		: quaternion(xyzw.x, xyzw.y, xyzw.z, xyzw.w) {}
	
	template<class T1> 
	inline constexpr explicit quaternion(quaternion<T1> q)
		: quaternion(q.x, q.y, q.z, q.w) {}

	inline T &operator[](int index) {
		return elem[index];
	}
	inline constexpr const T &operator[](int index) const {
		return elem[index];
	}
};

/*
 * --- Vector Operators ---
 */

template<class T>
inline constexpr vector<T, 2> operator +(vector<T, 2> v) {
	return vector<T, 2>(+v.x, +v.y);
}

template<class T>
inline constexpr vector<T, 3> operator +(vector<T, 3> v) {
	return vector<T, 3>(+v.x, +v.y, +v.z);
}

template<class T>
inline constexpr vector<T, 4> operator +(vector<T, 4> v) {
	return vector<T, 4>(+v.x, +v.y, +v.z, +v.w);
}

template<class T>
inline constexpr vector<T, 2> operator -(vector<T, 2> v) {
	return vector<T, 2>(-v.x, -v.y);
}

template<class T>
inline constexpr vector<T, 3> operator -(vector<T, 3> v) {
	return vector<T, 3>(-v.x, -v.y, -v.z);
}

template<class T>
inline constexpr vector<T, 4> operator -(vector<T, 4> v) {
	return vector<T, 4>(-v.x, -v.y, -v.z, -v.w);
}

template<class T>
inline constexpr vector<T, 2> operator ~(vector<T, 2> v) {
	return vector<T, 2>(~v.x, ~v.y);
}

template<class T>
inline constexpr vector<T, 3> operator ~(vector<T, 3> v) {
	return vector<T, 3>(~v.x, ~v.y, ~v.z);
}

template<class T>
inline constexpr vector<T, 4> operator ~(vector<T, 4> v) {
	return vector<T, 4>(~v.x, ~v.y, ~v.z, ~v.w);
}

template<class T>
inline constexpr vector<bool, 2> operator !(vector<T, 2> v) {
	return vector<bool, 2>(!v.x, !v.y);
}

template<class T>
inline constexpr vector<bool, 3> operator !(vector<T, 3> v) {
	return vector<bool, 3>(!v.x, !v.y, !v.z);
}

template<class T>
inline constexpr vector<bool, 4> operator !(vector<T, 4> v) {
	return vector<bool, 4>(!v.x, !v.y, !v.z, !v.w);
}

template<class T>
inline constexpr vector<T, 2> operator +(vector<T, 2> left, vector<T, 2> right) {
	return vector<T, 2>(
		left.x + right.x,
		left.y + right.y);
}

template<class T>
inline constexpr vector<T, 3> operator +(vector<T, 3> left, vector<T, 3> right) {
	return vector<T, 3>(
		left.x + right.x,
		left.y + right.y,
		left.z + right.z);
}

template<class T>
inline constexpr vector<T, 4> operator +(vector<T, 4> left, vector<T, 4> right) {
	return vector<T, 4>(
		left.x + right.x,
		left.y + right.y,
		left.z + right.z,
		left.w + right.w);
}

template<class T>
inline constexpr vector<T, 2> operator -(vector<T, 2> left, vector<T, 2> right) {
	return vector<T, 2>(
		left.x - right.x,
		left.y - right.y);
}

template<class T>
inline constexpr vector<T, 3> operator -(vector<T, 3> left, vector<T, 3> right) {
	return vector<T, 3>(
		left.x - right.x,
		left.y - right.y,
		left.z - right.z);
}

template<class T>
inline constexpr vector<T, 4> operator -(vector<T, 4> left, vector<T, 4> right) {
	return vector<T, 4>(
		left.x - right.x,
		left.y - right.y,
		left.z - right.z,
		left.w - right.w);
}

template<class T>
inline constexpr vector<T, 2> operator *(vector<T, 2> left, vector<T, 2> right) {
	return vector<T, 2>(
		left.x * right.x,
		left.y * right.y);
}

template<class T>
inline constexpr vector<T, 3> operator *(vector<T, 3> left, vector<T, 3> right) {
	return vector<T, 3>(
		left.x * right.x,
		left.y * right.y,
		left.z * right.z);
}

template<class T>
inline constexpr vector<T, 4> operator *(vector<T, 4> left, vector<T, 4> right) {
	return vector<T, 4>(
		left.x * right.x,
		left.y * right.y,
		left.z * right.z,
		left.w * right.w);
}

template<class T>
inline constexpr vector<T, 2> operator /(vector<T, 2> left, vector<T, 2> right) {
	return vector<T, 2>(
		left.x / right.x,
		left.y / right.y);
}

template<class T>
inline constexpr vector<T, 3> operator /(vector<T, 3> left, vector<T, 3> right) {
	return vector<T, 3>(
		left.x / right.x,
		left.y / right.y,
		left.z / right.z);
}

template<class T>
inline constexpr vector<T, 4> operator /(vector<T, 4> left, vector<T, 4> right) {
	return vector<T, 4>(
		left.x / right.x,
		left.y / right.y,
		left.z / right.z,
		left.w / right.w);
}

template<class T>
inline constexpr vector<T, 2> operator %(vector<T, 2> left, vector<T, 2> right) {
	return vector<T, 2>(
		left.x % right.x,
		left.y % right.y);
}

template<class T>
inline constexpr vector<T, 3> operator %(vector<T, 3> left, vector<T, 3> right) {
	return vector<T, 3>(
		left.x % right.x,
		left.y % right.y,
		left.z % right.z);
}

template<class T>
inline constexpr vector<T, 4> operator %(vector<T, 4> left, vector<T, 4> right) {
	return vector<T, 4>(
		left.x % right.x,
		left.y % right.y,
		left.z % right.z,
		left.w % right.w);
}

template<class T>
inline constexpr vector<T, 2> operator &(vector<T, 2> left, vector<T, 2> right) {
	return vector<T, 2>(
		left.x & right.x,
		left.y & right.y);
}

template<class T>
inline constexpr vector<T, 3> operator &(vector<T, 3> left, vector<T, 3> right) {
	return vector<T, 3>(
		left.x & right.x,
		left.y & right.y,
		left.z & right.z);
}

template<class T>
inline constexpr vector<T, 4> operator &(vector<T, 4> left, vector<T, 4> right) {
	return vector<T, 4>(
		left.x & right.x,
		left.y & right.y,
		left.z & right.z,
		left.w & right.w);
}

template<class T>
inline constexpr vector<T, 2> operator |(vector<T, 2> left, vector<T, 2> right) {
	return vector<T, 2>(
		left.x | right.x,
		left.y | right.y);
}

template<class T>
inline constexpr vector<T, 3> operator |(vector<T, 3> left, vector<T, 3> right) {
	return vector<T, 3>(
		left.x | right.x,
		left.y | right.y,
		left.z | right.z);
}

template<class T>
inline constexpr vector<T, 4> operator |(vector<T, 4> left, vector<T, 4> right) {
	return vector<T, 4>(
		left.x | right.x,
		left.y | right.y,
		left.z | right.z,
		left.w | right.w);
}

template<class T>
inline constexpr vector<T, 2> operator ^(vector<T, 2> left, vector<T, 2> right) {
	return vector<T, 2>(
		left.x ^ right.x,
		left.y ^ right.y);
}

template<class T>
inline constexpr vector<T, 3> operator ^(vector<T, 3> left, vector<T, 3> right) {
	return vector<T, 3>(
		left.x ^ right.x,
		left.y ^ right.y,
		left.z ^ right.z);
}

template<class T>
inline constexpr vector<T, 4> operator ^(vector<T, 4> left, vector<T, 4> right) {
	return vector<T, 4>(
		left.x ^ right.x,
		left.y ^ right.y,
		left.z ^ right.z,
		left.w ^ right.w);
}

template<class T>
inline constexpr vector<T, 2> operator <<(vector<T, 2> left, vector<T, 2> right) {
	return vector<T, 2>(
		left.x << right.x,
		left.y << right.y);
}

template<class T>
inline constexpr vector<T, 3> operator <<(vector<T, 3> left, vector<T, 3> right) {
	return vector<T, 3>(
		left.x << right.x,
		left.y << right.y,
		left.z << right.z);
}

template<class T>
inline constexpr vector<T, 4> operator <<(vector<T, 4> left, vector<T, 4> right) {
	return vector<T, 4>(
		left.x << right.x,
		left.y << right.y,
		left.z << right.z,
		left.w << right.w);
}

template<class T>
inline constexpr vector<T, 2> operator >>(vector<T, 2> left, vector<T, 2> right) {
	return vector<T, 2>(
		left.x >> right.x,
		left.y >> right.y);
}

template<class T>
inline constexpr vector<T, 3> operator >>(vector<T, 3> left, vector<T, 3> right) {
	return vector<T, 3>(
		left.x >> right.x,
		left.y >> right.y,
		left.z >> right.z);
}

template<class T>
inline constexpr vector<T, 4> operator >>(vector<T, 4> left, vector<T, 4> right) {
	return vector<T, 4>(
		left.x >> right.x,
		left.y >> right.y,
		left.z >> right.z,
		left.w >> right.w);
}

template<class T>
inline constexpr vector<bool, 2> operator ==(vector<T, 2> left, vector<T, 2> right) {
	return vector<bool, 2>(
		left.x == right.x,
		left.y == right.y);
}

template<class T>
inline constexpr vector<bool, 3> operator ==(vector<T, 3> left, vector<T, 3> right) {
	return vector<bool, 3>(
		left.x == right.x,
		left.y == right.y,
		left.z == right.z);
}

template<class T>
inline constexpr vector<bool, 4> operator ==(vector<T, 4> left, vector<T, 4> right) {
	return vector<bool, 4>(
		left.x == right.x,
		left.y == right.y,
		left.z == right.z,
		left.w == right.w);
}

template<class T>
inline constexpr vector<bool, 2> operator !=(vector<T, 2> left, vector<T, 2> right) {
	return vector<bool, 2>(
		left.x != right.x,
		left.y != right.y);
}

template<class T>
inline constexpr vector<bool, 3> operator !=(vector<T, 3> left, vector<T, 3> right) {
	return vector<bool, 3>(
		left.x != right.x,
		left.y != right.y,
		left.z != right.z);
}

template<class T>
inline constexpr vector<bool, 4> operator !=(vector<T, 4> left, vector<T, 4> right) {
	return vector<bool, 4>(
		left.x != right.x,
		left.y != right.y,
		left.z != right.z,
		left.w != right.w);
}

template<class T>
inline constexpr vector<bool, 2> operator >=(vector<T, 2> left, vector<T, 2> right) {
	return vector<bool, 2>(
		left.x >= right.x,
		left.y >= right.y);
}

template<class T>
inline constexpr vector<bool, 3> operator >=(vector<T, 3> left, vector<T, 3> right) {
	return vector<bool, 3>(
		left.x >= right.x,
		left.y >= right.y,
		left.z >= right.z);
}

template<class T>
inline constexpr vector<bool, 4> operator >=(vector<T, 4> left, vector<T, 4> right) {
	return vector<bool, 4>(
		left.x >= right.x,
		left.y >= right.y,
		left.z >= right.z,
		left.w >= right.w);
}

template<class T>
inline constexpr vector<bool, 2> operator <=(vector<T, 2> left, vector<T, 2> right) {
	return vector<bool, 2>(
		left.x <= right.x,
		left.y <= right.y);
}

template<class T>
inline constexpr vector<bool, 3> operator <=(vector<T, 3> left, vector<T, 3> right) {
	return vector<bool, 3>(
		left.x <= right.x,
		left.y <= right.y,
		left.z <= right.z);
}

template<class T>
inline constexpr vector<bool, 4> operator <=(vector<T, 4> left, vector<T, 4> right) {
	return vector<bool, 4>(
		left.x <= right.x,
		left.y <= right.y,
		left.z <= right.z,
		left.w <= right.w);
}

template<class T>
inline constexpr vector<bool, 2> operator >(vector<T, 2> left, vector<T, 2> right) {
	return vector<bool, 2>(
		left.x > right.x,
		left.y > right.y);
}

template<class T>
inline constexpr vector<bool, 3> operator >(vector<T, 3> left, vector<T, 3> right) {
	return vector<bool, 3>(
		left.x > right.x,
		left.y > right.y,
		left.z > right.z);
}

template<class T>
inline constexpr vector<bool, 4> operator >(vector<T, 4> left, vector<T, 4> right) {
	return vector<bool, 4>(
		left.x > right.x,
		left.y > right.y,
		left.z > right.z,
		left.w > right.w);
}

template<class T>
inline constexpr vector<bool, 2> operator <(vector<T, 2> left, vector<T, 2> right) {
	return vector<bool, 2>(
		left.x < right.x,
		left.y < right.y);
}

template<class T>
inline constexpr vector<bool, 3> operator <(vector<T, 3> left, vector<T, 3> right) {
	return vector<bool, 3>(
		left.x < right.x,
		left.y < right.y,
		left.z < right.z);
}

template<class T>
inline constexpr vector<bool, 4> operator <(vector<T, 4> left, vector<T, 4> right) {
	return vector<bool, 4>(
		left.x < right.x,
		left.y < right.y,
		left.z < right.z,
		left.w < right.w);
}

template<class T, int N>
inline constexpr vector<T, N> operator +(vector<T, N> left, T right) {
	return left + vector<T, N>(right);
}

template<class T, int N>
inline constexpr vector<T, N> operator -(vector<T, N> left, T right) {
	return left - vector<T, N>(right);
}

template<class T, int N>
inline constexpr vector<T, N> operator *(vector<T, N> left, T right) {
	return left * vector<T, N>(right);
}

template<class T, int N>
inline constexpr vector<T, N> operator /(vector<T, N> left, T right) {
	return left / vector<T, N>(right);
}

template<class T, int N>
inline constexpr vector<T, N> operator %(vector<T, N> left, T right) {
	return left % vector<T, N>(right);
}

template<class T, int N>
inline constexpr vector<T, N> operator &(vector<T, N> left, T right) {
	return left & vector<T, N>(right);
}

template<class T, int N>
inline constexpr vector<T, N> operator |(vector<T, N> left, T right) {
	return left | vector<T, N>(right);
}

template<class T, int N>
inline constexpr vector<T, N> operator ^(vector<T, N> left, T right) {
	return left ^ vector<T, N>(right);
}

template<class T, int N>
inline constexpr vector<T, N> operator <<(vector<T, N> left, T right) {
	return left << vector<T, N>(right);
}

template<class T, int N>
inline constexpr vector<T, N> operator >>(vector<T, N> left, T right) {
	return left >> vector<T, N>(right);
}

template<class T, int N>
inline constexpr vector<bool, N> operator ==(vector<T, N> left, T right) {
	return left == vector<T, N>(right);
}

template<class T, int N>
inline constexpr vector<bool, N> operator !=(vector<T, N> left, T right) {
	return left != vector<T, N>(right);
}

template<class T, int N>
inline constexpr vector<bool, N> operator >=(vector<T, N> left, T right) {
	return left >= vector<T, N>(right);
}

template<class T, int N>
inline constexpr vector<bool, N> operator <=(vector<T, N> left, T right) {
	return left <= vector<T, N>(right);
}

template<class T, int N>
inline constexpr vector<bool, N> operator >(vector<T, N> left, T right) {
	return left > vector<T, N>(right);
}

template<class T, int N>
inline constexpr vector<bool, N> operator <(vector<T, N> left, T right) {
	return left < vector<T, N>(right);
}

template<class T, int N>
inline constexpr vector<T, N> operator +(T left, vector<T, N> right) {
	return vector<T, N>(left) + right;
}

template<class T, int N>
inline constexpr vector<T, N> operator -(T left, vector<T, N> right) {
	return vector<T, N>(left) - right;
}

template<class T, int N>
inline constexpr vector<T, N> operator *(T left, vector<T, N> right) {
	return vector<T, N>(left) * right;
}

template<class T, int N>
inline constexpr vector<T, N> operator /(T left, vector<T, N> right) {
	return vector<T, N>(left) / right;
}

template<class T, int N>
inline constexpr vector<T, N> operator %(T left, vector<T, N> right) {
	return vector<T, N>(left) % right;
}

template<class T, int N>
inline constexpr vector<T, N> operator &(T left, vector<T, N> right) {
	return vector<T, N>(left) & right;
}

template<class T, int N>
inline constexpr vector<T, N> operator |(T left, vector<T, N> right) {
	return vector<T, N>(left) | right;
}

template<class T, int N>
inline constexpr vector<T, N> operator ^(T left, vector<T, N> right) {
	return vector<T, N>(left) ^ right;
}

template<class T, int N>
inline constexpr vector<T, N> operator <<(T left, vector<T, N> right) {
	return vector<T, N>(left) << right;
}

template<class T, int N>
inline constexpr vector<T, N> operator >>(T left, vector<T, N> right) {
	return vector<T, N>(left) >> right;
}

template<class T, int N>
inline constexpr vector<bool, N> operator ==(T left, vector<T, N> right) {
	return vector<T, N>(left) == right;
}

template<class T, int N>
inline constexpr vector<bool, N> operator !=(T left, vector<T, N> right) {
	return vector<T, N>(left) != right;
}

template<class T, int N>
inline constexpr vector<bool, N> operator >=(T left, vector<T, N> right) {
	return vector<T, N>(left) >= right;
}

template<class T, int N>
inline constexpr vector<bool, N> operator <=(T left, vector<T, N> right) {
	return vector<T, N>(left) <= right;
}

template<class T, int N>
inline constexpr vector<bool, N> operator >(T left, vector<T, N> right) {
	return vector<T, N>(left) > right;
}

template<class T, int N>
inline constexpr vector<bool, N> operator <(T left, vector<T, N> right) {
	return vector<T, N>(left) < right;
}

template<class T, int N>
inline vector<T, N> &operator +=(vector<T, N> &left, vector<T, N> right) {
	return left = left + right;
}

template<class T, int N>
inline vector<T, N> &operator -=(vector<T, N> &left, vector<T, N> right) {
	return left = left - right;
}

template<class T, int N>
inline vector<T, N> &operator *=(vector<T, N> &left, vector<T, N> right) {
	return left = left * right;
}

template<class T, int N>
inline vector<T, N> &operator /=(vector<T, N> &left, vector<T, N> right) {
	return left = left / right;
}

template<class T, int N>
inline vector<T, N> &operator %=(vector<T, N> &left, vector<T, N> right) {
	return left = left % right;
}

template<class T, int N>
inline vector<T, N> &operator &=(vector<T, N> &left, vector<T, N> right) {
	return left = left & right;
}

template<class T, int N>
inline vector<T, N> &operator |=(vector<T, N> &left, vector<T, N> right) {
	return left = left | right;
}

template<class T, int N>
inline vector<T, N> &operator ^=(vector<T, N> &left, vector<T, N> right) {
	return left = left ^ right;
}

template<class T, int N>
inline vector<T, N> &operator <<=(vector<T, N> &left, vector<T, N> right) {
	return left = left << right;
}

template<class T, int N>
inline vector<T, N> &operator >>=(vector<T, N> &left, vector<T, N> right) {
	return left = left >> right;
}

template<class T, int N>
inline vector<T, N> &operator +=(vector<T, N> &left, T right) {
	return left = left + right;
}

template<class T, int N>
inline vector<T, N> &operator -=(vector<T, N> &left, T right) {
	return left = left - right;
}

template<class T, int N>
inline vector<T, N> &operator *=(vector<T, N> &left, T right) {
	return left = left * right;
}

template<class T, int N>
inline vector<T, N> &operator /=(vector<T, N> &left, T right) {
	return left = left / right;
}

template<class T, int N>
inline vector<T, N> &operator %=(vector<T, N> &left, T right) {
	return left = left % right;
}

template<class T, int N>
inline vector<T, N> &operator &=(vector<T, N> &left, T right) {
	return left = left & right;
}

template<class T, int N>
inline vector<T, N> &operator |=(vector<T, N> &left, T right) {
	return left = left | right;
}

template<class T, int N>
inline vector<T, N> &operator ^=(vector<T, N> &left, T right) {
	return left = left ^ right;
}

template<class T, int N>
inline vector<T, N> &operator <<=(vector<T, N> &left, T right) {
	return left = left << right;
}

template<class T, int N>
inline vector<T, N> &operator >>=(vector<T, N> &left, T right) {
	return left = left >> right;
}

template<class T, int N>
inline vector<T, N> operator ++(vector<T, N> &v) {
	return v += T(1);
}

template<class T, int N>
inline vector<T, N> operator ++(vector<T, N> &v, int) {
	vector<T, N> copy = v;
	v += T(1);
	return copy;
}

/*
 * --- Matrix Operators ---
 */

template<class T>
inline constexpr matrix<T, 2, 2> operator +(matrix<T, 2, 2> m) {
	return matrix<T, 2, 2>(
		+m.col[0],
		+m.col[1]);
}

template<class T>
inline constexpr matrix<T, 3, 3> operator +(matrix<T, 3, 3> m) {
	return matrix<T, 3, 3>(
		+m.col[0],
		+m.col[1],
		+m.col[2]);
}

template<class T>
inline constexpr matrix<T, 4, 4> operator +(matrix<T, 4, 4> m) {
	return matrix<T, 4, 4>(
		+m.col[0],
		+m.col[1],
		+m.col[2],
		+m.col[3]);
}

template<class T>
inline constexpr matrix<T, 2, 2> operator -(matrix<T, 2, 2> m) {
	return matrix<T, 2, 2>(
		-m.col[0],
		-m.col[1]);
}

template<class T>
inline constexpr matrix<T, 3, 3> operator -(matrix<T, 3, 3> m) {
	return matrix<T, 3, 3>(
		-m.col[0],
		-m.col[1],
		-m.col[2]);
}

template<class T>
inline constexpr matrix<T, 4, 4> operator -(matrix<T, 4, 4> m) {
	return matrix<T, 4, 4>(
		-m.col[0],
		-m.col[1],
		-m.col[2],
		-m.col[3]);
}

template<class T>
inline constexpr matrix<T, 2, 2> operator +(matrix<T, 2, 2> left, matrix<T, 2, 2> right) {
	return matrix<T, 2, 2>(
		left.col[0] + right.col[0],
		left.col[1] + right.col[1]);
}

template<class T>
inline constexpr matrix<T, 3, 3> operator +(matrix<T, 3, 3> left, matrix<T, 3, 3> right) {
	return matrix<T, 3, 3>(
		left.col[0] + right.col[0],
		left.col[1] + right.col[1],
		left.col[2] + right.col[2]);
}

template<class T>
inline constexpr matrix<T, 4, 4> operator +(matrix<T, 4, 4> left, matrix<T, 4, 4> right) {
	return matrix<T, 4, 4>(
		left.col[0] + right.col[0],
		left.col[1] + right.col[1],
		left.col[2] + right.col[2],
		left.col[3] + right.col[3]);
}

template<class T>
inline constexpr matrix<T, 2, 2> operator -(matrix<T, 2, 2> left, matrix<T, 2, 2> right) {
	return matrix<T, 2, 2>(
		left.col[0] - right.col[0],
		left.col[1] - right.col[1]);
}

template<class T>
inline constexpr matrix<T, 3, 3> operator -(matrix<T, 3, 3> left, matrix<T, 3, 3> right) {
	return matrix<T, 3, 3>(
		left.col[0] - right.col[0],
		left.col[1] - right.col[1],
		left.col[2] - right.col[2]);
}

template<class T>
inline constexpr matrix<T, 4, 4> operator -(matrix<T, 4, 4> left, matrix<T, 4, 4> right) {
	return matrix<T, 4, 4>(
		left.col[0] - right.col[0],
		left.col[1] - right.col[1],
		left.col[2] - right.col[2],
		left.col[3] - right.col[3]);
}

template<class T>
inline constexpr matrix<T, 2, 2> operator *(matrix<T, 2, 2> left, matrix<T, 2, 2> right) {
	return matrix<T, 2, 2>(

		left.col[0].x * right.col[0].x + left.col[1].x * right.col[0].y,
		left.col[0].y * right.col[0].x + left.col[1].y * right.col[0].y,

		left.col[0].x * right.col[1].x + left.col[1].x * right.col[1].y,
		left.col[0].y * right.col[1].x + left.col[1].y * right.col[1].y);
}

template<class T>
inline constexpr matrix<T, 3, 3> operator *(matrix<T, 3, 3> left, matrix<T, 3, 3> right) {
	return matrix<T, 3, 3>(

		left.col[0].x * right.col[0].x + left.col[1].x * right.col[0].y + left.col[2].x * right.col[0].z,
		left.col[0].y * right.col[0].x + left.col[1].y * right.col[0].y + left.col[2].y * right.col[0].z,
		left.col[0].z * right.col[0].x + left.col[1].z * right.col[0].y + left.col[2].z * right.col[0].z,

		left.col[0].x * right.col[1].x + left.col[1].x * right.col[1].y + left.col[2].x * right.col[1].z,
		left.col[0].y * right.col[1].x + left.col[1].y * right.col[1].y + left.col[2].y * right.col[1].z,
		left.col[0].z * right.col[1].x + left.col[1].z * right.col[1].y + left.col[2].z * right.col[1].z, 
		
		left.col[0].x * right.col[2].x + left.col[1].x * right.col[2].y + left.col[2].x * right.col[2].z,
		left.col[0].y * right.col[2].x + left.col[1].y * right.col[2].y + left.col[2].y * right.col[2].z,
		left.col[0].z * right.col[2].x + left.col[1].z * right.col[2].y + left.col[2].z * right.col[2].z);
}

template<class T>
inline constexpr matrix<T, 4, 4> operator *(matrix<T, 4, 4> left, matrix<T, 4, 4> right) {
	return matrix<T, 4, 4>(

		left.col[0].x * right.col[0].x + left.col[1].x * right.col[0].y + left.col[2].x * right.col[0].z + left.col[3].x * right.col[0].w,
		left.col[0].y * right.col[0].x + left.col[1].y * right.col[0].y + left.col[2].y * right.col[0].z + left.col[3].y * right.col[0].w,
		left.col[0].z * right.col[0].x + left.col[1].z * right.col[0].y + left.col[2].z * right.col[0].z + left.col[3].z * right.col[0].w,
		left.col[0].w * right.col[0].x + left.col[1].w * right.col[0].y + left.col[2].w * right.col[0].z + left.col[3].w * right.col[0].w,
		
		left.col[0].x * right.col[1].x + left.col[1].x * right.col[1].y + left.col[2].x * right.col[1].z + left.col[3].x * right.col[1].w,
		left.col[0].y * right.col[1].x + left.col[1].y * right.col[1].y + left.col[2].y * right.col[1].z + left.col[3].y * right.col[1].w,
		left.col[0].z * right.col[1].x + left.col[1].z * right.col[1].y + left.col[2].z * right.col[1].z + left.col[3].z * right.col[1].w,
		left.col[0].w * right.col[1].x + left.col[1].w * right.col[1].y + left.col[2].w * right.col[1].z + left.col[3].w * right.col[1].w,

		left.col[0].x * right.col[2].x + left.col[1].x * right.col[2].y + left.col[2].x * right.col[2].z + left.col[3].x * right.col[2].w,
		left.col[0].y * right.col[2].x + left.col[1].y * right.col[2].y + left.col[2].y * right.col[2].z + left.col[3].y * right.col[2].w,
		left.col[0].z * right.col[2].x + left.col[1].z * right.col[2].y + left.col[2].z * right.col[2].z + left.col[3].z * right.col[2].w,
		left.col[0].w * right.col[2].x + left.col[1].w * right.col[2].y + left.col[2].w * right.col[2].z + left.col[3].w * right.col[2].w,

		left.col[0].x * right.col[3].x + left.col[1].x * right.col[3].y + left.col[2].x * right.col[3].z + left.col[3].x * right.col[3].w,
		left.col[0].y * right.col[3].x + left.col[1].y * right.col[3].y + left.col[2].y * right.col[3].z + left.col[3].y * right.col[3].w,
		left.col[0].z * right.col[3].x + left.col[1].z * right.col[3].y + left.col[2].z * right.col[3].z + left.col[3].z * right.col[3].w,
		left.col[0].w * right.col[3].x + left.col[1].w * right.col[3].y + left.col[2].w * right.col[3].z + left.col[3].w * right.col[3].w);
}

template<class T>
inline constexpr vector<T, 2> operator *(matrix<T, 2, 2> left, vector<T, 2> right) {
	return vector<T, 2>(
		left.col[0].x * right.x + left.col[1].x * right.y,
		left.col[0].y * right.x + left.col[1].y * right.y);
}

template<class T>
inline constexpr vector<T, 3> operator *(matrix<T, 3, 3> left, vector<T, 3> right) {
	return vector<T, 3>(
		left.col[0].x * right.x + left.col[1].x * right.y + left.col[2].x * right.z,
		left.col[0].y * right.x + left.col[1].y * right.y + left.col[2].y * right.z,
		left.col[0].z * right.x + left.col[1].z * right.y + left.col[2].z * right.z);
}

template<class T>
inline constexpr vector<T, 4> operator *(matrix<T, 4, 4> left, vector<T, 4> right) {
	return vector<T, 4>(
		left.col[0].x * right.x + left.col[1].x * right.y + left.col[2].x * right.z + left.col[3].x * right.w,
		left.col[0].y * right.x + left.col[1].y * right.y + left.col[2].y * right.z + left.col[3].y * right.w,
		left.col[0].z * right.x + left.col[1].z * right.y + left.col[2].z * right.z + left.col[3].z * right.w,
		left.col[0].w * right.x + left.col[1].w * right.y + left.col[2].w * right.z + left.col[3].w * right.w);
}

template<class T>
inline constexpr matrix<T, 2, 2> operator /(matrix<T, 2, 2> left, matrix<T, 2, 2> right) {
	return matrix<T, 2, 2>(
		left.col[0] / right.col[0],
		left.col[1] / right.col[1]);
}

template<class T>
inline constexpr matrix<T, 3, 3> operator /(matrix<T, 3, 3> left, matrix<T, 3, 3> right) {
	return matrix<T, 3, 3>(
		left.col[0] / right.col[0],
		left.col[1] / right.col[1],
		left.col[2] / right.col[2]);
}

template<class T>
inline constexpr matrix<T, 4, 4> operator /(matrix<T, 4, 4> left, matrix<T, 4, 4> right) {
	return matrix<T, 4, 4>(
		left.col[0] / right.col[0],
		left.col[1] / right.col[1],
		left.col[2] / right.col[2],
		left.col[3] / right.col[3]);
}

template<class T>
inline constexpr matrix<T, 2, 2> operator +(matrix<T, 2, 2> left, T right) {
	return left + matrix<T, 2, 2>(
		vector<T, 2>(right),
		vector<T, 2>(right));
}

template<class T>
inline constexpr matrix<T, 3, 3> operator +(matrix<T, 3, 3> left, T right) {
	return left + matrix<T, 3, 3>(
		vector<T, 3>(right),
		vector<T, 3>(right),
		vector<T, 3>(right));
}

template<class T>
inline constexpr matrix<T, 4, 4> operator +(matrix<T, 4, 4> left, T right) {
	return left + matrix<T, 4, 4>(
		vector<T, 4>(right),
		vector<T, 4>(right),
		vector<T, 4>(right),
		vector<T, 4>(right));
}

template<class T>
inline constexpr matrix<T, 2, 2> operator -(matrix<T, 2, 2> left, T right) {
	return left - matrix<T, 2, 2>(
		vector<T, 2>(right),
		vector<T, 2>(right));
}

template<class T>
inline constexpr matrix<T, 3, 3> operator -(matrix<T, 3, 3> left, T right) {
	return left - matrix<T, 3, 3>(
		vector<T, 3>(right),
		vector<T, 3>(right),
		vector<T, 3>(right));
}

template<class T>
inline constexpr matrix<T, 4, 4> operator -(matrix<T, 4, 4> left, T right) {
	return left - matrix<T, 4, 4>(
		vector<T, 4>(right),
		vector<T, 4>(right),
		vector<T, 4>(right),
		vector<T, 4>(right));
}

template<class T>
inline constexpr matrix<T, 2, 2> operator *(matrix<T, 2, 2> left, T right) {
	return matrix<T, 2, 2>(
		left.col[0] * vector<T, 2>(right),
		left.col[1] * vector<T, 2>(right));
}

template<class T>
inline constexpr matrix<T, 3, 3> operator *(matrix<T, 3, 3> left, T right) {
	return matrix<T, 3, 3>(
		left.col[0] * vector<T, 3>(right),
		left.col[1] * vector<T, 3>(right),
		left.col[2] * vector<T, 3>(right));
}

template<class T>
inline constexpr matrix<T, 4, 4> operator *(matrix<T, 4, 4> left, T right) {
	return matrix<T, 4, 4>(
		left.col[0] * vector<T, 4>(right),
		left.col[1] * vector<T, 4>(right),
		left.col[2] * vector<T, 4>(right),
		left.col[3] * vector<T, 4>(right));
}

template<class T>
inline constexpr matrix<T, 2, 2> operator /(matrix<T, 2, 2> left, T right) {
	return left / matrix<T, 2, 2>(
		vector<T, 2>(right),
		vector<T, 2>(right));
}

template<class T>
inline constexpr matrix<T, 3, 3> operator /(matrix<T, 3, 3> left, T right) {
	return left / matrix<T, 3, 3>(
		vector<T, 3>(right),
		vector<T, 3>(right),
		vector<T, 3>(right));
}

template<class T>
inline constexpr matrix<T, 4, 4> operator /(matrix<T, 4, 4> left, T right) {
	return left / matrix<T, 4, 4>(
		vector<T, 4>(right),
		vector<T, 4>(right),
		vector<T, 4>(right),
		vector<T, 4>(right));
}

template<class T, int C, int R>
inline constexpr matrix<T, C, R> operator +(T left, matrix<T, C, R> right) {
	return right + left;
}

template<class T, int C, int R>
inline constexpr matrix<T, C, R> operator *(T left, matrix<T, C, R> right) {
	return right * left;
}

template<class T>
inline constexpr matrix<T, 2, 2> operator -(T left, matrix<T, 2, 2> right) {
	return matrix<T, 2, 2>(
		vector<T, 2>(left),
		vector<T, 2>(left)) - right;
}

template<class T>
inline constexpr matrix<T, 3, 3> operator -(T left, matrix<T, 3, 3> right) {
	return matrix<T, 3, 3>(
		vector<T, 3>(left),
		vector<T, 3>(left),
		vector<T, 3>(left)) - right;
}

template<class T>
inline constexpr matrix<T, 4, 4> operator -(T left, matrix<T, 4, 4> right) {
	return matrix<T, 4, 4>(
		vector<T, 4>(left),
		vector<T, 4>(left),
		vector<T, 4>(left),
		vector<T, 4>(left)) - right;
}

template<class T>
inline constexpr matrix<T, 2, 2> operator /(T left, matrix<T, 2, 2> right) {
	return matrix<T, 2, 2>(
		vector<T, 2>(left),
		vector<T, 2>(left)) / right;
}

template<class T>
inline constexpr matrix<T, 3, 3> operator /(T left, matrix<T, 3, 3> right) {
	return matrix<T, 3, 3>(
		vector<T, 3>(left),
		vector<T, 3>(left),
		vector<T, 3>(left)) / right;
}

template<class T>
inline constexpr matrix<T, 4, 4> operator /(T left, matrix<T, 4, 4> right) {
	return matrix<T, 4, 4>(
		vector<T, 4>(left),
		vector<T, 4>(left),
		vector<T, 4>(left),
		vector<T, 4>(left)) / right;
}

template<class T>
inline constexpr bool operator ==(matrix<T, 2, 2> left, matrix<T, 2, 2> right) {
	return
		left.col[0].x == right.col[0].x &&
		left.col[0].y == right.col[0].y &&
		left.col[1].x == right.col[1].x &&
		left.col[1].y == right.col[1].y;
}

template<class T>
inline constexpr bool operator ==(matrix<T, 3, 3> left, matrix<T, 3, 3> right) {
	return
		left.col[0].x == right.col[0].x &&
		left.col[0].y == right.col[0].y &&
		left.col[0].z == right.col[0].z &&
		left.col[1].x == right.col[1].x &&
		left.col[1].y == right.col[1].y &&
		left.col[1].z == right.col[1].z &&
		left.col[2].x == right.col[2].x &&
		left.col[2].y == right.col[2].y &&
		left.col[2].z == right.col[2].z;
}

template<class T>
inline constexpr bool operator ==(matrix<T, 4, 4> left, matrix<T, 4, 4> right) {
	return
		left.col[0].x == right.col[0].x &&
		left.col[0].y == right.col[0].y &&
		left.col[0].z == right.col[0].z &&
		left.col[0].w == right.col[0].w &&
		left.col[1].x == right.col[1].x &&
		left.col[1].y == right.col[1].y &&
		left.col[1].z == right.col[1].z &&
		left.col[1].w == right.col[1].w &&
		left.col[2].x == right.col[2].x &&
		left.col[2].y == right.col[2].y &&
		left.col[2].z == right.col[2].z &&
		left.col[2].w == right.col[2].w &&
		left.col[3].x == right.col[3].x &&
		left.col[3].y == right.col[3].y &&
		left.col[3].z == right.col[3].z &&
		left.col[3].w == right.col[3].w;
}

template<class T>
inline constexpr bool operator !=(matrix<T, 2, 2> left, matrix<T, 2, 2> right) {
	return
		left.col[0].x != right.col[0].x ||
		left.col[0].y != right.col[0].y ||
		left.col[1].x != right.col[1].x ||
		left.col[1].y != right.col[1].y;
}

template<class T>
inline constexpr bool operator !=(matrix<T, 3, 3> left, matrix<T, 3, 3> right) {
	return
		left.col[0].x != right.col[0].x ||
		left.col[0].y != right.col[0].y ||
		left.col[0].z != right.col[0].z ||
		left.col[1].x != right.col[1].x ||
		left.col[1].y != right.col[1].y ||
		left.col[1].z != right.col[1].z ||
		left.col[2].x != right.col[2].x ||
		left.col[2].y != right.col[2].y ||
		left.col[2].z != right.col[2].z;
}

template<class T>
inline constexpr bool operator !=(matrix<T, 4, 4> left, matrix<T, 4, 4> right) {
	return
		left.col[0].x != right.col[0].x ||
		left.col[0].y != right.col[0].y ||
		left.col[0].z != right.col[0].z ||
		left.col[0].w != right.col[0].w ||
		left.col[1].x != right.col[1].x ||
		left.col[1].y != right.col[1].y ||
		left.col[1].z != right.col[1].z ||
		left.col[1].w != right.col[1].w ||
		left.col[2].x != right.col[2].x ||
		left.col[2].y != right.col[2].y ||
		left.col[2].z != right.col[2].z ||
		left.col[2].w != right.col[2].w ||
		left.col[3].x != right.col[3].x ||
		left.col[3].y != right.col[3].y ||
		left.col[3].z != right.col[3].z ||
		left.col[3].w != right.col[3].w;
}

template<class T, int C, int R>
inline matrix<T, C, R> &operator +=(matrix<T, C, R> &left, matrix<T, C, R> right) {
	return left = left + right;
}

template<class T, int C, int R>
inline matrix<T, C, R> &operator -=(matrix<T, C, R> &left, matrix<T, C, R> right) {
	return left = left - right;
}

template<class T, int N>
inline matrix<T, N, N> &operator *=(matrix<T, N, N> &left, matrix<T, N, N> right) {
	return left = left * right;
}

template<class T, int C, int R>
inline matrix<T, C, R> &operator /=(matrix<T, C, R> &left, matrix<T, C, R> right) {
	return left = left / right;
}

template<class T, int C, int R>
inline matrix<T, C, R> &operator +=(matrix<T, C, R> &left, T right) {
	return left = left + right;
}

template<class T, int C, int R>
inline matrix<T, C, R> &operator -=(matrix<T, C, R> &left, T right) {
	return left = left - right;
}

template<class T, int C, int R>
inline matrix<T, C, R> &operator *=(matrix<T, C, R> &left, T right) {
	return left = left * right;
}

template<class T, int C, int R>
inline matrix<T, C, R> &operator /=(matrix<T, C, R> &left, T right) {
	return left = left / right;
}

/*
 * --- Quaternion Operators ---
 */

template<class T>
inline constexpr quaternion<T> operator +(quaternion<T> q) {
	return quaternion<T>(
		+q.x,
		+q.y,
		+q.z,
		+q.w);
}

template<class T>
inline constexpr quaternion<T> operator -(quaternion<T> q) {
	return quaternion<T>(
		-q.x,
		-q.y,
		-q.z,
		-q.w);
}

template<class T> 
inline constexpr quaternion<T> operator +(quaternion<T> left, quaternion<T> right) {
	return quaternion<T>(
		left.x + right.x,
		left.y + right.y,
		left.z + right.z,
		left.w + right.w);
}

template<class T>
inline constexpr quaternion<T> operator -(quaternion<T> left, quaternion<T> right) {
	return quaternion<T>(
		left.x - right.x,
		left.y - right.y,
		left.z - right.z,
		left.w - right.w);
}

template<class T>
inline constexpr quaternion<T> operator *(quaternion<T> left, quaternion<T> right) {
	return quaternion<T>(
		left.w * right.x + left.x * right.w + left.y * right.z - left.z * right.y,
		left.w * right.y - left.x * right.z + left.y * right.w + left.z * right.x,
		left.w * right.z + left.x * right.y - left.y * right.x + left.z * right.w,
		left.w * right.w - left.x * right.x - left.y * right.y - left.z * right.z);
}

template<class T>
inline constexpr quaternion<T> operator /(quaternion<T> left, quaternion<T> right) {
	return quaternion<T>(
		left.x / right.x,
		left.y / right.y,
		left.z / right.z,
		left.w / right.w);
}

template<class T>
inline constexpr quaternion<T> operator +(quaternion<T> left, T right) {
	return quaternion<T>(
		left.x + right,
		left.y + right,
		left.z + right,
		left.w + right);
}

template<class T>
inline constexpr quaternion<T> operator -(quaternion<T> left, T right) {
	return quaternion<T>(
		left.x - right,
		left.y - right,
		left.z - right,
		left.w - right);
}

template<class T>
inline constexpr quaternion<T> operator *(quaternion<T> left, T right) {
	return quaternion<T>(
		left.x * right,
		left.y * right,
		left.z * right,
		left.w * right);
}

template<class T>
inline constexpr quaternion<T> operator /(quaternion<T> left, T right) {
	return quaternion<T>(
		left.x / right,
		left.y / right,
		left.z / right,
		left.w / right);
}

template<class T>
inline constexpr quaternion<T> operator +(T left, quaternion<T> right) {
	return quaternion<T>(
		left + right.x,
		left + right.y,
		left + right.z,
		left + right.w);
}

template<class T>
inline constexpr quaternion<T> operator -(T left, quaternion<T> right) {
	return quaternion<T>(
		left - right.x,
		left - right.y,
		left - right.z,
		left - right.w);
}

template<class T>
inline constexpr quaternion<T> operator *(T left, quaternion<T> right) {
	return quaternion<T>(
		left * right.x,
		left * right.y,
		left * right.z,
		left * right.w);
}

template<class T>
inline constexpr quaternion<T> operator /(T left, quaternion<T> right) {
	return quaternion<T>(
		left / right.x,
		left / right.y,
		left / right.z,
		left / right.w);
}

template<class T>
inline constexpr vector<bool, 4> operator ==(quaternion<T> left, quaternion<T> right) {
	return vector<bool, 4>(
		left.x == right.x,
		left.y == right.y,
		left.z == right.z,
		left.w == right.w);
}

template<class T>
inline constexpr vector<bool, 4> operator !=(quaternion<T> left, quaternion<T> right) {
	return vector<bool, 4>(
		left.x != right.x,
		left.y != right.y,
		left.z != right.z,
		left.w != right.w);
}

template<class T>
inline quaternion<T> &operator +=(quaternion<T> &left, quaternion<T> right) {
	return left = left + right;
}

template<class T>
inline quaternion<T> &operator -=(quaternion<T> &left, quaternion<T> right) {
	return left = left - right;
}

template<class T>
inline quaternion<T> &operator *=(quaternion<T> &left, quaternion<T> right) {
	return left = left * right;
}

template<class T>
inline quaternion<T> &operator /=(quaternion<T> &left, quaternion<T> right) {
	return left = left / right;
}

template<class T>
inline quaternion<T> &operator +=(quaternion<T> &left, T right) {
	return left = left + right;
}

template<class T>
inline quaternion<T> &operator -=(quaternion<T> &left, T right) {
	return left = left - right;
}

template<class T>
inline quaternion<T> &operator *=(quaternion<T> &left, T right) {
	return left = left * right;
}

template<class T>
inline quaternion<T> &operator /=(quaternion<T> &left, T right) {
	return left = left / right;
}

/*
 * --- Trigonometric Functions ---
 */

using std::sin;
using std::cos;
using std::tan;
using std::asin;
using std::acos;
using std::atan;
using std::atan2;
using std::sinh;
using std::cosh;
using std::tanh;
using std::asinh;
using std::acosh;
using std::atanh;

template<class T> 
inline constexpr T radians(T degrees) {
	return degrees * T(pi64) / T(180);
}

template<class T> 
inline constexpr T degrees(T radians) {
	return radians * T(180) / T(pi64);
}

/*
 * --- Exponential Functions ---
 */

using std::pow;
using std::exp;
using std::log;
using std::exp2;
using std::log2;
using std::sqrt;

template<class T>
inline vector<T, 2> pow(vector<T, 2> left, vector<T, 2> right) {
	return vector<T, 2>(
		pow(left.x, right.x),
		pow(left.y, right.y));
}

template<class T>
inline vector<T, 3> pow(vector<T, 3> left, vector<T, 3> right) {
	return vector<T, 3>(
		pow(left.x, right.x),
		pow(left.y, right.y),
		pow(left.z, right.z));
}

template<class T>
inline vector<T, 4> pow(vector<T, 4> left, vector<T, 4> right) {
	return vector<T, 4>(
		pow(left.x, right.x),
		pow(left.y, right.y),
		pow(left.z, right.z),
		pow(left.w, right.w));
}

template<class T, int N>
inline vector<T, N> pow(vector<T, N> left, T right) {
	return pow(left, vector<T, N>(right));
}

template<class T, int N>
inline vector<T, N> pow(T left, vector<T, N> right) {
	return pow(vector<T, N>(left), right);
}

template<class T>
inline vector<T, 2> exp(vector<T, 2> v) {
	return vector<T, 2>(
		exp(v.x),
		exp(v.y));
}

template<class T>
inline vector<T, 3> exp(vector<T, 3> v) {
	return vector<T, 3>(
		exp(v.x),
		exp(v.y),
		exp(v.z));
}

template<class T>
inline vector<T, 4> exp(vector<T, 4> v) {
	return vector<T, 4>(
		exp(v.x),
		exp(v.y),
		exp(v.z),
		exp(v.w));
}

template<class T>
inline vector<T, 2> log(vector<T, 2> v) {
	return vector<T, 2>(
		log(v.x),
		log(v.y));
}

template<class T>
inline vector<T, 3> log(vector<T, 3> v) {
	return vector<T, 3>(
		log(v.x),
		log(v.y),
		log(v.z));
}

template<class T>
inline vector<T, 4> log(vector<T, 4> v) {
	return vector<T, 4>(
		log(v.x),
		log(v.y),
		log(v.z),
		log(v.w));
}

template<class T>
inline vector<T, 2> exp2(vector<T, 2> v) {
	return vector<T, 2>(
		exp2(v.x),
		exp2(v.y));
}

template<class T>
inline vector<T, 3> exp2(vector<T, 3> v) {
	return vector<T, 3>(
		exp2(v.x),
		exp2(v.y),
		exp2(v.z));
}

template<class T>
inline vector<T, 4> exp2(vector<T, 4> v) {
	return vector<T, 4>(
		exp2(v.x),
		exp2(v.y),
		exp2(v.z),
		exp2(v.w));
}

template<class T>
inline vector<T, 2> log2(vector<T, 2> v) {
	return vector<T, 2>(
		log2(v.x),
		log2(v.y));
}

template<class T>
inline vector<T, 3> log2(vector<T, 3> v) {
	return vector<T, 3>(
		log2(v.x),
		log2(v.y),
		log2(v.z));
}

template<class T>
inline vector<T, 4> log2(vector<T, 4> v) {
	return vector<T, 4>(
		log2(v.x),
		log2(v.y),
		log2(v.z),
		log2(v.w));
}

template<class T>
inline vector<T, 2> sqrt(vector<T, 2> v) {
	return vector<T, 2>(
		sqrt(v.x),
		sqrt(v.y));
}

template<class T>
inline vector<T, 3> sqrt(vector<T, 3> v) {
	return vector<T, 3>(
		sqrt(v.x),
		sqrt(v.y),
		sqrt(v.z));
}

template<class T>
inline vector<T, 4> sqrt(vector<T, 4> v) {
	return vector<T, 4>(
		sqrt(v.x),
		sqrt(v.y),
		sqrt(v.z),
		sqrt(v.w));
}

/*
 * --- COMMON FUNCTIONS ---
 */

using std::abs;
using std::floor;
using std::trunc;
using std::round;
using std::ceil;
using std::isnan;
using std::isinf;

template<class T>
inline constexpr T max(T a, T b) {
	return a > b ? a : b;
}

template<class T>
inline constexpr vector<T, 2> max(vector<T, 2> a, vector<T, 2> b) {
	return vector<T, 2>(
		max(a.x, b.x),
		max(a.y, b.y));
}

template<class T>
inline constexpr vector<T, 3> max(vector<T, 3> a, vector<T, 3> b) {
	return vector<T, 3>(
		max(a.x, b.x),
		max(a.y, b.y),
		max(a.z, b.z));
}

template<class T>
inline constexpr vector<T, 4> max(vector<T, 4> a, vector<T, 4> b) {
	return vector<T, 4>(
		max(a.x, b.x),
		max(a.y, b.y),
		max(a.z, b.z),
		max(a.w, b.w));
}

template<class T, int N>
inline constexpr vector<T, N> max(vector<T, N> a, T b) {
	return max(a, vector<T, N>(b));
}

template<class T, int N>
inline constexpr vector<T, N> max(T a, vector<T, N> b) {
	return max(vector<T, N>(a), b);
}

template<class T>
inline constexpr T min(T a, T b) {
	return a < b ? a : b;
}

template<class T>
inline constexpr vector<T, 2> min(vector<T, 2> a, vector<T, 2> b) {
	return vector<T, 2>(
		min(a.x, b.x),
		min(a.y, b.y));
}

template<class T>
inline constexpr vector<T, 3> min(vector<T, 3> a, vector<T, 3> b) {
	return vector<T, 3>(
		min(a.x, b.x),
		min(a.y, b.y),
		min(a.z, b.z));
}

template<class T>
inline constexpr vector<T, 4> min(vector<T, 4> a, vector<T, 4> b) {
	return vector<T, 4>(
		min(a.x, b.x),
		min(a.y, b.y),
		min(a.z, b.z),
		min(a.w, b.w));
}

template<class T, int N>
inline constexpr vector<T, N> min(vector<T, N> a, T b) {
	return min(a, vector<T, N>(b));
}

template<class T, int N>
inline constexpr vector<T, N> min(T a, vector<T, N> b) {
	return min(vector<T, N>(a), b);
}

template<class T>
inline constexpr T clamp(T x, T minVal, T maxVal) {
	return min(max(x, minVal), maxVal);
}

template<class T, int N>
inline constexpr vector<T, N> clamp(vector<T, N> v, vector<T, N> minVal, vector<T, N> maxVal) {
	return min(max(v, minVal), maxVal);
}

template<class T, int N>
inline constexpr vector<T, N> clamp(vector<T, N> v, T minVal, T maxVal) {
	return min(max(v, minVal), maxVal);
}

template<class T, int N>
inline constexpr vector<T, N> saturate(vector<T, N> v) {
	return clamp(v, T(0), T(1));
}

template<class T>
inline constexpr T compSum(vector<T, 2> v) {
	return v.x + v.y;
}

template<class T>
inline constexpr T compSum(vector<T, 3> v) {
	return v.x + v.y + v.z;
}

template<class T>
inline constexpr T compSum(vector<T, 4> v) {
	return v.x + v.y + v.z + v.w;
}

template<class T>
inline constexpr T compMax(vector<T, 2> v) {
	return max(v.x, v.y);
}

template<class T>
inline constexpr T compMax(vector<T, 3> v) {
	return max(v.x, max(v.y, v.z));
}

template<class T>
inline constexpr T compMax(vector<T, 4> v) {
	return max(v.x, max(v.y, max(v.z, v.w)));
}

template<class T>
inline constexpr T compMin(vector<T, 2> v) {
	return min(v.x, v.y);
}

template<class T>
inline constexpr T compMin(vector<T, 3> v) {
	return min(v.x, min(v.y, v.z));
}

template<class T>
inline constexpr T compMin(vector<T, 4> v) {
	return min(v.x, min(v.y, min(v.z, v.w)));
}

template<class T>
inline constexpr vector<T, 2> abs(vector<T, 2> v) {
	return vector<T, 2>(
		v.x < T(0) ? -v.x : v.x,
		v.y < T(0) ? -v.y : v.y);
}

template<class T>
inline constexpr vector<T, 3> abs(vector<T, 3> v) {
	return vector<T, 3>(
		v.x < T(0) ? -v.x : v.x,
		v.y < T(0) ? -v.y : v.y);
}

template<class T>
inline constexpr vector<T, 4> abs(vector<T, 4> v) {
	return vector<T, 4>(
		v.x < T(0) ? -v.x : v.x,
		v.y < T(0) ? -v.y : v.y);
}

template<class T>
inline constexpr T sign(T x) {
	return (x > T(0)) - (x < T(0));
}

template<class T, int N>
inline constexpr vector<T, N> sign(vector<T, N> v) {
	return vector<T, N>(v > T(0)) - vector<T, N>(v < T(0));
}

template<class T>
inline vector<T, 2> floor(vector<T, 2> v) {
	return vector<T, 2>(
		floor(v.x),
		floor(v.y));
}

template<class T>
inline vector<T, 3> floor(vector<T, 3> v) {
	return vector<T, 3>(
		floor(v.x),
		floor(v.y),
		floor(v.z));
}

template<class T>
inline vector<T, 4> floor(vector<T, 4> v) {
	return vector<T, 4>(
		floor(v.x),
		floor(v.y),
		floor(v.z),
		floor(v.w));
}

template<class T>
inline vector<T, 2> trunc(vector<T, 2> v) {
	return vector<T, 2>(
		trunc(v.x),
		trunc(v.y));
}

template<class T>
inline vector<T, 3> trunc(vector<T, 3> v) {
	return vector<T, 3>(
		trunc(v.x),
		trunc(v.y),
		trunc(v.z));
}

template<class T>
inline vector<T, 4> trunc(vector<T, 4> v) {
	return vector<T, 4>(
		trunc(v.x),
		trunc(v.y),
		trunc(v.z),
		trunc(v.w));
}

template<class T>
inline vector<T, 2> round(vector<T, 2> v) {
	return vector<T, 2>(
		round(v.x),
		round(v.y));
}

template<class T>
inline vector<T, 3> round(vector<T, 3> v) {
	return vector<T, 3>(
		round(v.x),
		round(v.y),
		round(v.z));
}

template<class T>
inline vector<T, 4> round(vector<T, 4> v) {
	return vector<T, 4>(
		round(v.x),
		round(v.y),
		round(v.z),
		round(v.w));
}

template<class T>
inline vector<T, 2> ceil(vector<T, 2> v) {
	return vector<T, 2>(
		ceil(v.x),
		ceil(v.y));
}

template<class T>
inline vector<T, 3> ceil(vector<T, 3> v) {
	return vector<T, 3>(
		ceil(v.x),
		ceil(v.y),
		ceil(v.z));
}

template<class T>
inline vector<T, 4> ceil(vector<T, 4> v) {
	return vector<T, 4>(
		ceil(v.x),
		ceil(v.y),
		ceil(v.z),
		ceil(v.w));
}

template<class T>
inline T fract(T x) {
	T intpart;
	return modf(x, &intpart);
}

template<class T>
inline vector<T, 2> fract(vector<T, 2> v) {
	return vector<T, 2>(
		fract(v.x),
		fract(v.y));
}

template<class T>
inline vector<T, 3> fract(vector<T, 3> v) {
	return vector<T, 3>(
		fract(v.x),
		fract(v.y),
		fract(v.z));
}

template<class T>
inline vector<T, 4> fract(vector<T, 4> v) {
	return vector<T, 4>(
		fract(v.x),
		fract(v.y),
		fract(v.z),
		fract(v.w));
}

template<class T>
inline constexpr T mod(T a, T b) {
	return a % b;
}

inline float mod(float a, float b) {
	return fmodf(a, b);
}

inline double mod(double a, double b) {
	return fmod(a, b);
}

template<class T>
inline vector<T, 2> mod(vector<T, 2> left, vector<T, 2> right) {
	return vector<T, 2>(
		mod(left.x, right.x),
		mod(left.y, right.y));
}

template<class T>
inline vector<T, 3> mod(vector<T, 3> left, vector<T, 3> right) {
	return vector<T, 3>(
		mod(left.x, right.x),
		mod(left.y, right.y),
		mod(left.z, right.z));
}

template<class T>
inline vector<T, 4> mod(vector<T, 4> left, vector<T, 4> right) {
	return vector<T, 4>(
		mod(left.x, right.x),
		mod(left.y, right.y),
		mod(left.z, right.z),
		mod(left.w, right.w));
}

template<class T, int N>
inline vector<T, N> mod(vector<T, N> left, T right) {
	return mod(left, vector<T, N>(right));
}

template<class T, int N>
inline vector<T, N> mod(T left, vector<T, N> right) {
	return mod(vector<T, N>(left), right);
}

template<class T>
inline constexpr T step(T edge, T x) {
	return T(x >= edge);
}

template<class T, int N>
inline constexpr vector<T, N> step(vector<T, N> edge, vector<T, N> x) {
	return vector<T, N>(x >= edge);
}

template<class T, int N>
inline constexpr vector<T, N> step(T edge, vector<T, N> x) {
	return vector<T, N>(x >= edge);
}

template<class T>
inline constexpr T smoothstep(T edge1, T edge2, T x) {
	T t = clamp((x - edge1) / (edge2 - edge1), T(0), T(1));
	return t * t * (T(3) - T(2) * t);
}

template<class T, int N>
inline constexpr vector<T, N> smoothstep(vector<T, N> edge1, vector<T, N> edge2, vector<T, N> x) {
	vector<T, N> t = clamp((x - edge1) / (edge2 - edge1), T(0), T(1));
	return t * t * (T(3) - T(2) * t);
}

template<class T, int N>
inline constexpr vector<T, N> smoothstep(T edge1, T edge2, vector<T, N> x) {
	vector<T, N> t = clamp((x - edge1) / (edge2 - edge1), T(0), T(1));
	return t * t * (T(3) - T(2) * t);
}

template<class T>
inline constexpr vector<bool, 2> isnan(vector<T, 2> v) {
	return vector<bool, 2>(
		isnan(v.x),
		isnan(v.y));
}

template<class T>
inline constexpr vector<bool, 3> isnan(vector<T, 3> v) {
	return vector<bool, 3>(
		isnan(v.x),
		isnan(v.y),
		isnan(v.z));
}

template<class T>
inline constexpr vector<bool, 4> isnan(vector<T, 4> v) {
	return vector<bool, 4>(
		isnan(v.x),
		isnan(v.y),
		isnan(v.z),
		isnan(v.w));
}

template<class T>
inline constexpr vector<bool, 2> isinf(vector<T, 2> v) {
	return vector<bool, 2>(
		isinf(v.x),
		isinf(v.y));
}

template<class T>
inline constexpr vector<bool, 3> isinf(vector<T, 3> v) {
	return vector<bool, 3>(
		isinf(v.x),
		isinf(v.y),
		isinf(v.z));
}

template<class T>
inline constexpr vector<bool, 4> isinf(vector<T, 4> v) {
	return vector<bool, 4>(
		isinf(v.x),
		isinf(v.y),
		isinf(v.z),
		isinf(v.w));
}

/*
 * --- Color Space Functions ---
 */

inline constexpr vec4 unpackRGBA8(uint r8g8b8a8) {
	uint r = (r8g8b8a8 >> 24) & 0xFF;
	uint g = (r8g8b8a8 >> 16) & 0xFF;
	uint b = (r8g8b8a8 >>  8) & 0xFF;
	uint a = (r8g8b8a8 >>  0) & 0xFF;
	return vec4(r, g, b, a) * (1.0f / 255.0f);
}

inline constexpr uint packRGBA8(vec4 rgba) {
	uint r = uint(rgba.x * 255.5f);
	uint g = uint(rgba.g * 255.5f);
	uint b = uint(rgba.b * 255.5f);
	uint a = uint(rgba.a * 255.5f);
	return
		(r << 24) |
		(g << 16) |
		(b <<  8) |
		(a <<  0);
}

inline constexpr vec3 HSVtoRGB(vec3 hsv) {
	float h = hsv.x;
	float s = hsv.y;
	float v = hsv.z;
	int i = (int)(h * 6.0f);
	float f = h * 6.0f - i;
	float p = v * (1.0f - s);
	float q = v * (1.0f - f * s);
	float t = v * (1.0f - (1.0f - f) * s);
	switch (i % 6) {
		case 0:  return vec3(v, t, p);
		case 1:  return vec3(q, v, p);
		case 2:  return vec3(p, v, t);
		case 3:  return vec3(p, q, v);
		case 4:  return vec3(t, p, v);
		default: return vec3(v, p, q);
	}
}

inline constexpr vec3 RGBtoHSV(vec3 rgb) {
	float min = compMin(rgb);
	float max = compMax(rgb);
	if (max == 0)
		return vec3(0);

	float delta = max - min;
	float v = max;
	float s = delta / max;
	float h =
		rgb.x == max ? (rgb.g - rgb.b) / (6 * delta) + 0 / 3.0f :
		rgb.y == max ? (rgb.b - rgb.r) / (6 * delta) + 1 / 3.0f :
		(rgb.r - rgb.g) / (6 * delta) + 2 / 3.0f;

	return vec3(h < 0 ? 1 + h : h, s, v);
}

/*
 * --- Geometric Functions ---
 */

template<class T, int N>
inline constexpr T dot(vector<T, N> left, vector<T, N> right) {
	return compSum(left * right);
}

template<class T>
inline constexpr vector<T, 3> cross(vector<T, 3> left, vector<T, 3> right) {
	return vector<T, 3>(
		left.y * right.z - right.y * left.z,
		left.z * right.x - right.z * left.x,
		left.x * right.y - right.x * left.y);
}

template<class T, int N>
inline constexpr T lengthSq(vector<T, N> v) {
	return dot(v, v);
}

template<class T, int N>
inline constexpr T distanceSq(vector<T, N> p1, vector<T, N> p2) {
	return lengthSquated(p1 - p2);
}

template<class T, int N>
inline T length(vector<T, N> v) {
	return sqrt(dot(v, v));
}

template<class T, int N>
inline T distance(vector<T, N> p1, vector<T, N> p2) {
	return length(p1 - p2);
}

template<class T, int N>
inline vector<T, N> normalize(vector<T, N> v) {
	return v / length(v);
}

template<class T, int N>
inline constexpr vector<T, N> faceforward(vector<T, N> normal, vector<T, N> incidence) {
	return dot(incidence, normal) < T(0) ? normal : -normal;
}

template<class T, int N>
inline constexpr vector<T, N> reflect(vector<T, N> incidence, vector<T, N> normal) {
	return incidence - T(2) * dot(incidence, normal) * normal;
}

template<class T, int N>
inline vector<T, N> refract(vector<T, N> incidence, vector<T, N> normal, T eta) {
	T d = dot(incidence, normal);
	T k = T(1) - eta * eta * (T(1) - d * d);
	if (k < T(0))
		return vector<T, N>(T(0));
	return eta * incidence - normal * (eta * d + sqrt(k));
}

template<class T>
inline constexpr T lerp(T from, T to, T amount) {
	return from + (to - from) * amount;
}

template<class T, int N>
inline constexpr vector<T, N> lerp(vector<T, N> from, vector<T, N> to, vector<T, N> amount) {
	return from + (to - from) * amount;
}

template<class T, int N>
inline constexpr vector<T, N> lerp(vector<T, N> from, vector<T, N> to, T amount) {
	return from + (to - from) * amount;
}

template<class T>
inline vector<T, 3> slerp(vector<T, 3> from, vector<T, 3> to, T amount) {
	vector<T, 3> z = to;

	T cosTheta = dot(from, to);
	/* if cosTheta < 0, the interpolation will take the long way around the sphere */
	if (cosTheta < T(0)) {
		z = -to;
		cosTheta = -cosTheta;
	}

	/* sin(angle) -> 0! too close for comfort - do a lerp instead */
	if (cosTheta > T(0.99999))
		return lerp(from, to, amount);

	/* Essential Mathematics, page 467 */
	T angle = acos(cosTheta);
	return (sin((T(1) - amount) * angle) * from + sin(amount * angle) * z) / sin(angle);
}

/*
 * --- Matrix Functions ---
 */

template<class T>
inline constexpr matrix<T, 2, 2> outerProduct(vector<T, 2> left, vector<T, 2> right) {
	return matrix<T, 2, 2>(
		left.x * right.x, left.y * right.x,
		left.x * right.y, left.y * right.y);
}

template<class T>
inline constexpr matrix<T, 3, 3> outerProduct(vector<T, 3> left, vector<T, 3> right) {
	return matrix<T, 3, 3>(
		left.x * right.x, left.y * right.x, left.z * right.x,
		left.x * right.y, left.y * right.y, left.z * right.y,
		left.x * right.z, left.y * right.z, left.z * right.z);
}

template<class T>
inline constexpr matrix<T, 4, 4> outerProduct(vector<T, 4> left, vector<T, 4> right) {
	return matrix<T, 4, 4>(
		left.x * right.x, left.y * right.x, left.z * right.x, left.w * right.x,
		left.x * right.y, left.y * right.y, left.z * right.y, left.w * right.y,
		left.x * right.z, left.y * right.z, left.z * right.z, left.w * right.z,
		left.x * right.w, left.y * right.w, left.z * right.w, left.w * right.w);
}

template<class T>
inline constexpr matrix<T, 2, 2> matCompMul(matrix<T, 2, 2> left, matrix<T, 2, 2> right) {
	return matrix<T, 2, 2>(
		left.col[0] * right.col[0],
		left.col[1] * right.col[1]);
}

template<class T>
inline constexpr matrix<T, 3, 3> matCompMul(matrix<T, 3, 3> left, matrix<T, 3, 3> right) {
	return matrix<T, 3, 3>(
		left.col[0] * right.col[0],
		left.col[1] * right.col[1],
		left.col[2] * right.col[2]);
}

template<class T>
inline constexpr matrix<T, 4, 4> matCompMul(matrix<T, 4, 4> left, matrix<T, 4, 4> right) {
	return matrix<T, 4, 4>(
		left.col[0] * right.col[0],
		left.col[1] * right.col[1],
		left.col[2] * right.col[2],
		left.col[3] * right.col[3]);
}

template<class T>
inline constexpr matrix<T, 2, 2> transpose(matrix<T, 2, 2> m) {
	return matrix<T, 2, 2>(
		m.col[0].x, m.col[1].x,
		m.col[0].y, m.col[1].y);
}

template<class T>
inline constexpr matrix<T, 3, 3> transpose(matrix<T, 3, 3> m) {
	return matrix<T, 3, 3>(
		m.col[0].x, m.col[1].x, m.col[2].x,
		m.col[0].y, m.col[1].y, m.col[2].y,
		m.col[0].z, m.col[1].z, m.col[2].z);
}

template<class T>
inline constexpr matrix<T, 4, 4> transpose(matrix<T, 4, 4> m) {
	return matrix<T, 4, 4>(
		m.col[0].x, m.col[1].x, m.col[2].x, m.col[3].x,
		m.col[0].y, m.col[1].y, m.col[2].y, m.col[3].y,
		m.col[0].z, m.col[1].z, m.col[2].z, m.col[3].z,
		m.col[0].w, m.col[1].w, m.col[2].w, m.col[3].w);
}

template<class T>
inline constexpr T determinant(matrix<T, 2, 2> m) {
	return m.col[0].x * m.col[1].y - m.col[1].x * m.col[0].y;
}

template<class T>
inline constexpr T determinant(matrix<T, 3, 3> m) {
	return
		+ m.col[0].x * (m.col[1].y * m.col[2].z - m.col[2].y * m.col[1].z)
		- m.col[1].x * (m.col[0].y * m.col[2].z - m.col[2].y * m.col[0].z)
		+ m.col[2].x * (m.col[0].y * m.col[1].z - m.col[1].y * m.col[0].z);
}

template<class T>
inline constexpr T determinant(matrix<T, 4, 4> m) {
	T f0 = m.col[2].z * m.col[3].w - m.col[3].z * m.col[2].w;
	T f1 = m.col[2].y * m.col[3].w - m.col[3].y * m.col[2].w;
	T f2 = m.col[2].y * m.col[3].z - m.col[3].y * m.col[2].z;
	T f3 = m.col[2].x * m.col[3].w - m.col[3].x * m.col[2].w;
	T f4 = m.col[2].x * m.col[3].z - m.col[3].x * m.col[2].z;
	T f5 = m.col[2].x * m.col[3].y - m.col[3].x * m.col[2].y;

	return
		m.col[0].x * (m.col[1].y * f0 - m.col[1].z * f1 + m.col[1].w * f2) -
		m.col[0].y * (m.col[1].x * f0 - m.col[1].z * f3 + m.col[1].w * f4) +
		m.col[0].z * (m.col[1].x * f1 - m.col[1].y * f3 + m.col[1].w * f5) -
		m.col[0].w * (m.col[1].x * f2 - m.col[1].y * f4 + m.col[1].z * f5);
}

template<class T>
inline constexpr matrix<T, 2, 2> inverse(matrix<T, 2, 2> m) {
	T inverseDet = T(1) / (
		+ m.col[0].x * m.col[1].y
		- m.col[1].x * m.col[0].y);

	return matrix<T, 2, 2>(
		+m.col[1].y * inverseDet,
		-m.col[0].y * inverseDet,
		-m.col[1].x * inverseDet,
		+m.col[0].x * inverseDet);
}

template<class T>
inline constexpr matrix<T, 3, 3> inverse(matrix<T, 3, 3> m) {
	T inverseDet = T(1) / (
		+ m.col[0].x * (m.col[1].y * m.col[2].z - m.col[2].y * m.col[1].z)
		- m.col[1].x * (m.col[0].y * m.col[2].z - m.col[2].y * m.col[0].z)
		+ m.col[2].x * (m.col[0].y * m.col[1].z - m.col[1].y * m.col[0].z));

	return matrix<T, 3, 3>(
		+(m.col[1].y * m.col[2].z - m.col[2].y * m.col[1].z) * inverseDet,
		-(m.col[0].y * m.col[2].z - m.col[2].y * m.col[0].z) * inverseDet,
		+(m.col[0].y * m.col[1].z - m.col[1].y * m.col[0].z) * inverseDet,
		-(m.col[1].x * m.col[2].z - m.col[2].x * m.col[1].z) * inverseDet,
		+(m.col[0].x * m.col[2].z - m.col[2].x * m.col[0].z) * inverseDet,
		-(m.col[0].x * m.col[1].z - m.col[1].x * m.col[0].z) * inverseDet,
		+(m.col[1].x * m.col[2].y - m.col[2].x * m.col[1].y) * inverseDet,
		-(m.col[0].x * m.col[2].y - m.col[2].x * m.col[0].y) * inverseDet,
		+(m.col[0].x * m.col[1].y - m.col[1].x * m.col[0].y) * inverseDet);
}

template<class T>
inline constexpr matrix<T, 4, 4> inverse(matrix<T, 4, 4> m) {

	T c00 = m.col[2].z * m.col[3].w - m.col[3].z * m.col[2].w;
	T c02 = m.col[1].z * m.col[3].w - m.col[3].z * m.col[1].w;
	T c03 = m.col[1].z * m.col[2].w - m.col[2].z * m.col[1].w;
	T c04 = m.col[2].y * m.col[3].w - m.col[3].y * m.col[2].w;
	T c06 = m.col[1].y * m.col[3].w - m.col[3].y * m.col[1].w;
	T c07 = m.col[1].y * m.col[2].w - m.col[2].y * m.col[1].w;
	T c08 = m.col[2].y * m.col[3].z - m.col[3].y * m.col[2].z;
	T c10 = m.col[1].y * m.col[3].z - m.col[3].y * m.col[1].z;
	T c11 = m.col[1].y * m.col[2].z - m.col[2].y * m.col[1].z;
	T c12 = m.col[2].x * m.col[3].w - m.col[3].x * m.col[2].w;
	T c14 = m.col[1].x * m.col[3].w - m.col[3].x * m.col[1].w;
	T c15 = m.col[1].x * m.col[2].w - m.col[2].x * m.col[1].w;
	T c16 = m.col[2].x * m.col[3].z - m.col[3].x * m.col[2].z;
	T c18 = m.col[1].x * m.col[3].z - m.col[3].x * m.col[1].z;
	T c19 = m.col[1].x * m.col[2].z - m.col[2].x * m.col[1].z;
	T c20 = m.col[2].x * m.col[3].y - m.col[3].x * m.col[2].y;
	T c22 = m.col[1].x * m.col[3].y - m.col[3].x * m.col[1].y;
	T c23 = m.col[1].x * m.col[2].y - m.col[2].x * m.col[1].y;

	vector<T, 4> f0(c00, c00, c02, c03);
	vector<T, 4> f1(c04, c04, c06, c07);
	vector<T, 4> f2(c08, c08, c10, c11);
	vector<T, 4> f3(c12, c12, c14, c15);
	vector<T, 4> f4(c16, c16, c18, c19);
	vector<T, 4> f5(c20, c20, c22, c23);

	vector<T, 4> v0(m.col[1].x, m.col[0].x, m.col[0].x, m.col[0].x);
	vector<T, 4> v1(m.col[1].y, m.col[0].y, m.col[0].y, m.col[0].y);
	vector<T, 4> v2(m.col[1].z, m.col[0].z, m.col[0].z, m.col[0].z);
	vector<T, 4> v3(m.col[1].w, m.col[0].w, m.col[0].w, m.col[0].w);

	vector<T, 4> inverse0(v1 * f0 - v2 * f1 + v3 * f2);
	vector<T, 4> inverse1(v0 * f0 - v2 * f3 + v3 * f4);
	vector<T, 4> inverse2(v0 * f1 - v1 * f3 + v3 * f5);
	vector<T, 4> inverse3(v0 * f2 - v1 * f4 + v2 * f5);

	vector<T, 4> signA(+1, -1, +1, -1);
	vector<T, 4> signB(-1, +1, -1, +1);
	matrix<T, 4, 4> inverse(
		inverse0 * signA,
		inverse1 * signB,
		inverse2 * signA,
		inverse3 * signB);

	vector<T, 4> row0(
		inverse.col[0].x, 
		inverse.col[1].x, 
		inverse.col[2].x, 
		inverse.col[3].x);

	vector<T, 4> d(m.col[0] * row0);	
	return inverse / (d.x + d.y + d.z + d.w);
}

template<class T>
inline constexpr matrix<T, 4, 4> scaleMat(vector<T, 3> xyz) {
	return matrix<T, 4, 4>(vector<T, 4>(xyz, T(1)));
}

template<class T>
inline constexpr matrix<T, 4, 4> scaleMat(T x, T y, T z) {
	return matrix<T, 4, 4>(vector<T, 4>(x, y, z, T(1)));
}

template<class T>
inline constexpr matrix<T, 4, 4> translationMat(vector<T, 3> xyz) {
	return matrix<T, 4, 4>(
		 T(1),  T(0),  T(0), T(0),
		 T(0),  T(1),  T(0), T(0),
		 T(0),  T(0),  T(1), T(0),
		xyz.x, xyz.y, xyz.z, T(1));
}

template<class T>
inline constexpr matrix<T, 4, 4> translationMat(T x, T y, T z) {
	return matrix<T, 4, 4>(
		T(1), T(0), T(0), T(0),
		T(0), T(1), T(0), T(0),
		T(0), T(0), T(1), T(0),
		  x,    y,    z,  T(1));
}

template<class T>
inline matrix<T, 4, 4> rotationMat(vector<T, 3> axis, T angleRad) {
	T a = angleRad;
	T s = sin(a);
	T c = cos(a);

	axis = normalize(axis);
	vector<T, 3> temp((T(1) - c) * axis);

	return matrix<T, 4, 4>(
		c + temp.x * axis.x,
		temp.x * axis.y + s * axis.z,
		temp.x * axis.z - s * axis.y,
		T(0),

		temp.y * axis.x - s * axis.z,
		c + temp.y * axis.y,
		temp.y * axis.z + s * axis.x,
		T(0),

		temp.z * axis.x + s * axis.y,
		temp.z * axis.y - s * axis.x,
		c + temp.z * axis.z,
		T(0),
		
		T(0), T(0), T(0), T(1));
}

template<class T>
inline matrix<T, 4, 4> lookAtMatRH(vector<T, 3> pos, vector<T, 3> dir, vector<T, 3> up) {
	vector<T, 3> f = normalize(dir);
	vector<T, 3> r = normalize(cross(f, up));
	vector<T, 3> u = cross(r, f);

	return matrix<T, 4, 4>(
		r.x, u.x, -f.x, -dot(r, pos),
		r.y, u.y, -f.y, -dot(u, pos),
		r.z, u.z, -f.z, +dot(f, pos),
		T(0), T(0), T(0), T(1));
}

template<class T>
inline matrix<T, 4, 4> lookAtMatLH(vector<T, 3> pos, vector<T, 3> dir, vector<T, 3> up) {
	vector<T, 3> f = normalize(dir);
	vector<T, 3> r = normalize(cross(up, f));
	vector<T, 3> u = cross(r, f);

	return matrix<T, 4, 4>(
		r.x, u.x, f.x, -dot(r, pos),
		r.y, u.y, f.y, -dot(u, pos),
		r.z, u.z, f.z, -dot(f, pos),
		T(0), T(0), T(0), T(1));
}

template<class T>
inline matrix<T, 4, 4> perspectiveMatRH(T vertFOV, T aspect, T near, T far) {
	T theta = tan(vertFOV / T(2));

	matrix<T, 4, 4> m(T(0));
	m.col[0].x = +T(1) / (aspect * theta);
	m.col[1].y = +T(1) / (theta);
	m.col[2].w = -T(1);

#if defined(BMATH_DEPTH_ZERO_TO_ONE)
	m.col[2].z = +far / (near - far);
	m.col[3].z = -(far * near) / (far - near);
#elif defined(BMATH_DEPTH_MINUS_ONE_TO_ONE)
	m.col[2].z = -(far + near) / (far - near);
	m.col[3].z = -(T(2) * far * near) / (far - near);
#endif

	return m;
}

template<class T>
inline matrix<T, 4, 4> perspectiveMatLH(T vertFOV, T aspect, T near, T far) {
	T theta = tan(vertFOV / T(2));

	matrix<T, 4, 4> m(T(0));
	m.col[0].x = +T(1) / (aspect * theta);
	m.col[1].y = +T(1) / (theta);
	m.col[2].w = +T(1);

#if defined(B_DEPTH_CLIP_ZERO_TO_ONE)
	m.col[2].z = +far / (far - near);
	m.col[3].z = -(far * near) / (far - near);
#elif defined(B_DEPTH_CLIP_MINUS_ONE_TO_ONE)
	m.col[2].z = +(far + near) / (far - near);
	m.col[3].z = -(T(2) * far * near) / (far - near);
#endif

	return m;
}

template <typename T>
inline matrix<T, 4, 4> orthoRH(
	T left, T right,
	T bottom, T top,
	T near, T far) 
{
	matrix<T, 4, 4> m(1);
	m.col[0].x = T(2) / (right - left);
	m.col[1].y = T(2) / (top - bottom);
	m.col[3].x = -(right + left) / (right - left);
	m.col[3].y = -(top + bottom) / (top - bottom);

#if defined(B_DEPTH_CLIP_ZERO_TO_ONE)
	m.col[2].z = -T(1) / (zFar - near);
	m.col[3].z = -near / (zFar - near);
#elif defined(B_DEPTH_CLIP_MINUS_ONE_TO_ONE)
	m.col[2].z = -T(2) / (far - near);
	m.col[3].z = -(far + near) / (far - near);
#endif

	return m;
}


template <typename T>
inline matrix<T, 4, 4> orthoMatLH(
	T left, T right,
	T bottom, T top,
	T near, T far)
{
	matrix<T, 4, 4> m(1);
	m.col[0].x = T(2) / (right - left);
	m.col[1].y = T(2) / (top - bottom);
	m.col[3].x = -(right + left) / (right - left);
	m.col[3].y = -(top + bottom) / (top - bottom);

#if defined(B_DEPTH_CLIP_ZERO_TO_ONE)
	m.col[2].z = T(1) / (far - near);
	m.col[3].z = -near / (far - near);
#elif defined(B_DEPTH_CLIP_MINUS_ONE_TO_ONE)
	m.col[2].z = T(2) / (far - near);
	m.col[3].z = -(far + near) / (far - near);
#endif

	return m;
}

/*
 * --- Vector Relational Functions ---
 */

inline constexpr bool all(bvec2 v) {
	return v.x && v.y;
}

inline constexpr bool all(bvec3 v) {
	return v.x && v.y && v.z;
}

inline constexpr bool all(bvec4 v) {
	return v.x && v.y && v.z && v.w;
}

inline constexpr bool any(bvec2 v) {
	return v.x || v.y;
}

inline constexpr bool any(bvec3 v) {
	return v.x || v.y || v.z;
}

inline constexpr bool any(bvec4 v) {
	return v.x || v.y || v.z || v.w;
}

template<class T>
inline bool epsilonEqual(T left, T right, T epsilon) {
	return fdim(left, right) <= epsilon;
}

template<class T, int N>
inline constexpr vector<bool, N> epsilonEqual(vector<T, N> left, vector<T, N> right, T epsilon) {
	return abs(left - right) <= epsilon;
}

template<class T>
inline bool epsilonNotEqual(T left, T right, T epsilon) {
	return fdim(left, right) > epsilon;
}

template<class T, int N>
inline constexpr vector<bool, N> epsilonNotEqual(vector<T, N> left, vector<T, N> right, T epsilon) {
	return abs(left - right) > epsilon;
}

/*
 * --- Quaternion Functions ---
 */

template<class T>
inline vector<T, 3> axis(quaternion<T> q) {
	T s = T(1) - q.w * q.w;
	if (s <= T(0))
		return vector<T, 3>(0, 0, 1);
	return q.xyz / sqrt(s);
}

template<class T>
inline T angle(quaternion<T> q) {
	return acos(q.w) * T(2);
}

template<class T>
inline constexpr quaternion<T> conjugate(quaternion<T> q) {
	return quaternion<T>(-q.x, -q.y, -q.z, q.w);
}

template<class T>
inline constexpr T lengthSq(quaternion<T> q) {
	return q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
}

template<class T>
inline T length(quaternion<T> q) {
	return sqrt(lengthSq(q));
}

template<class T>
inline quaternion<T> normalize(quaternion<T> q) {
	return q / length(q);
}

template<class T>
inline constexpr quaternion<T> inverse(quaternion<T> q) {
	return conjugate(q) / dot(q.xyzw, q.xyzw);
}

template<class T>
inline quaternion<T> slerp(quaternion<T> from, quaternion<T> to, T amount) {
	quaternion<T> z = to;

	T cosTheta = dot(from.xyzw, to.xyzw);
	/* if cosTheta < 0, the interpolation will take the long way around the sphere */
	if (cosTheta < T(0)) {
		z = -to;
		cosTheta = -cosTheta;
	}

	/* sin(angle) -> 0! too close for comfort - do a lerp instead */
	if (cosTheta > T(0.99999))
		return lerp(from, to, amount);

	/* Essential Mathematics, page 467 */
	T angle = acos(cosTheta);
	return (sin((T(1) - amount) * angle) * from + sin(amount * angle) * z) / sin(angle);
}

template<class T>
inline quaternion<T> nlerp(quaternion<T> from, quaternion<T> to, T amount) {
	return normalize(from + (to - from) * amount);
}

template<class T>
inline quaternion<T> rotationQuat(vector<T, 3> axis, T angleRad) {
	T a = angleRad / 2;
	axis = normalize(axis);
	return quaternion<T>(sin(a) * axis, cos(a));
}

template<class T>
inline quaternion<T> rotationQuat(vector<T, 3> from, vector<T, 3> to) {
	T cosTheta = dot(from, to);
	vector<T, 3> axis;

	if (cosTheta >= T(0.99999))
		return quat(T(0), T(0), T(0), T(0));

	if (cosTheta < T(-0.99999)) {
		/* special case when vectors in opposite directions :
		 * there is no "ideal" rotation axis
		 * so guess one; any will do as long as it's perpendicular to start
		 * this implementation favors a rotation around the Up axis (Y),
		 * since it's often what you want to do. */
		axis = cross(vector<T, 3>(0, 0, 1), from);
		if (lengthSq(axis) < T(0.00001)) /* bad luck, they were parallel, try again! */
			axis = cross(vector<T, 3>(1, 0, 0), from);

		axis = normalize(axis);
		return rotationQuat(axis, T(pi64));
	}

	/* from Stan Melax's Game Programming Gems 1 article */
	axis = cross(from, to);

	T s = sqrt((T(1) + cosTheta) * T(2));
	T invs = T(1) / s;

	return quaternion<T>(
		s * T(0.5f),
		axis.x * invs,
		axis.y * invs,
		axis.z * invs);
}

template<class T>
inline constexpr quaternion<T> rotate(quaternion<T> q, quaternion<T> rot) {
	return rot * q * inverse(rot);
}

template<class T>
inline constexpr quaternion<T> rotate(vector<T, 3> v, quaternion<T> rot) {
	return rot * quaternion<T>(v, T(0)) * inverse(rot);
}

template<class T>
inline constexpr quaternion<T> rotate(vector<T, 3> v, vector<T, 3> axis, T angleRad) {
	quaternion<T> rot = rotationQuat(axis, angleRad);
	return rotate(v, rot);
}

template<class T>
inline constexpr matrix<T, 4, 4> quatToMat(quaternion<T> q) {
	return matrix<T, 4, 4>(
		T(1) - T(2) * (q.y * q.y + q.z * q.z),
		T(2) * (q.x * q.y + q.w * q.z),
		T(2) * (q.x * q.z - q.w * q.y),
		T(0),

		T(2) * (q.x * q.y - q.w * q.z),
		T(1) - T(2) * (q.x * q.x + q.z * q.z),
		T(2) * (q.y * q.z + q.w * q.x),
		T(0),

		T(2) * (q.x * q.z + q.w * q.y),
		T(2) * (q.y * q.z - q.w * q.x),
		T(1) - T(2) * (q.x * q.x + q.y * q.y),
		T(0),

		T(0), T(0), T(0), T(1));
}

template<class T>
inline quaternion<T> matToQuat(matrix<T, 4, 4> m) {
	T x = m.col[0].x - m.col[1].y - m.col[2].z;
	T y = m.col[1].y - m.col[0].x - m.col[2].z;
	T z = m.col[2].z - m.col[0].x - m.col[1].y;
	T w = m.col[0].x + m.col[1].y + m.col[2].z;

	T maxVal = w;
	int maxIdx = 0;
	if (x > maxVal) {
		maxVal = x;
		maxIdx = 1;
	}
	if (y > maxVal) {
		maxVal = y;
		maxIdx = 2;
	}
	if (z > maxVal) {
		maxVal = z;
		maxIdx = 3;
	}

	maxVal = sqrt(maxVal + T(1)) / T(2);
	T mult = T(0.25) / maxVal;

	switch (maxIdx) {
		case 0:
			return quaternion<T>(
				(m.col[1].z - m.col[2].y) * mult,
				(m.col[2].x - m.col[0].z) * mult,
				(m.col[0].y - m.col[1].x) * mult,
				maxVal);
		case 1:
			return quaternion<T>(
				maxVal,
				(m.col[0].y + m.col[1].x) * mult,
				(m.col[2].x + m.col[0].z) * mult,
				(m.col[1].z - m.col[2].y) * mult);
		case 2:
			return quaternion<T>(
				(m.col[0].y + m.col[1].x) * mult,
				maxVal,
				(m.col[1].z + m.col[2].y) * mult,
				(m.col[2].x - m.col[0].z) * mult);
		case 3:
			return quaternion<T>(
				(m.col[2].x + m.col[0].z) * mult,
				(m.col[1].z + m.col[2].y) * mult,
				maxVal,
				(m.col[0].y - m.col[1].x) * mult);
		default: return quaternion<T>(0, 0, 0, 0);
	}
}

B_MATH_END

#undef B_MATH_BEGIN
#undef B_MATH_END

#endif /* !B_MATH */

/*
  PUBLIC DOMAIN LICENCE

  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or
  distribute this software, either in source code form or as a compiled
  binary, for any purpose, commercial or non - commercial, and by any
  means.

  In jurisdictions that recognize copyright laws, the author or authors
  of this software dedicate any and all copyright interest in the
  software to the public domain. We make this dedication for the benefit
  of the public at large and to the detriment of our heirs and
  successors. We intend this dedication to be an overt act of
  relinquishment in perpetuity of all present and future rights to this
  software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.
*/