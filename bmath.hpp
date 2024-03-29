/*
  bmath.hpp v0.32 - public domain math library by Blat Blatnik
  
  last updated February 2023

  NO WARRANTY IMPLIED - USE AT YOUR OWN RISK! For licence information see end of file.

  Primarily of interest to game developers and others who only need
  2D, 3D, and 4D vector/matrix math and nothing more. This library 
  is intended to be a lightweight alternative to GLM. Unlike GLM it
  does not go out of its way to provide GLSL equivalent functionality.
  Only the commonly used stuff is provided, things like sin(vec4) are not. 
  Just like GLM, this is a header-only library.

  This library provides:
  + 2D, 3D and 4D vectors, 2x2, 3x3, 4x4 matrices, generic to any type
  + quaternions, generic to any type
  + full suite of vector operators (unary: + - ~ binary: + - * / % & | ^ << >>)
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
  - packing functions (packDouble2x32, ..)
  - arbitrary vector swizzles
  - complete set of operators for matrices and quaternions
  - low-level optimization (simd, forceinline, ...)

  Most functions are implemented as templates in order to reduce code duplication.

  Matrices are stored in a column-major memory order to keep compatible with OpenGL.

  C++11 features (constexpr, log2, exp2) are used when compiler support is detected.
  But the library will still work in C++98.

  ===================
  ----- Options -----
  ===================

  #define BMATH_NAMESPACE [your-custom-namespace]
  - Place all type definitions and functions in a namespace of your choosing. By
    default everything is defined in the global namespace

  #define BMATH_DEPTH_CLIP_ZERO_TO_ONE
  - By default the projection matrix functions assume depth values are clipped when
    they are outside [-1,+1] which is the default in OpenGL. Using this option the
    projection matrices will clip depth values outside of [0,1] which is the default
    in Vulkan and Direct3D

  #define BMATH_LEFT_HANDED
  - By default the projection and view matrices (orthoMat,lookAtMat) use a left-handed
    coordinate system. You can switch to right-handed coordinates.

  #define BMATH_NO_CPP11
  - Don't use C++ 11 features: constexpr and log2, exp2 from <cmath>

  Either #define these before including the file, or just uncomment the lines below.
*/

//#define BMATH_NAMESPACE [your-custom-namespace]
//#define BMATH_LEFT_HANDED
//#define BMATH_DEPTH_CLIP_ZERO_TO_ONE
//#define BMATH_NO_CPP11

#pragma once
#ifndef BMATH_H
#define BMATH_H

#include <cmath>

#ifdef BMATH_NAMESPACE
#	define BMATH_BEGIN namespace BMATH_NAMESPACE {
#	define BMATH_END }
#else
#	define BMATH_BEGIN 
#	define BMATH_END
#endif

#if !defined BMATH_DEPTH_CLIP_ZERO_TO_ONE && !defined BMATH_DEPTH_CLIP_MINUS_ONE_TO_ONE
#	define BMATH_DEPTH_CLIP_MINUS_ONE_TO_ONE
#endif
#if !defined BMATH_LEFT_HANDED && !defined BMATH_RIGHT_HANDED
#	define BMATH_RIGHT_HANDED
#endif

#ifndef BMATH_NO_CPP11
#	if defined _MSC_VER
#		if _MSC_VER >= 1900
#			define BMATH_HAS_CONSTEXPR
#		endif
#		if _MSC_VER >= 1800
#			define BMATH_HAS_DEFAULT_CONSTRUCTOR
#		endif
#		if _MSC_VER >= 1600
#			define BMATH_HAS_EXP2_LOG2
#		endif
#	elif defined __GNUC__ || defined __MINGW32__
#		if (__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4
#			define BMATH_HAS_CONSTEXPR
#		endif
#		if (__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4
#			define BMATH_HAS_DEFAULT_CONSTRUCTOR
#		endif
#		define BMATH_HAS_EXP2_LOG2
#	elif defined __clang__
#		if (__clang_major__ == 3 && __clang_minor__ >= 1) || __clang_major__ > 3
#			define BMATH_HAS_CONSTEXPR
#		endif
#		if (__clang_major__ == 3 && __clang_minor__ >= 0) || __clang_major__ > 3
#			define BMATH_HAS_DEFAULT_CONSTRUCTOR
#		endif
#		define BMATH_HAS_EXP2_LOG2
#	elif __cplusplus >= 201103L
#		define BMATH_HAS_CONSTEXPR
#		define BMATH_HAS_DEFAULT_CONSTRUCTOR
#		define BMATH_HAS_EXP2_LOG2
#	endif
#endif // !BMATH_NO_CPP11

#ifdef BMATH_HAS_CONSTEXPR
#	define BMATH_CONSTEXPR constexpr
#else
#	define BMATH_CONSTEXPR
#endif

BMATH_BEGIN

#ifdef BMATH_HAS_CONSTEXPR
BMATH_CONSTEXPR float  PI   = 3.141592741f;
BMATH_CONSTEXPR double PI64 = 3.141592653589793;
#endif

// Type Declarations

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

// Type Definitions

// disable warning: nonstandard extension used nameless struct/union
#if defined _MSC_VER
#	pragma warning(push)
#	pragma warning(disable: 4201)
#elif defined __GNUC__ || defined __MINGW32__
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wpedantic"
#elif defined __clang__
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#	pragma clang diagnostic ignored "-Wnested-anon-types"
#endif

template<class T> 
struct vector<T, 2> {

	union {
		struct { T x, y; };
		struct { T r, g; };
		struct { T u, v; };
		T elem[2];
	};

#ifdef BMATH_HAS_DEFAULT_CONSTRUCTOR
	inline vector() = default;
	inline BMATH_CONSTEXPR vector(const vector &v) = default;
#else
	inline vector() {}
	inline BMATH_CONSTEXPR vector(const vector &v)
		: x(v.x), y(v.y) {};
#endif

	template<class X, class Y> 
	inline BMATH_CONSTEXPR vector(X x, Y y)
		: x(T(x)), y(T(y)) {}

	template<class XY> 
	inline BMATH_CONSTEXPR explicit vector(XY xy)
		: x(T(xy)), y(T(xy)) {}
	
	template<class XY> 
	inline BMATH_CONSTEXPR explicit vector(vector<XY, 2> xy)
		: x(T(xy.x)), y(T(xy.y)) {}
	
	template<class XY>
	inline BMATH_CONSTEXPR explicit vector(vector<XY, 3> xy)
		: x(T(xy.x)), y(T(xy.y)) {}

	template<class XY>
	inline BMATH_CONSTEXPR explicit vector(vector<XY, 4> xy)
		: x(T(xy.x)), y(T(xy.y)) {}

	inline T &operator[](int index) {
		return elem[index];
	}
	inline BMATH_CONSTEXPR const T &operator[](int index) const {
		return elem[index];
	}
};

template<class T> 
struct vector<T, 3> {

	union {
		struct { T x, y, z; };
		struct { T r, g, b; };
		vector<T, 2> xy;
		T elem[3];
	};

#ifdef BMATH_HAS_DEFAULT_CONSTRUCTOR
	inline vector() = default;
	inline BMATH_CONSTEXPR vector(const vector & v) = default;
#else
	inline vector() {}
	inline BMATH_CONSTEXPR vector(const vector &v)
		: x(v.x), y(v.y), z(v.z) {};
#endif

	template<class X, class Y, class Z> 
	inline BMATH_CONSTEXPR vector(X x, Y y, Z z)
		: x(T(x)), y(T(y)), z(T(z)) {}
	
	template<class XY, class Z> 
	inline BMATH_CONSTEXPR vector(vector<XY, 2> xy, Z z)
		: x(T(xy.x)), y(T(xy.y)), z(T(z)) {}
	
	template<class X, class YZ> 
	inline BMATH_CONSTEXPR vector(X x, vector<YZ, 2> yz)
		: x(T(x)), y(T(yz.x)), z(T(yz.y)) {}
	
	template<class XYX> 
	inline BMATH_CONSTEXPR explicit vector(XYX xyz)
		: x(T(xyz)), y(T(xyz)), z(T(xyz)) {}
	
	template<class XY> 
	inline BMATH_CONSTEXPR explicit vector(vector<XY, 2> xy)
		: x(T(xy.x)), y(T(xy.y)), z(T(0)) {}
	
	template<class XYZ>
	inline BMATH_CONSTEXPR explicit vector(vector<XYZ, 3> xyz)
		: x(T(xyz.x)), y(T(xyz.y)), z(T(xyz.z)) {}
	
	template<class XYZ>
	inline BMATH_CONSTEXPR explicit vector(vector<XYZ, 4> xyz)
		: x(T(xyz.x)), y(T(xyz.y)), z(T(xyz.z)) {}

	inline T &operator[](int index) {
		return elem[index];
	}
	inline BMATH_CONSTEXPR const T &operator[](int index) const {
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
		struct { vector<T, 2> xy, zw; };
		T elem[4];
	};

#ifdef BMATH_HAS_DEFAULT_CONSTRUCTOR
	inline vector() = default;
	inline BMATH_CONSTEXPR vector(const vector & v) = default;
#else
	inline vector() {}
	inline BMATH_CONSTEXPR vector(const vector &v)
		: x(v.x), y(v.y), z(v.z), w(v.w) {};
#endif

	template<class X, class Y, class Z, class W> 
	inline BMATH_CONSTEXPR vector(X x, Y y, Z z, W w)
		: x(T(x)), y(T(y)), z(T(z)), w(T(w)) {}
	
	template<class XY, class Z, class W> 
	inline BMATH_CONSTEXPR vector(vector<XY, 2> xy, Z z, W w) 
		: x(T(xy.x)), y(T(xy.y)), z(T(z)), w(T(w)) {}
	
	template<class X, class YZ, class W> 
	inline BMATH_CONSTEXPR vector(X x, vector<YZ, 2> yz, W w)
		: x(T(x)), y(T(yz.x)), z(T(yz.y)), w(T(w)) {}
	
	template<class X, class Y, class ZW> 
	inline BMATH_CONSTEXPR vector(X x, Y y, vector<ZW, 2> zw)
		: x(T(x)), y(T(y)), z(T(zw.x)), w(T(zw.y)) {}
	
	template<class XY, class ZW> 
	inline BMATH_CONSTEXPR vector(vector<XY, 2> xy, vector<ZW, 2> zw)
		: x(T(xy.x)), y(T(xy.y)), z(T(zw.x)), w(T(zw.y)) {}
	
	template<class XYZ, class W> 
	inline BMATH_CONSTEXPR vector(vector<XYZ, 3> xyz, W w)
		: x(T(xyz.x)), y(T(xyz.y)), z(T(xyz.z)), w(T(w)) {}
	
	template<class X, class YZW> 
	inline BMATH_CONSTEXPR vector(X x, vector<YZW, 3> yzw)
		: x(T(x)), y(T(yzw.x)), z(T(yzw.y)), w(T(yzw.z)) {}
	
	template<class XYZW> 
	inline BMATH_CONSTEXPR explicit vector(XYZW xyzw)
		: x(T(xyzw)), y(T(xyzw)), z(T(xyzw)), w(T(xyzw)) {}
	
	template<class XY> 
	inline BMATH_CONSTEXPR explicit vector(vector<XY, 2> xy)
		: x(T(xy.x)), y(T(xy.y)), z(T(0)), w(T(0)) {}
	
	template<class XYZ> 
	inline BMATH_CONSTEXPR explicit vector(vector<XYZ, 3> xyz)
		: x(T(xyz.x)), y(T(xyz.y)), z(T(xyz.z)), w(T(0)) {}
	
	template<class XYZW> 
	inline BMATH_CONSTEXPR explicit vector(vector<XYZW, 4> xyzw)
		: x(T(xyzw.x)), y(T(xyzw.y)), z(T(xyzw.z)), w(T(xyzw.w)) {}

	inline T &operator[](int index) {
		return elem[index];
	}
	inline BMATH_CONSTEXPR const T &operator[](int index) const {
		return elem[index];
	}
};

template<class T> 
struct matrix<T, 2, 2> {

	vector<T, 2> col[2];

#ifdef BMATH_HAS_DEFAULT_CONSTRUCTOR
	inline matrix() = default;
	inline BMATH_CONSTEXPR matrix(const matrix &m) = default;
#else
	inline matrix() {}
	inline BMATH_CONSTEXPR matrix(const matrix &m)
		: col{ m.col[0], m.col[1] } {};
#endif

	template<
		class X0, class Y0,
		class X1, class Y1>
	inline BMATH_CONSTEXPR matrix(
		X0 x0, Y0 y0,
		X1 x1, Y1 y1)
		: col{
			vector<T, 2>(x0, y0),
			vector<T, 2>(x1, y1) } {}

	template<
		class C0,
		class C1>
	inline BMATH_CONSTEXPR matrix(
		vector<C0, 2> col0,
		vector<C1, 2> col1)
		: col{ col0, col1 } {}

	template<class M22> 
	inline BMATH_CONSTEXPR explicit matrix(matrix<M22, 2, 2> m)
		: col{ 
			vector<T, 2>(m.col[0]), 
			vector<T, 2>(m.col[1]) } {}

	template<class M33> 
	inline BMATH_CONSTEXPR explicit matrix(matrix<M33, 3, 3> m)
		: col{ 
			vector<T, 2>(m.col[0].xy), 
			vector<T, 2>(m.col[1].xy) } {}

	template<class M44> 
	inline BMATH_CONSTEXPR explicit matrix(matrix<M44, 4, 4> m)
		: col{ 
			vector<T, 2>(m.col[0].xy), 
			vector<T, 2>(m.col[1].xy) } {}

	template<class D> 
	inline BMATH_CONSTEXPR explicit matrix(vector<D, 2> diag)
		: col{
			vector<T, 2>(diag.x,      0), 
			vector<T, 2>(     0, diag.y) } {}
	
	template<class D> 
	inline BMATH_CONSTEXPR explicit matrix(D diag)
		: col{
			vector<T, 2>(diag,    0),
			vector<T, 2>(   0, diag) } {}

	inline vector<T, 2> &operator[](int index) {
		return col[index];
	}
	inline BMATH_CONSTEXPR const vector<T, 2> &operator[](int index) const {
		return col[index];
	}
};

template<class T> 
struct matrix<T, 3, 3> {

	vector<T, 3> col[3];

#ifdef BMATH_HAS_DEFAULT_CONSTRUCTOR
	inline matrix() = default;
	inline BMATH_CONSTEXPR matrix(const matrix & m) = default;
#else
	inline matrix() {}
	inline BMATH_CONSTEXPR matrix(const matrix &m)
		: col{ m.col[0], m.col[1], m.col[2] } {};
#endif

	template<
		class X0, class Y0, class Z0,
		class X1, class Y1, class Z1,
		class X2, class Y2, class Z2>
	inline BMATH_CONSTEXPR matrix(
		X0 x0, Y0 y0, Z0 z0,
		X1 x1, Y1 y1, Z1 z1,
		X2 x2, Y2 y2, Z2 z2)
		: col{
			vector<T, 3>(x0, y0, z0),
			vector<T, 3>(x1, y1, z1),
			vector<T, 3>(x2, y2, z2)} {}

	template<
		class C0,
		class C1,
		class C2>
	inline BMATH_CONSTEXPR matrix(
		vector<C0, 3> col0,
		vector<C1, 3> col1,
		vector<C2, 3> col2)
		: col{ 
			vector<T, 3>(col0), 
			vector<T, 3>(col1), 
			vector<T, 3>(col2) } {}

	template<class M22> 
	inline BMATH_CONSTEXPR explicit matrix(matrix<M22, 2, 2> m)
		: col{
			vector<T, 3>(m.col[0], 0),
			vector<T, 3>(m.col[1], 0),
			vector<T, 3>(0,   0,   1) } {}
	
	template<class M33> 
	inline BMATH_CONSTEXPR explicit matrix(matrix<M33, 3, 3> m)
		: col{ 
			vector<T, 3>(m.col[0]), 
			vector<T, 3>(m.col[1]), 
			vector<T, 3>(m.col[2]) } {}
	
	template<class M44> 
	inline BMATH_CONSTEXPR explicit matrix(matrix<M44, 4, 4> m)
		: col{ 
			vector<T, 3>(m.col[0].xyz), 
			vector<T, 3>(m.col[1].xyz), 
			vector<T, 3>(m.col[2].xyz) } {}
	
	template<class D> 
	inline BMATH_CONSTEXPR explicit matrix(vector<D, 3> diag)
		: col{
			vector<T, 3>(diag.x,      0,      0),
			vector<T, 3>(     0, diag.y,      0),
			vector<T, 3>(     0,      0, diag.z) } {}
	
	template<class D> 
	inline BMATH_CONSTEXPR explicit matrix(D diag)
		: col{
			vector<T, 3>(diag,    0,    0),
			vector<T, 3>(   0, diag,    0),
			vector<T, 3>(   0,    0, diag) } {}

	inline vector<T, 3> &operator[](int index) {
		return col[index];
	}
	inline BMATH_CONSTEXPR const vector<T, 3> &operator[](int index) const {
		return col[index];
	}
};

template<class T> 
struct matrix<T, 4, 4> {

	vector<T, 4> col[4];

#ifdef BMATH_HAS_DEFAULT_CONSTRUCTOR
	inline matrix() = default;
	inline BMATH_CONSTEXPR matrix(const matrix & m) = default;
#else
	inline matrix() {}
	inline BMATH_CONSTEXPR matrix(const matrix &m)
		: col{ m.col[0], m.col[1], m.col[2], m.col[3] } {};
#endif

	template<
		class X0, class Y0, class Z0, class W0,
		class X1, class Y1, class Z1, class W1,
		class X2, class Y2, class Z2, class W2,
		class X3, class Y3, class Z3, class W3>
	inline BMATH_CONSTEXPR matrix(
		X0 x0, Y0 y0, Z0 z0, W0 w0,
		X1 x1, Y1 y1, Z1 z1, W1 w1,
		X2 x2, Y2 y2, Z2 z2, W2 w2,
		X3 x3, Y3 y3, Z3 z3, W3 w3)
		: col{
			vector<T, 4>(x0, y0, z0, w0),
			vector<T, 4>(x1, y1, z1, w1),
			vector<T, 4>(x2, y2, z2, w2),
			vector<T, 4>(x3, y3, z3, w3) } {}

	template<
		class C0,
		class C1,
		class C2,
		class C3>
	inline BMATH_CONSTEXPR matrix(
		vector<C0, 4> col0,
		vector<C1, 4> col1,
		vector<C2, 4> col2,
		vector<C3, 4> col3)
		: col{ 
			vector<T, 4>(col0), 
			vector<T, 4>(col1), 
			vector<T, 4>(col2), 
			vector<T, 4>(col3) } {}

	template<class M22> 
	inline BMATH_CONSTEXPR explicit matrix(matrix<M22, 2, 2> m)
		: col{
			vector<T, 4>(m.col[0], 0, 0),
			vector<T, 4>(m.col[1], 0, 0),
			vector<T, 4>(0,   0,   1, 1),
			vector<T, 4>(0,   0,   0, 1) } {}

	template<class M33> 
	inline BMATH_CONSTEXPR explicit matrix(matrix<M33, 3, 3> m)
		: col{
			vector<T, 4>(m.col[0], 0),
			vector<T, 4>(m.col[1], 0),
			vector<T, 4>(m.col[2], 0),
			vector<T, 4>(0, 0, 0,  1) } {}

	template<class M44> 
	inline BMATH_CONSTEXPR explicit matrix(matrix<M44, 4, 4> m)
		: col{
			vector<T, 4>(m.col[0]),
			vector<T, 4>(m.col[1]),
			vector<T, 4>(m.col[2]),
			vector<T, 4>(m.col[3]) } {}

	template<class D> 
	inline BMATH_CONSTEXPR explicit matrix(vector<D, 4> diag)
		: col{
			vector<T, 4>(diag.x,      0,      0,      0),
			vector<T, 4>(     0, diag.y,      0,      0),
			vector<T, 4>(     0,      0, diag.z,      0),
			vector<T, 4>(     0,      0,      0, diag.w) } {}

	template<class D> 
	inline BMATH_CONSTEXPR explicit matrix(D diag)
		: col{
			vector<T, 4>(diag,    0,    0,    0),
			vector<T, 4>(   0, diag,    0,    0),
			vector<T, 4>(   0,    0, diag,    0),
			vector<T, 4>(   0,    0,    0, diag) } {}

	inline vector<T, 4> &operator[](int index) {
		return col[index];
	}
	inline BMATH_CONSTEXPR const vector<T, 4> &operator[](int index) const {
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

#ifdef BMATH_HAS_DEFAULT_CONSTRUCTOR
	inline quaternion() = default;
	inline BMATH_CONSTEXPR quaternion(const quaternion & q) = default;
#else
	inline quaternion() {}
	inline BMATH_CONSTEXPR quaternion(const quaternion &q)
		: x(q.x), y(q.y), z(q.z), w(q.w) {};
#endif

	template<class X, class Y, class Z, class W> 
	inline BMATH_CONSTEXPR quaternion(X x, Y y, Z z, W w)
		: x(T(x)), y(T(y)), z(T(z)), w(T(w)) {}
	
	template<class XYZ, class W> 
	inline BMATH_CONSTEXPR quaternion(vector<XYZ, 3> xyz, W w)
		: x(T(xyz.x)), y(T(xyz.y)), z(T(xyz.z)), w(T(w)) {}
	
	template<class XYZW> 
	inline BMATH_CONSTEXPR explicit quaternion(vector<XYZW, 4> xyzw)
		: x(T(xyzw.x)), y(T(xyzw.y)), z(T(xyzw.z)), w(T(xyzw.w)) {}
	
	template<class XYZW> 
	inline BMATH_CONSTEXPR explicit quaternion(quaternion<XYZW> q)
		: x(T(q.x)), y(T(q.y)), z(T(q.z)), w(T(q.w)) {}

	inline T &operator[](int index) {
		return elem[index];
	}
	inline BMATH_CONSTEXPR const T &operator[](int index) const {
		return elem[index];
	}
};

// nonstandard extension used: nameless struct/union
#if defined _MSC_VER
#	pragma warning(pop)
#elif defined __GNUC__ || defined __MINGW32__
#	pragma GCC diagnostic pop
#elif defined __clang__
#	pragma clang diagnostic pop
#endif

// Vector Operators

template<class T>
inline BMATH_CONSTEXPR vector<T, 2> operator +(vector<T, 2> v) {
	return vector<T, 2>(+v.x, +v.y);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 3> operator +(vector<T, 3> v) {
	return vector<T, 3>(+v.x, +v.y, +v.z);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 4> operator +(vector<T, 4> v) {
	return vector<T, 4>(+v.x, +v.y, +v.z, +v.w);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 2> operator -(vector<T, 2> v) {
	return vector<T, 2>(-v.x, -v.y);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 3> operator -(vector<T, 3> v) {
	return vector<T, 3>(-v.x, -v.y, -v.z);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 4> operator -(vector<T, 4> v) {
	return vector<T, 4>(-v.x, -v.y, -v.z, -v.w);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 2> operator ~(vector<T, 2> v) {
	return vector<T, 2>(~v.x, ~v.y);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 3> operator ~(vector<T, 3> v) {
	return vector<T, 3>(~v.x, ~v.y, ~v.z);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 4> operator ~(vector<T, 4> v) {
	return vector<T, 4>(~v.x, ~v.y, ~v.z, ~v.w);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 2> operator +(vector<T, 2> left, vector<T, 2> right) {
	return vector<T, 2>(
		left.x + right.x,
		left.y + right.y);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 3> operator +(vector<T, 3> left, vector<T, 3> right) {
	return vector<T, 3>(
		left.x + right.x,
		left.y + right.y,
		left.z + right.z);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 4> operator +(vector<T, 4> left, vector<T, 4> right) {
	return vector<T, 4>(
		left.x + right.x,
		left.y + right.y,
		left.z + right.z,
		left.w + right.w);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 2> operator -(vector<T, 2> left, vector<T, 2> right) {
	return vector<T, 2>(
		left.x - right.x,
		left.y - right.y);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 3> operator -(vector<T, 3> left, vector<T, 3> right) {
	return vector<T, 3>(
		left.x - right.x,
		left.y - right.y,
		left.z - right.z);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 4> operator -(vector<T, 4> left, vector<T, 4> right) {
	return vector<T, 4>(
		left.x - right.x,
		left.y - right.y,
		left.z - right.z,
		left.w - right.w);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 2> operator *(vector<T, 2> left, vector<T, 2> right) {
	return vector<T, 2>(
		left.x * right.x,
		left.y * right.y);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 3> operator *(vector<T, 3> left, vector<T, 3> right) {
	return vector<T, 3>(
		left.x * right.x,
		left.y * right.y,
		left.z * right.z);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 4> operator *(vector<T, 4> left, vector<T, 4> right) {
	return vector<T, 4>(
		left.x * right.x,
		left.y * right.y,
		left.z * right.z,
		left.w * right.w);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 2> operator /(vector<T, 2> left, vector<T, 2> right) {
	return vector<T, 2>(
		left.x / right.x,
		left.y / right.y);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 3> operator /(vector<T, 3> left, vector<T, 3> right) {
	return vector<T, 3>(
		left.x / right.x,
		left.y / right.y,
		left.z / right.z);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 4> operator /(vector<T, 4> left, vector<T, 4> right) {
	return vector<T, 4>(
		left.x / right.x,
		left.y / right.y,
		left.z / right.z,
		left.w / right.w);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 2> operator %(vector<T, 2> left, vector<T, 2> right) {
	return vector<T, 2>(
		left.x % right.x,
		left.y % right.y);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 3> operator %(vector<T, 3> left, vector<T, 3> right) {
	return vector<T, 3>(
		left.x % right.x,
		left.y % right.y,
		left.z % right.z);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 4> operator %(vector<T, 4> left, vector<T, 4> right) {
	return vector<T, 4>(
		left.x % right.x,
		left.y % right.y,
		left.z % right.z,
		left.w % right.w);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 2> operator &(vector<T, 2> left, vector<T, 2> right) {
	return vector<T, 2>(
		left.x & right.x,
		left.y & right.y);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 3> operator &(vector<T, 3> left, vector<T, 3> right) {
	return vector<T, 3>(
		left.x & right.x,
		left.y & right.y,
		left.z & right.z);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 4> operator &(vector<T, 4> left, vector<T, 4> right) {
	return vector<T, 4>(
		left.x & right.x,
		left.y & right.y,
		left.z & right.z,
		left.w & right.w);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 2> operator |(vector<T, 2> left, vector<T, 2> right) {
	return vector<T, 2>(
		left.x | right.x,
		left.y | right.y);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 3> operator |(vector<T, 3> left, vector<T, 3> right) {
	return vector<T, 3>(
		left.x | right.x,
		left.y | right.y,
		left.z | right.z);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 4> operator |(vector<T, 4> left, vector<T, 4> right) {
	return vector<T, 4>(
		left.x | right.x,
		left.y | right.y,
		left.z | right.z,
		left.w | right.w);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 2> operator ^(vector<T, 2> left, vector<T, 2> right) {
	return vector<T, 2>(
		left.x ^ right.x,
		left.y ^ right.y);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 3> operator ^(vector<T, 3> left, vector<T, 3> right) {
	return vector<T, 3>(
		left.x ^ right.x,
		left.y ^ right.y,
		left.z ^ right.z);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 4> operator ^(vector<T, 4> left, vector<T, 4> right) {
	return vector<T, 4>(
		left.x ^ right.x,
		left.y ^ right.y,
		left.z ^ right.z,
		left.w ^ right.w);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 2> operator <<(vector<T, 2> left, vector<T, 2> right) {
	return vector<T, 2>(
		left.x << right.x,
		left.y << right.y);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 3> operator <<(vector<T, 3> left, vector<T, 3> right) {
	return vector<T, 3>(
		left.x << right.x,
		left.y << right.y,
		left.z << right.z);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 4> operator <<(vector<T, 4> left, vector<T, 4> right) {
	return vector<T, 4>(
		left.x << right.x,
		left.y << right.y,
		left.z << right.z,
		left.w << right.w);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 2> operator >>(vector<T, 2> left, vector<T, 2> right) {
	return vector<T, 2>(
		left.x >> right.x,
		left.y >> right.y);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 3> operator >>(vector<T, 3> left, vector<T, 3> right) {
	return vector<T, 3>(
		left.x >> right.x,
		left.y >> right.y,
		left.z >> right.z);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 4> operator >>(vector<T, 4> left, vector<T, 4> right) {
	return vector<T, 4>(
		left.x >> right.x,
		left.y >> right.y,
		left.z >> right.z,
		left.w >> right.w);
}

template<class T>
inline BMATH_CONSTEXPR vector<bool, 2> operator ==(vector<T, 2> left, vector<T, 2> right) {
	return vector<bool, 2>(
		left.x == right.x,
		left.y == right.y);
}

template<class T>
inline BMATH_CONSTEXPR vector<bool, 3> operator ==(vector<T, 3> left, vector<T, 3> right) {
	return vector<bool, 3>(
		left.x == right.x,
		left.y == right.y,
		left.z == right.z);
}

template<class T>
inline BMATH_CONSTEXPR vector<bool, 4> operator ==(vector<T, 4> left, vector<T, 4> right) {
	return vector<bool, 4>(
		left.x == right.x,
		left.y == right.y,
		left.z == right.z,
		left.w == right.w);
}

template<class T>
inline BMATH_CONSTEXPR vector<bool, 2> operator !=(vector<T, 2> left, vector<T, 2> right) {
	return vector<bool, 2>(
		left.x != right.x,
		left.y != right.y);
}

template<class T>
inline BMATH_CONSTEXPR vector<bool, 3> operator !=(vector<T, 3> left, vector<T, 3> right) {
	return vector<bool, 3>(
		left.x != right.x,
		left.y != right.y,
		left.z != right.z);
}

template<class T>
inline BMATH_CONSTEXPR vector<bool, 4> operator !=(vector<T, 4> left, vector<T, 4> right) {
	return vector<bool, 4>(
		left.x != right.x,
		left.y != right.y,
		left.z != right.z,
		left.w != right.w);
}

template<class T>
inline BMATH_CONSTEXPR vector<bool, 2> operator >=(vector<T, 2> left, vector<T, 2> right) {
	return vector<bool, 2>(
		left.x >= right.x,
		left.y >= right.y);
}

template<class T>
inline BMATH_CONSTEXPR vector<bool, 3> operator >=(vector<T, 3> left, vector<T, 3> right) {
	return vector<bool, 3>(
		left.x >= right.x,
		left.y >= right.y,
		left.z >= right.z);
}

template<class T>
inline BMATH_CONSTEXPR vector<bool, 4> operator >=(vector<T, 4> left, vector<T, 4> right) {
	return vector<bool, 4>(
		left.x >= right.x,
		left.y >= right.y,
		left.z >= right.z,
		left.w >= right.w);
}

template<class T>
inline BMATH_CONSTEXPR vector<bool, 2> operator <=(vector<T, 2> left, vector<T, 2> right) {
	return vector<bool, 2>(
		left.x <= right.x,
		left.y <= right.y);
}

template<class T>
inline BMATH_CONSTEXPR vector<bool, 3> operator <=(vector<T, 3> left, vector<T, 3> right) {
	return vector<bool, 3>(
		left.x <= right.x,
		left.y <= right.y,
		left.z <= right.z);
}

template<class T>
inline BMATH_CONSTEXPR vector<bool, 4> operator <=(vector<T, 4> left, vector<T, 4> right) {
	return vector<bool, 4>(
		left.x <= right.x,
		left.y <= right.y,
		left.z <= right.z,
		left.w <= right.w);
}

template<class T>
inline BMATH_CONSTEXPR vector<bool, 2> operator >(vector<T, 2> left, vector<T, 2> right) {
	return vector<bool, 2>(
		left.x > right.x,
		left.y > right.y);
}

template<class T>
inline BMATH_CONSTEXPR vector<bool, 3> operator >(vector<T, 3> left, vector<T, 3> right) {
	return vector<bool, 3>(
		left.x > right.x,
		left.y > right.y,
		left.z > right.z);
}

template<class T>
inline BMATH_CONSTEXPR vector<bool, 4> operator >(vector<T, 4> left, vector<T, 4> right) {
	return vector<bool, 4>(
		left.x > right.x,
		left.y > right.y,
		left.z > right.z,
		left.w > right.w);
}

template<class T>
inline BMATH_CONSTEXPR vector<bool, 2> operator <(vector<T, 2> left, vector<T, 2> right) {
	return vector<bool, 2>(
		left.x < right.x,
		left.y < right.y);
}

template<class T>
inline BMATH_CONSTEXPR vector<bool, 3> operator <(vector<T, 3> left, vector<T, 3> right) {
	return vector<bool, 3>(
		left.x < right.x,
		left.y < right.y,
		left.z < right.z);
}

template<class T>
inline BMATH_CONSTEXPR vector<bool, 4> operator <(vector<T, 4> left, vector<T, 4> right) {
	return vector<bool, 4>(
		left.x < right.x,
		left.y < right.y,
		left.z < right.z,
		left.w < right.w);
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> operator +(vector<T, N> left, T right) {
	return left + vector<T, N>(right);
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> operator -(vector<T, N> left, T right) {
	return left - vector<T, N>(right);
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> operator *(vector<T, N> left, T right) {
	return left * vector<T, N>(right);
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> operator /(vector<T, N> left, T right) {
	return left / vector<T, N>(right);
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> operator %(vector<T, N> left, T right) {
	return left % vector<T, N>(right);
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> operator &(vector<T, N> left, T right) {
	return left & vector<T, N>(right);
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> operator |(vector<T, N> left, T right) {
	return left | vector<T, N>(right);
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> operator ^(vector<T, N> left, T right) {
	return left ^ vector<T, N>(right);
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> operator <<(vector<T, N> left, T right) {
	return left << vector<T, N>(right);
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> operator >>(vector<T, N> left, T right) {
	return left >> vector<T, N>(right);
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<bool, N> operator ==(vector<T, N> left, T right) {
	return left == vector<T, N>(right);
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<bool, N> operator !=(vector<T, N> left, T right) {
	return left != vector<T, N>(right);
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<bool, N> operator >=(vector<T, N> left, T right) {
	return left >= vector<T, N>(right);
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<bool, N> operator <=(vector<T, N> left, T right) {
	return left <= vector<T, N>(right);
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<bool, N> operator >(vector<T, N> left, T right) {
	return left > vector<T, N>(right);
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<bool, N> operator <(vector<T, N> left, T right) {
	return left < vector<T, N>(right);
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> operator +(T left, vector<T, N> right) {
	return vector<T, N>(left) + right;
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> operator -(T left, vector<T, N> right) {
	return vector<T, N>(left) - right;
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> operator *(T left, vector<T, N> right) {
	return vector<T, N>(left) * right;
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> operator /(T left, vector<T, N> right) {
	return vector<T, N>(left) / right;
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> operator %(T left, vector<T, N> right) {
	return vector<T, N>(left) % right;
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> operator &(T left, vector<T, N> right) {
	return vector<T, N>(left) & right;
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> operator |(T left, vector<T, N> right) {
	return vector<T, N>(left) | right;
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> operator ^(T left, vector<T, N> right) {
	return vector<T, N>(left) ^ right;
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> operator <<(T left, vector<T, N> right) {
	return vector<T, N>(left) << right;
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> operator >>(T left, vector<T, N> right) {
	return vector<T, N>(left) >> right;
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<bool, N> operator ==(T left, vector<T, N> right) {
	return vector<T, N>(left) == right;
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<bool, N> operator !=(T left, vector<T, N> right) {
	return vector<T, N>(left) != right;
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<bool, N> operator >=(T left, vector<T, N> right) {
	return vector<T, N>(left) >= right;
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<bool, N> operator <=(T left, vector<T, N> right) {
	return vector<T, N>(left) <= right;
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<bool, N> operator >(T left, vector<T, N> right) {
	return vector<T, N>(left) > right;
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<bool, N> operator <(T left, vector<T, N> right) {
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

template<class T, int N>
inline vector<T, N> operator --(vector<T, N> &v) {
	return v -= T(1);
}

template<class T, int N>
inline vector<T, N> operator --(vector<T, N> &v, int) {
	vector<T, N> copy = v;
	v -= T(1);
	return copy;
}

// Matrix Operators

template<class T>
inline BMATH_CONSTEXPR matrix<T, 2, 2> operator +(matrix<T, 2, 2> m) {
	return matrix<T, 2, 2>(
		+m.col[0],
		+m.col[1]);
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 3, 3> operator +(matrix<T, 3, 3> m) {
	return matrix<T, 3, 3>(
		+m.col[0],
		+m.col[1],
		+m.col[2]);
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 4, 4> operator +(matrix<T, 4, 4> m) {
	return matrix<T, 4, 4>(
		+m.col[0],
		+m.col[1],
		+m.col[2],
		+m.col[3]);
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 2, 2> operator -(matrix<T, 2, 2> m) {
	return matrix<T, 2, 2>(
		-m.col[0],
		-m.col[1]);
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 3, 3> operator -(matrix<T, 3, 3> m) {
	return matrix<T, 3, 3>(
		-m.col[0],
		-m.col[1],
		-m.col[2]);
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 4, 4> operator -(matrix<T, 4, 4> m) {
	return matrix<T, 4, 4>(
		-m.col[0],
		-m.col[1],
		-m.col[2],
		-m.col[3]);
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 2, 2> operator +(matrix<T, 2, 2> left, matrix<T, 2, 2> right) {
	return matrix<T, 2, 2>(
		left.col[0] + right.col[0],
		left.col[1] + right.col[1]);
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 3, 3> operator +(matrix<T, 3, 3> left, matrix<T, 3, 3> right) {
	return matrix<T, 3, 3>(
		left.col[0] + right.col[0],
		left.col[1] + right.col[1],
		left.col[2] + right.col[2]);
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 4, 4> operator +(matrix<T, 4, 4> left, matrix<T, 4, 4> right) {
	return matrix<T, 4, 4>(
		left.col[0] + right.col[0],
		left.col[1] + right.col[1],
		left.col[2] + right.col[2],
		left.col[3] + right.col[3]);
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 2, 2> operator -(matrix<T, 2, 2> left, matrix<T, 2, 2> right) {
	return matrix<T, 2, 2>(
		left.col[0] - right.col[0],
		left.col[1] - right.col[1]);
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 3, 3> operator -(matrix<T, 3, 3> left, matrix<T, 3, 3> right) {
	return matrix<T, 3, 3>(
		left.col[0] - right.col[0],
		left.col[1] - right.col[1],
		left.col[2] - right.col[2]);
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 4, 4> operator -(matrix<T, 4, 4> left, matrix<T, 4, 4> right) {
	return matrix<T, 4, 4>(
		left.col[0] - right.col[0],
		left.col[1] - right.col[1],
		left.col[2] - right.col[2],
		left.col[3] - right.col[3]);
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 2, 2> operator *(matrix<T, 2, 2> left, matrix<T, 2, 2> right) {
	return matrix<T, 2, 2>(

		left.col[0].x * right.col[0].x + left.col[1].x * right.col[0].y,
		left.col[0].y * right.col[0].x + left.col[1].y * right.col[0].y,

		left.col[0].x * right.col[1].x + left.col[1].x * right.col[1].y,
		left.col[0].y * right.col[1].x + left.col[1].y * right.col[1].y);
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 3, 3> operator *(matrix<T, 3, 3> left, matrix<T, 3, 3> right) {
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
inline BMATH_CONSTEXPR matrix<T, 4, 4> operator *(matrix<T, 4, 4> left, matrix<T, 4, 4> right) {
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
inline BMATH_CONSTEXPR vector<T, 2> operator *(matrix<T, 2, 2> left, vector<T, 2> right) {
	return vector<T, 2>(
		left.col[0].x * right.x + left.col[1].x * right.y,
		left.col[0].y * right.x + left.col[1].y * right.y);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 3> operator *(matrix<T, 3, 3> left, vector<T, 3> right) {
	return vector<T, 3>(
		left.col[0].x * right.x + left.col[1].x * right.y + left.col[2].x * right.z,
		left.col[0].y * right.x + left.col[1].y * right.y + left.col[2].y * right.z,
		left.col[0].z * right.x + left.col[1].z * right.y + left.col[2].z * right.z);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 4> operator *(matrix<T, 4, 4> left, vector<T, 4> right) {
	return vector<T, 4>(
		left.col[0].x * right.x + left.col[1].x * right.y + left.col[2].x * right.z + left.col[3].x * right.w,
		left.col[0].y * right.x + left.col[1].y * right.y + left.col[2].y * right.z + left.col[3].y * right.w,
		left.col[0].z * right.x + left.col[1].z * right.y + left.col[2].z * right.z + left.col[3].z * right.w,
		left.col[0].w * right.x + left.col[1].w * right.y + left.col[2].w * right.z + left.col[3].w * right.w);
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 2, 2> operator /(matrix<T, 2, 2> left, matrix<T, 2, 2> right) {
	return matrix<T, 2, 2>(
		left.col[0] / right.col[0],
		left.col[1] / right.col[1]);
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 3, 3> operator /(matrix<T, 3, 3> left, matrix<T, 3, 3> right) {
	return matrix<T, 3, 3>(
		left.col[0] / right.col[0],
		left.col[1] / right.col[1],
		left.col[2] / right.col[2]);
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 4, 4> operator /(matrix<T, 4, 4> left, matrix<T, 4, 4> right) {
	return matrix<T, 4, 4>(
		left.col[0] / right.col[0],
		left.col[1] / right.col[1],
		left.col[2] / right.col[2],
		left.col[3] / right.col[3]);
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 2, 2> operator +(matrix<T, 2, 2> left, T right) {
	return left + matrix<T, 2, 2>(
		vector<T, 2>(right),
		vector<T, 2>(right));
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 3, 3> operator +(matrix<T, 3, 3> left, T right) {
	return left + matrix<T, 3, 3>(
		vector<T, 3>(right),
		vector<T, 3>(right),
		vector<T, 3>(right));
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 4, 4> operator +(matrix<T, 4, 4> left, T right) {
	return left + matrix<T, 4, 4>(
		vector<T, 4>(right),
		vector<T, 4>(right),
		vector<T, 4>(right),
		vector<T, 4>(right));
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 2, 2> operator -(matrix<T, 2, 2> left, T right) {
	return left - matrix<T, 2, 2>(
		vector<T, 2>(right),
		vector<T, 2>(right));
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 3, 3> operator -(matrix<T, 3, 3> left, T right) {
	return left - matrix<T, 3, 3>(
		vector<T, 3>(right),
		vector<T, 3>(right),
		vector<T, 3>(right));
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 4, 4> operator -(matrix<T, 4, 4> left, T right) {
	return left - matrix<T, 4, 4>(
		vector<T, 4>(right),
		vector<T, 4>(right),
		vector<T, 4>(right),
		vector<T, 4>(right));
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 2, 2> operator *(matrix<T, 2, 2> left, T right) {
	return matrix<T, 2, 2>(
		left.col[0] * vector<T, 2>(right),
		left.col[1] * vector<T, 2>(right));
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 3, 3> operator *(matrix<T, 3, 3> left, T right) {
	return matrix<T, 3, 3>(
		left.col[0] * vector<T, 3>(right),
		left.col[1] * vector<T, 3>(right),
		left.col[2] * vector<T, 3>(right));
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 4, 4> operator *(matrix<T, 4, 4> left, T right) {
	return matrix<T, 4, 4>(
		left.col[0] * vector<T, 4>(right),
		left.col[1] * vector<T, 4>(right),
		left.col[2] * vector<T, 4>(right),
		left.col[3] * vector<T, 4>(right));
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 2, 2> operator /(matrix<T, 2, 2> left, T right) {
	return left / matrix<T, 2, 2>(
		vector<T, 2>(right),
		vector<T, 2>(right));
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 3, 3> operator /(matrix<T, 3, 3> left, T right) {
	return left / matrix<T, 3, 3>(
		vector<T, 3>(right),
		vector<T, 3>(right),
		vector<T, 3>(right));
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 4, 4> operator /(matrix<T, 4, 4> left, T right) {
	return left / matrix<T, 4, 4>(
		vector<T, 4>(right),
		vector<T, 4>(right),
		vector<T, 4>(right),
		vector<T, 4>(right));
}

template<class T, int C, int R>
inline BMATH_CONSTEXPR matrix<T, C, R> operator +(T left, matrix<T, C, R> right) {
	return right + left;
}

template<class T, int C, int R>
inline BMATH_CONSTEXPR matrix<T, C, R> operator *(T left, matrix<T, C, R> right) {
	return right * left;
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 2, 2> operator -(T left, matrix<T, 2, 2> right) {
	return matrix<T, 2, 2>(
		vector<T, 2>(left),
		vector<T, 2>(left)) - right;
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 3, 3> operator -(T left, matrix<T, 3, 3> right) {
	return matrix<T, 3, 3>(
		vector<T, 3>(left),
		vector<T, 3>(left),
		vector<T, 3>(left)) - right;
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 4, 4> operator -(T left, matrix<T, 4, 4> right) {
	return matrix<T, 4, 4>(
		vector<T, 4>(left),
		vector<T, 4>(left),
		vector<T, 4>(left),
		vector<T, 4>(left)) - right;
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 2, 2> operator /(T left, matrix<T, 2, 2> right) {
	return matrix<T, 2, 2>(
		vector<T, 2>(left),
		vector<T, 2>(left)) / right;
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 3, 3> operator /(T left, matrix<T, 3, 3> right) {
	return matrix<T, 3, 3>(
		vector<T, 3>(left),
		vector<T, 3>(left),
		vector<T, 3>(left)) / right;
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 4, 4> operator /(T left, matrix<T, 4, 4> right) {
	return matrix<T, 4, 4>(
		vector<T, 4>(left),
		vector<T, 4>(left),
		vector<T, 4>(left),
		vector<T, 4>(left)) / right;
}

template<class T>
inline BMATH_CONSTEXPR bool operator ==(matrix<T, 2, 2> left, matrix<T, 2, 2> right) {
	return
		left.col[0].x == right.col[0].x and
		left.col[0].y == right.col[0].y and
		left.col[1].x == right.col[1].x and
		left.col[1].y == right.col[1].y;
}

template<class T>
inline BMATH_CONSTEXPR bool operator ==(matrix<T, 3, 3> left, matrix<T, 3, 3> right) {
	return
		left.col[0].x == right.col[0].x and
		left.col[0].y == right.col[0].y and
		left.col[0].z == right.col[0].z and
		left.col[1].x == right.col[1].x and
		left.col[1].y == right.col[1].y and
		left.col[1].z == right.col[1].z and
		left.col[2].x == right.col[2].x and
		left.col[2].y == right.col[2].y and
		left.col[2].z == right.col[2].z;
}

template<class T>
inline BMATH_CONSTEXPR bool operator ==(matrix<T, 4, 4> left, matrix<T, 4, 4> right) {
	return
		left.col[0].x == right.col[0].x and
		left.col[0].y == right.col[0].y and
		left.col[0].z == right.col[0].z and
		left.col[0].w == right.col[0].w and
		left.col[1].x == right.col[1].x and
		left.col[1].y == right.col[1].y and
		left.col[1].z == right.col[1].z and
		left.col[1].w == right.col[1].w and
		left.col[2].x == right.col[2].x and
		left.col[2].y == right.col[2].y and
		left.col[2].z == right.col[2].z and
		left.col[2].w == right.col[2].w and
		left.col[3].x == right.col[3].x and
		left.col[3].y == right.col[3].y and
		left.col[3].z == right.col[3].z and
		left.col[3].w == right.col[3].w;
}

template<class T>
inline BMATH_CONSTEXPR bool operator !=(matrix<T, 2, 2> left, matrix<T, 2, 2> right) {
	return
		left.col[0].x != right.col[0].x or
		left.col[0].y != right.col[0].y or
		left.col[1].x != right.col[1].x or
		left.col[1].y != right.col[1].y;
}

template<class T>
inline BMATH_CONSTEXPR bool operator !=(matrix<T, 3, 3> left, matrix<T, 3, 3> right) {
	return
		left.col[0].x != right.col[0].x or
		left.col[0].y != right.col[0].y or
		left.col[0].z != right.col[0].z or
		left.col[1].x != right.col[1].x or
		left.col[1].y != right.col[1].y or
		left.col[1].z != right.col[1].z or
		left.col[2].x != right.col[2].x or
		left.col[2].y != right.col[2].y or
		left.col[2].z != right.col[2].z;
}

template<class T>
inline BMATH_CONSTEXPR bool operator !=(matrix<T, 4, 4> left, matrix<T, 4, 4> right) {
	return
		left.col[0].x != right.col[0].x or
		left.col[0].y != right.col[0].y or
		left.col[0].z != right.col[0].z or
		left.col[0].w != right.col[0].w or
		left.col[1].x != right.col[1].x or
		left.col[1].y != right.col[1].y or
		left.col[1].z != right.col[1].z or
		left.col[1].w != right.col[1].w or
		left.col[2].x != right.col[2].x or
		left.col[2].y != right.col[2].y or
		left.col[2].z != right.col[2].z or
		left.col[2].w != right.col[2].w or
		left.col[3].x != right.col[3].x or
		left.col[3].y != right.col[3].y or
		left.col[3].z != right.col[3].z or
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

// Quaternion Operators

template<class T>
inline BMATH_CONSTEXPR quaternion<T> operator +(quaternion<T> q) {
	return quaternion<T>(
		+q.x,
		+q.y,
		+q.z,
		+q.w);
}

template<class T>
inline BMATH_CONSTEXPR quaternion<T> operator -(quaternion<T> q) {
	return quaternion<T>(
		-q.x,
		-q.y,
		-q.z,
		-q.w);
}

template<class T> 
inline BMATH_CONSTEXPR quaternion<T> operator +(quaternion<T> left, quaternion<T> right) {
	return quaternion<T>(
		left.x + right.x,
		left.y + right.y,
		left.z + right.z,
		left.w + right.w);
}

template<class T>
inline BMATH_CONSTEXPR quaternion<T> operator -(quaternion<T> left, quaternion<T> right) {
	return quaternion<T>(
		left.x - right.x,
		left.y - right.y,
		left.z - right.z,
		left.w - right.w);
}

template<class T>
inline BMATH_CONSTEXPR quaternion<T> operator *(quaternion<T> left, quaternion<T> right) {
	return quaternion<T>(
		left.w * right.x + left.x * right.w + left.y * right.z - left.z * right.y,
		left.w * right.y - left.x * right.z + left.y * right.w + left.z * right.x,
		left.w * right.z + left.x * right.y - left.y * right.x + left.z * right.w,
		left.w * right.w - left.x * right.x - left.y * right.y - left.z * right.z);
}

template<class T>
inline BMATH_CONSTEXPR quaternion<T> operator /(quaternion<T> left, quaternion<T> right) {
	return quaternion<T>(
		left.x / right.x,
		left.y / right.y,
		left.z / right.z,
		left.w / right.w);
}

template<class T>
inline BMATH_CONSTEXPR quaternion<T> operator +(quaternion<T> left, T right) {
	return quaternion<T>(
		left.x + right,
		left.y + right,
		left.z + right,
		left.w + right);
}

template<class T>
inline BMATH_CONSTEXPR quaternion<T> operator -(quaternion<T> left, T right) {
	return quaternion<T>(
		left.x - right,
		left.y - right,
		left.z - right,
		left.w - right);
}

template<class T>
inline BMATH_CONSTEXPR quaternion<T> operator *(quaternion<T> left, T right) {
	return quaternion<T>(
		left.x * right,
		left.y * right,
		left.z * right,
		left.w * right);
}

template<class T>
inline BMATH_CONSTEXPR quaternion<T> operator /(quaternion<T> left, T right) {
	return quaternion<T>(
		left.x / right,
		left.y / right,
		left.z / right,
		left.w / right);
}

template<class T>
inline BMATH_CONSTEXPR quaternion<T> operator +(T left, quaternion<T> right) {
	return quaternion<T>(
		left + right.x,
		left + right.y,
		left + right.z,
		left + right.w);
}

template<class T>
inline BMATH_CONSTEXPR quaternion<T> operator -(T left, quaternion<T> right) {
	return quaternion<T>(
		left - right.x,
		left - right.y,
		left - right.z,
		left - right.w);
}

template<class T>
inline BMATH_CONSTEXPR quaternion<T> operator *(T left, quaternion<T> right) {
	return quaternion<T>(
		left * right.x,
		left * right.y,
		left * right.z,
		left * right.w);
}

template<class T>
inline BMATH_CONSTEXPR quaternion<T> operator /(T left, quaternion<T> right) {
	return quaternion<T>(
		left / right.x,
		left / right.y,
		left / right.z,
		left / right.w);
}

template<class T>
inline BMATH_CONSTEXPR vector<bool, 4> operator ==(quaternion<T> left, quaternion<T> right) {
	return vector<bool, 4>(
		left.x == right.x,
		left.y == right.y,
		left.z == right.z,
		left.w == right.w);
}

template<class T>
inline BMATH_CONSTEXPR vector<bool, 4> operator !=(quaternion<T> left, quaternion<T> right) {
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

// Trigonometric Functions

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
inline BMATH_CONSTEXPR T radians(T degrees) {
	return degrees * T(3.141592653589793) / T(180);
}

template<class T> 
inline BMATH_CONSTEXPR T degrees(T radians) {
	return radians * T(180) / T(3.141592653589793);
}

// Exponential Functions

using std::pow;
using std::exp;
using std::log;
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

#ifdef BMATH_HAS_EXP2_LOG2

using std::exp2;
using std::log2;

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

#endif

// Common Functions

using std::abs;
using std::floor;
using std::trunc;
using std::round;
using std::ceil;
using std::isnan;
using std::isinf;

template<class T>
inline BMATH_CONSTEXPR T max(T a, T b) {
	return a > b ? a : b;
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 2> max(vector<T, 2> a, vector<T, 2> b) {
	return vector<T, 2>(
		max(a.x, b.x),
		max(a.y, b.y));
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 3> max(vector<T, 3> a, vector<T, 3> b) {
	return vector<T, 3>(
		max(a.x, b.x),
		max(a.y, b.y),
		max(a.z, b.z));
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 4> max(vector<T, 4> a, vector<T, 4> b) {
	return vector<T, 4>(
		max(a.x, b.x),
		max(a.y, b.y),
		max(a.z, b.z),
		max(a.w, b.w));
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> max(vector<T, N> a, T b) {
	return max(a, vector<T, N>(b));
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> max(T a, vector<T, N> b) {
	return max(vector<T, N>(a), b);
}

template<class T>
inline BMATH_CONSTEXPR T min(T a, T b) {
	return a < b ? a : b;
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 2> min(vector<T, 2> a, vector<T, 2> b) {
	return vector<T, 2>(
		min(a.x, b.x),
		min(a.y, b.y));
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 3> min(vector<T, 3> a, vector<T, 3> b) {
	return vector<T, 3>(
		min(a.x, b.x),
		min(a.y, b.y),
		min(a.z, b.z));
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 4> min(vector<T, 4> a, vector<T, 4> b) {
	return vector<T, 4>(
		min(a.x, b.x),
		min(a.y, b.y),
		min(a.z, b.z),
		min(a.w, b.w));
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> min(vector<T, N> a, T b) {
	return min(a, vector<T, N>(b));
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> min(T a, vector<T, N> b) {
	return min(vector<T, N>(a), b);
}

template<class T>
inline BMATH_CONSTEXPR T clamp(T x, T minVal, T maxVal) {
	return min(max(x, minVal), maxVal);
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> clamp(vector<T, N> v, vector<T, N> minVal, vector<T, N> maxVal) {
	return min(max(v, minVal), maxVal);
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> clamp(vector<T, N> v, T minVal, T maxVal) {
	return min(max(v, minVal), maxVal);
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> saturate(vector<T, N> v) {
	return clamp(v, T(0), T(1));
}

template<class T>
inline BMATH_CONSTEXPR T compSum(vector<T, 2> v) {
	return v.x + v.y;
}

template<class T>
inline BMATH_CONSTEXPR T compSum(vector<T, 3> v) {
	return v.x + v.y + v.z;
}

template<class T>
inline BMATH_CONSTEXPR T compSum(vector<T, 4> v) {
	return v.x + v.y + v.z + v.w;
}

template<class T>
inline BMATH_CONSTEXPR T compMax(vector<T, 2> v) {
	return max(v.x, v.y);
}

template<class T>
inline BMATH_CONSTEXPR T compMax(vector<T, 3> v) {
	return max(v.x, max(v.y, v.z));
}

template<class T>
inline BMATH_CONSTEXPR T compMax(vector<T, 4> v) {
	return max(v.x, max(v.y, max(v.z, v.w)));
}

template<class T>
inline BMATH_CONSTEXPR T compMin(vector<T, 2> v) {
	return min(v.x, v.y);
}

template<class T>
inline BMATH_CONSTEXPR T compMin(vector<T, 3> v) {
	return min(v.x, min(v.y, v.z));
}

template<class T>
inline BMATH_CONSTEXPR T compMin(vector<T, 4> v) {
	return min(v.x, min(v.y, min(v.z, v.w)));
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 2> abs(vector<T, 2> v) {
	return vector<T, 2>(
		v.x < T(0) ? -v.x : v.x,
		v.y < T(0) ? -v.y : v.y);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 3> abs(vector<T, 3> v) {
	return vector<T, 3>(
		v.x < T(0) ? -v.x : v.x,
		v.y < T(0) ? -v.y : v.y,
		v.z < T(0) ? -v.z : v.z);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 4> abs(vector<T, 4> v) {
	return vector<T, 4>(
		v.x < T(0) ? -v.x : v.x,
		v.y < T(0) ? -v.y : v.y,
		v.z < T(0) ? -v.z : v.z,
		v.w < T(0) ? -v.w : v.w);
}

template<class T>
inline BMATH_CONSTEXPR T sign(T x) {
	return T(x > T(0)) - T(x < T(0));
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
	return x - trunc(x);
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
inline BMATH_CONSTEXPR T mod(T a, T b) {
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
inline BMATH_CONSTEXPR T step(T edge, T x) {
	return T(x >= edge);
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> step(vector<T, N> edge, vector<T, N> x) {
	return vector<T, N>(x >= edge);
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> step(T edge, vector<T, N> x) {
	return vector<T, N>(x >= edge);
}

template<class T>
inline BMATH_CONSTEXPR T smoothstep(T edge1, T edge2, T x) {
	T t = clamp((x - edge1) / (edge2 - edge1), T(0), T(1));
	return t * t * (T(3) - T(2) * t);
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> smoothstep(vector<T, N> edge1, vector<T, N> edge2, vector<T, N> x) {
	vector<T, N> t = clamp((x - edge1) / (edge2 - edge1), T(0), T(1));
	return t * t * (T(3) - T(2) * t);
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> smoothstep(T edge1, T edge2, vector<T, N> x) {
	vector<T, N> t = clamp((x - edge1) / (edge2 - edge1), T(0), T(1));
	return t * t * (T(3) - T(2) * t);
}

template<class T>
inline BMATH_CONSTEXPR vector<bool, 2> isnan(vector<T, 2> v) {
	return vector<bool, 2>(
		isnan(v.x),
		isnan(v.y));
}

template<class T>
inline BMATH_CONSTEXPR vector<bool, 3> isnan(vector<T, 3> v) {
	return vector<bool, 3>(
		isnan(v.x),
		isnan(v.y),
		isnan(v.z));
}

template<class T>
inline BMATH_CONSTEXPR vector<bool, 4> isnan(vector<T, 4> v) {
	return vector<bool, 4>(
		isnan(v.x),
		isnan(v.y),
		isnan(v.z),
		isnan(v.w));
}

template<class T>
inline BMATH_CONSTEXPR vector<bool, 2> isinf(vector<T, 2> v) {
	return vector<bool, 2>(
		isinf(v.x),
		isinf(v.y));
}

template<class T>
inline BMATH_CONSTEXPR vector<bool, 3> isinf(vector<T, 3> v) {
	return vector<bool, 3>(
		isinf(v.x),
		isinf(v.y),
		isinf(v.z));
}

template<class T>
inline BMATH_CONSTEXPR vector<bool, 4> isinf(vector<T, 4> v) {
	return vector<bool, 4>(
		isinf(v.x),
		isinf(v.y),
		isinf(v.z),
		isinf(v.w));
}

// Color Space Functions

inline BMATH_CONSTEXPR vec4 unpackRGBA8(uint r8g8b8a8) {
	uint r = (r8g8b8a8 >> 24) & 0xFF;
	uint g = (r8g8b8a8 >> 16) & 0xFF;
	uint b = (r8g8b8a8 >>  8) & 0xFF;
	uint a = (r8g8b8a8 >>  0) & 0xFF;
	return vec4(r, g, b, a) * (1.0f / 255.0f);
}

inline BMATH_CONSTEXPR uint packRGBA8(vec4 rgba) {
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

inline BMATH_CONSTEXPR vec3 HSVtoRGB(vec3 hsv) {
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

inline BMATH_CONSTEXPR vec3 RGBtoHSV(vec3 rgb) {
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

// Geometric Functions

template<class T, int N>
inline BMATH_CONSTEXPR T dot(vector<T, N> left, vector<T, N> right) {
	return compSum(left * right);
}

template<class T>
inline BMATH_CONSTEXPR vector<T, 3> cross(vector<T, 3> left, vector<T, 3> right) {
	return vector<T, 3>(
		left.y * right.z - right.y * left.z,
		left.z * right.x - right.z * left.x,
		left.x * right.y - right.x * left.y);
}

template<class T, int N>
inline BMATH_CONSTEXPR T lengthSq(vector<T, N> v) {
	return dot(v, v);
}

template<class T, int N>
inline BMATH_CONSTEXPR T distanceSq(vector<T, N> p1, vector<T, N> p2) {
	return lengthSq(p1 - p2);
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
inline BMATH_CONSTEXPR vector<T, N> faceforward(vector<T, N> normal, vector<T, N> incidence) {
	return dot(incidence, normal) < T(0) ? normal : -normal;
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> reflect(vector<T, N> incidence, vector<T, N> normal) {
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
inline BMATH_CONSTEXPR T lerp(T from, T to, T amount) {
	return from + (to - from) * amount;
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> lerp(vector<T, N> from, vector<T, N> to, vector<T, N> amount) {
	return from + (to - from) * amount;
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<T, N> lerp(vector<T, N> from, vector<T, N> to, T amount) {
	return from + (to - from) * amount;
}

template<class T>
inline vector<T, 3> slerp(vector<T, 3> from, vector<T, 3> to, T amount) {
	vector<T, 3> z = to;

	T cosTheta = dot(from, to);
	// if cosTheta < 0, the interpolation will take the long way around the sphere.
	if (cosTheta < T(0)) {
		z = -to;
		cosTheta = -cosTheta;
	}

	// sin(angle) -> 0! too close for comfort - do a lerp instead.
	if (cosTheta > T(0.99999))
		return lerp(from, to, amount);

	// Essential Mathematics, page 467.
	T angle = acos(cosTheta);
	return (sin((T(1) - amount) * angle) * from + sin(amount * angle) * z) / sin(angle);
}

// Matrix Functions

template<class T>
inline BMATH_CONSTEXPR matrix<T, 2, 2> outerProduct(vector<T, 2> left, vector<T, 2> right) {
	return matrix<T, 2, 2>(
		left.x * right.x, left.y * right.x,
		left.x * right.y, left.y * right.y);
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 3, 3> outerProduct(vector<T, 3> left, vector<T, 3> right) {
	return matrix<T, 3, 3>(
		left.x * right.x, left.y * right.x, left.z * right.x,
		left.x * right.y, left.y * right.y, left.z * right.y,
		left.x * right.z, left.y * right.z, left.z * right.z);
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 4, 4> outerProduct(vector<T, 4> left, vector<T, 4> right) {
	return matrix<T, 4, 4>(
		left.x * right.x, left.y * right.x, left.z * right.x, left.w * right.x,
		left.x * right.y, left.y * right.y, left.z * right.y, left.w * right.y,
		left.x * right.z, left.y * right.z, left.z * right.z, left.w * right.z,
		left.x * right.w, left.y * right.w, left.z * right.w, left.w * right.w);
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 2, 2> matCompMul(matrix<T, 2, 2> left, matrix<T, 2, 2> right) {
	return matrix<T, 2, 2>(
		left.col[0] * right.col[0],
		left.col[1] * right.col[1]);
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 3, 3> matCompMul(matrix<T, 3, 3> left, matrix<T, 3, 3> right) {
	return matrix<T, 3, 3>(
		left.col[0] * right.col[0],
		left.col[1] * right.col[1],
		left.col[2] * right.col[2]);
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 4, 4> matCompMul(matrix<T, 4, 4> left, matrix<T, 4, 4> right) {
	return matrix<T, 4, 4>(
		left.col[0] * right.col[0],
		left.col[1] * right.col[1],
		left.col[2] * right.col[2],
		left.col[3] * right.col[3]);
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 2, 2> transpose(matrix<T, 2, 2> m) {
	return matrix<T, 2, 2>(
		m.col[0].x, m.col[1].x,
		m.col[0].y, m.col[1].y);
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 3, 3> transpose(matrix<T, 3, 3> m) {
	return matrix<T, 3, 3>(
		m.col[0].x, m.col[1].x, m.col[2].x,
		m.col[0].y, m.col[1].y, m.col[2].y,
		m.col[0].z, m.col[1].z, m.col[2].z);
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 4, 4> transpose(matrix<T, 4, 4> m) {
	return matrix<T, 4, 4>(
		m.col[0].x, m.col[1].x, m.col[2].x, m.col[3].x,
		m.col[0].y, m.col[1].y, m.col[2].y, m.col[3].y,
		m.col[0].z, m.col[1].z, m.col[2].z, m.col[3].z,
		m.col[0].w, m.col[1].w, m.col[2].w, m.col[3].w);
}

template<class T>
inline BMATH_CONSTEXPR T determinant(matrix<T, 2, 2> m) {
	return m.col[0].x * m.col[1].y - m.col[1].x * m.col[0].y;
}

template<class T>
inline BMATH_CONSTEXPR T determinant(matrix<T, 3, 3> m) {
	return
		+ m.col[0].x * (m.col[1].y * m.col[2].z - m.col[2].y * m.col[1].z)
		- m.col[1].x * (m.col[0].y * m.col[2].z - m.col[2].y * m.col[0].z)
		+ m.col[2].x * (m.col[0].y * m.col[1].z - m.col[1].y * m.col[0].z);
}

template<class T>
inline BMATH_CONSTEXPR T determinant(matrix<T, 4, 4> m) {
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
inline BMATH_CONSTEXPR matrix<T, 2, 2> inverse(matrix<T, 2, 2> m) {
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
inline BMATH_CONSTEXPR matrix<T, 3, 3> inverse(matrix<T, 3, 3> m) {
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
inline BMATH_CONSTEXPR matrix<T, 4, 4> inverse(matrix<T, 4, 4> m) {

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
inline BMATH_CONSTEXPR matrix<T, 4, 4> scaleMat(vector<T, 3> xyz) {
	return matrix<T, 4, 4>(vector<T, 4>(xyz, T(1)));
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 4, 4> translationMat(vector<T, 3> xyz) {
	return matrix<T, 4, 4>(
		 T(1),  T(0),  T(0), T(0),
		 T(0),  T(1),  T(0), T(0),
		 T(0),  T(0),  T(1), T(0),
		xyz.x, xyz.y, xyz.z, T(1));
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
inline matrix<T, 4, 4> lookToMat(vector<T, 3> pos, vector<T, 3> dir, vector<T, 3> up) {
	#if defined BMATH_RIGHT_HANDED
	{
		vector<T, 3> f = normalize(dir);
		vector<T, 3> r = normalize(cross(f, up));
		vector<T, 3> u = cross(r, f);

		return matrix<T, 4, 4>(
			r.x, u.x, -f.x, T(0),
			r.y, u.y, -f.y, T(0),
			r.z, u.z, -f.z, T(0),
			-dot(r, pos), -dot(u, pos), +dot(f, pos), T(1));
	}
	#elif defined BMATH_LEFT_HANDED
	{
		vector<T, 3> f = normalize(dir);
		vector<T, 3> r = normalize(cross(up, f));
		vector<T, 3> u = cross(f, r);

		return matrix<T, 4, 4>(
			r.x, u.x, f.x, T(0),
			r.y, u.y, f.y, T(0),
			r.z, u.z, f.z, T(0),
			-dot(r, pos), -dot(u, pos), -dot(f, pos), T(1));
	}
	#endif
}

template<class T>
inline matrix<T, 4, 4> lookAtMat(vector<T, 3> pos, vector<T, 3> target, vector<T, 3> up) {
	return lookToMat(pos, target - pos, up);
}

template<class T>
inline matrix<T, 4, 4> perspectiveMat(T vertFOV, T aspect, T near, T far) {
	T theta = tan(vertFOV / T(2));
	matrix<T, 4, 4> m(T(0));
	m.col[0].x = +T(1) / (aspect * theta);
	m.col[1].y = +T(1) / (theta);
	
	#if defined BMATH_RIGHT_HANDED
	{
		m.col[2].w = -T(1);
		#if defined BMATH_DEPTH_CLIP_ZERO_TO_ONE
		{
			m.col[2].z = +far / (near - far);
			m.col[3].z = -(far * near) / (far - near);
		}
		#elif defined BMATH_DEPTH_CLIP_MINUS_ONE_TO_ONE
		{
			m.col[2].z = -(far + near) / (far - near);
			m.col[3].z = -(T(2) * far * near) / (far - near);
		}
		#endif
	}
	#elif defined BMATH_LEFT_HANDED
	{
		m.col[2].w = +T(1);
		#if defined BMATH_DEPTH_CLIP_ZERO_TO_ONE
		{
			m.col[2].z = +far / (far - near);
			m.col[3].z = -(far * near) / (far - near);
		}
		#elif defined BMATH_DEPTH_CLIP_MINUS_ONE_TO_ONE
		{
			m.col[2].z = +(far + near) / (far - near);
			m.col[3].z = -(T(2) * far * near) / (far - near);
		}
		#endif
	}
	#endif

	return m;
}

template <typename T>
inline matrix<T, 4, 4> orthoMat(T left, T right, T bottom, T top, T near, T far) 
{
	matrix<T, 4, 4> m(1);
	m.col[0].x = T(2) / (right - left);
	m.col[1].y = T(2) / (top - bottom);
	m.col[3].x = -(right + left) / (right - left);
	m.col[3].y = -(top + bottom) / (top - bottom);

	#if defined BMATH_RIGHT_HANDED
	{
		#if defined BMATH_DEPTH_CLIP_ZERO_TO_ONE
		{
			m.col[2].z = -T(1) / (zFar - near);
			m.col[3].z = -near / (zFar - near);
		}
		#elif defined BMATH_DEPTH_CLIP_MINUS_ONE_TO_ONE
		{
			m.col[2].z = -T(2) / (far - near);
			m.col[3].z = -(far + near) / (far - near);
		}
		#endif
	}
	#elif defined BMATH_LEFT_HANDED
	{
		#if defined BMATH_DEPTH_CLIP_ZERO_TO_ONE
		{
			m.col[2].z = T(1) / (far - near);
			m.col[3].z = -near / (far - near);
		}
		#elif defined BMATH_DEPTH_CLIP_MINUS_ONE_TO_ONE
		{
			m.col[2].z = T(2) / (far - near);
			m.col[3].z = -(far + near) / (far - near);
		}
		#endif
	}
	#endif

	return m;
}

// Vector Relational Functions

inline BMATH_CONSTEXPR bool all(bvec2 v) {
	return v.x and v.y;
}

inline BMATH_CONSTEXPR bool all(bvec3 v) {
	return v.x and v.y and v.z;
}

inline BMATH_CONSTEXPR bool all(bvec4 v) {
	return v.x and v.y and v.z and v.w;
}

inline BMATH_CONSTEXPR bool any(bvec2 v) {
	return v.x or v.y;
}

inline BMATH_CONSTEXPR bool any(bvec3 v) {
	return v.x or v.y or v.z;
}

inline BMATH_CONSTEXPR bool any(bvec4 v) {
	return v.x or v.y or v.z or v.w;
}

template<class T>
inline bool epsilonEqual(T left, T right, T epsilon) {
	return abs(left - right) <= epsilon;
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<bool, N> epsilonEqual(vector<T, N> left, vector<T, N> right, T epsilon) {
	return abs(left - right) <= epsilon;
}

template<class T>
inline bool epsilonNotEqual(T left, T right, T epsilon) {
	return abs(left - right) > epsilon;
}

template<class T, int N>
inline BMATH_CONSTEXPR vector<bool, N> epsilonNotEqual(vector<T, N> left, vector<T, N> right, T epsilon) {
	return abs(left - right) > epsilon;
}

// Quaternion Functions

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
inline BMATH_CONSTEXPR quaternion<T> conjugate(quaternion<T> q) {
	return quaternion<T>(-q.x, -q.y, -q.z, q.w);
}

template<class T>
inline BMATH_CONSTEXPR T lengthSq(quaternion<T> q) {
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
inline BMATH_CONSTEXPR quaternion<T> inverse(quaternion<T> q) {
	return conjugate(q) / dot(q.xyzw, q.xyzw);
}

template<class T>
inline quaternion<T> slerp(quaternion<T> from, quaternion<T> to, T amount) {
	quaternion<T> z = to;

	T cosTheta = dot(from.xyzw, to.xyzw);
	// if cosTheta < 0, the interpolation will take the long way around the sphere.
	if (cosTheta < T(0)) {
		z = -to;
		cosTheta = -cosTheta;
	}

	// sin(angle) -> 0! too close for comfort - do a lerp instead.
	if (cosTheta > T(0.99999))
		return lerp(from, to, amount);

	// Essential Mathematics, page 467.
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
		return quat(T(0), T(0), T(0), T(1));

	if (cosTheta < T(-0.99999)) {
		// special case when vectors in opposite directions :
		// there is no "ideal" rotation axis
		// so guess one; any will do as long as it's perpendicular to start
		// this implementation favors a rotation around the Up axis (Y),
		// since it's often what you want to do.
		axis = cross(vector<T, 3>(0, 0, 1), from);
		if (lengthSq(axis) < T(0.00001)) // bad luck, they were parallel, try again!
			axis = cross(vector<T, 3>(1, 0, 0), from);

		axis = normalize(axis);
		return rotationQuat(axis, T(3.141592653589793));
	}

	// from Stan Melax's Game Programming Gems 1 article.
	axis = cross(from, to);

	T s = sqrt((T(1) + cosTheta) * T(2));
	T invs = T(1) / s;

	return quaternion<T>(
		axis.x * invs,
		axis.y * invs,
		axis.z * invs,
		s * T(0.5f));
}

template<class T>
inline BMATH_CONSTEXPR quaternion<T> rotate(quaternion<T> q, quaternion<T> rot) {
	return rot * q * inverse(rot);
}

template<class T>
inline BMATH_CONSTEXPR quaternion<T> rotate(vector<T, 3> v, quaternion<T> rot) {
	return rot * quaternion<T>(v, T(0)) * inverse(rot);
}

template<class T>
inline BMATH_CONSTEXPR quaternion<T> rotate(vector<T, 3> v, vector<T, 3> axis, T angleRad) {
	quaternion<T> rot = rotationQuat(axis, angleRad);
	return rotate(v, rot);
}

template<class T>
inline BMATH_CONSTEXPR matrix<T, 4, 4> quatToMat(quaternion<T> q) {
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

BMATH_END

#undef BMATH_BEGIN
#undef BMATH_END
#undef BMATH_HAS_CONSTEXPR
#undef BMATH_HAS_EXP2_LOG2
#undef BMATH_HAS_DEFAULT_CONSTRUCTOR
#undef BMATH_CONSTEXPR

#endif // !BMATH_H

/*
  ------------------------------------------------------------------------------
  This software is available under 2 licenses - choose whichever you prefer.
  ------------------------------------------------------------------------------
  ALTERNATIVE A - MIT License
  Copyright (c) 2020 Blat Blatnik
  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the "Software"), to deal in
  the Software without restriction, including without limitation the rights to
  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
  of the Software, and to permit persons to whom the Software is furnished to do
  so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
  ------------------------------------------------------------------------------
  ALTERNATIVE B - Public Domain (www.unlicense.org)
  This is free and unencumbered software released into the public domain.
  Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
  software, either in source code form or as a compiled binary, for any purpose,
  commercial or non-commercial, and by any means.
  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright interest in the software to the public
  domain. We make this dedication for the benefit of the public at large and to
  the detriment of our heirs and successors. We intend this dedication to be an
  overt act of relinquishment in perpetuity of all present and future rights to
  this software under copyright law.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  ------------------------------------------------------------------------------
*/
