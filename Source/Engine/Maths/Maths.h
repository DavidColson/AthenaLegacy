#ifndef MATHS_
#define MATHS_

#include <SDL.h>

#include "ErrorHandling.h"
#include "Reflection.h"

#define ToRadian(x) ((x) * 3.14159f /180.0f)
#define ToDegree(x) ((x) * 180.0f / 3.14159f)

typedef unsigned int uint;
template<uint nrows, uint ncols, typename T>
class mat;

// Maths code taken from an old project, I'd very much like to rewrite it so it's simpler and easier to extend

//////////////////
// Vectors ///////
//////////////////

template <uint dim, typename T>
struct vec 
{
	vec() 
	{
		for (uint i = 0; i < dim; i++)
		{
			m_data[i] = T();
		}
	}

	inline T& operator[](const uint i) { ASSERT(i < dim, "You're accessing an element not in this vector"); return m_data[i]; }
	inline const T& operator[](const uint i) const { ASSERT(i < dim, "You're accessing an element not in this vector"); return m_data[i]; }

private:
	T m_data[dim];
};

template <typename T>
struct vec<2, T> 
{
	vec() : x(T()), y(T()) {}

	vec(T X, T Y) : x(X), y(Y) {}

	template <class U>
	vec<2, T>(const vec<2, U> &v);

	inline T& operator[](const uint i) { ASSERT(i < 2, "You're accessing an element not in this vector"); return i <= 0 ? x : y; }
	inline const T& operator[](const uint i) const { ASSERT(i < 2, "You're accessing an element not in this vector"); return i <= 0 ? x : y; }

	T x, y;
};

template<typename T> 
struct vec<3, T> 
{
	vec() : x(T()), y(T()), z(T()) {}

	vec(T X, T Y, T Z) : x(X), y(Y), z(Z) {}

	template <class U> vec<3, T>(const vec<3, U> &v);

	inline T& operator[](const uint i) { ASSERT(i < 3, "You're accessing an element not in this vector"); return i <= 0 ? x : (1 == i ? y : z); }
	inline const T& operator[](const uint i) const { ASSERT(i < 3, "You're accessing an element not in this vector"); return i <= 0 ? x : (1 == i ? y : z); }

	inline float mag() { return sqrtf(x*x + y*y + z*z); }
	inline vec<3, T>& normalise() { *this = (*this)*(1 / mag()); return *this; }

	T x, y, z;
};

template<typename T>
struct vec<4, T>
{
	vec() : x(T()), y(T()), z(T()), w(T()) {}

	vec(T X, T Y, T Z, T W) : x(X), y(Y), z(Z), w(W) {}

	inline T& operator[](const uint i) { ASSERT(i < 4, "You're accessing an element not in this vector"); return i <= 0 ? x : (1 == i ? y : (2 == i ? z : w)); }
	inline const T& operator[](const uint i) const { ASSERT(i < 4, "You're accessing an element not in this vector"); return i <= 0 ? x : (1 == i ? y : (2 == i ? z : w)); }

	inline float mag() { return sqrt(x*x + y*y + z*z + w*w); }
	inline vec<4, T>& normalise() { *this = (*this)*(1 / mag()); return *this; }

	T x, y, z, w;
};

template<uint dim, typename T>
inline T operator*(const vec<dim, T>& lhs, const vec<dim, T>& rhs)
{
	T ret = T();
	for (uint i = dim; i--; ret += lhs[i] * rhs[i]);
	return ret;
}

template<uint dim, typename T>
inline vec<dim, T> operator+(vec<dim, T> lhs, const vec<dim, T>& rhs)
{
	for (uint i = dim; i--; lhs[i] += rhs[i]);
	return lhs;
}

template<uint dim, typename T>
inline vec<dim, T> operator-(vec<dim, T> lhs, const vec<dim, T>& rhs)
{
	for (uint i = dim; i--; lhs[i] -= rhs[i]);
	return lhs;
}

template<uint dim, typename T, typename U>
inline vec<dim, T> operator*(vec<dim, T> lhs, const U& rhs)
{
	for (uint i = dim; i--; lhs[i] *= rhs);
	return lhs;
}

template<uint dim, typename T, typename U>
inline vec<dim, T> operator/(vec<dim, T> lhs, const U& rhs)
{
	for (uint i = dim; i--; lhs[i] /= rhs);
	return lhs;
}

template<uint len, uint dim, typename T>
inline vec<len, T> embed(const vec<dim, T>& v, T fill = 1)
{
	vec<len, T> ret;
	for (uint i = len; i--; ret[i] = (i < dim ? v[i] : fill));
	return ret;
}

template<uint len, uint dim, typename T>
inline vec<len, T> proj(const vec<dim, T>& v)
{
	vec<len, T> ret;
	for (uint i = len; i--; ret[i] =  v[i]);
	return ret;
}

template <typename T>
inline vec<3, T> cross(vec<3, T> v1, vec<3, T> v2)
{
	return vec<3, T>(v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x);
}

//////////////////
// Matrices //////
//////////////////

// This little helper function will recursively calculate the determinant of any square matrix with dimension "dim"
template<uint dim, typename T>
struct dt
{
	inline static T det(const mat<dim, dim, T>& src)
	{
		T ret = 0;
		for (uint i=dim; i--; ret += src[0][i]*src.cofactor(0, i));
		return ret;
	}
};

// This is required to make sure the recursion above stops at some point
template<typename T>
struct dt<1, T> {
	inline static T det(const mat<1, 1, T>& src) {
		return src[0][0];
	}
};

template<uint nrows, uint ncols, typename T>
class mat 
{
	vec<ncols, T> m_rows[nrows];
public:
	mat() {}

	inline vec<ncols, T>& operator[] (const uint id)
	{
		ASSERT(id < nrows, "Invalid row index for this matrix");
		return m_rows[id];
	}

	inline const vec<ncols, T>& operator[] (const uint id) const
	{
		ASSERT(id < nrows, "Invalid row index for this matrix");
		return m_rows[id];
	}

	inline vec<nrows, T> col(const uint id) const
	{
		ASSERT(id < nrows, "Invalid column index for this matrix");
		vec<nrows, T> ret;
		for (uint i = nrows; i--; ret[i] = m_rows[i][id]);
		return ret;
	}

	inline void setcol(const uint id, vec<nrows, T> v)
	{
		ASSERT(id < nrows, "Invalid column index for this matrix");
		for (uint i = nrows; i--; m_rows[i][id] = v[i]);
	}

	inline static mat<nrows, ncols, T> identity()
	{
		mat<nrows, ncols, T> ret;
		for (uint i = nrows; i--;)
			for (uint j = ncols; j--; ret[i][j] = (i == j));
		return ret;
	}

	inline T det() const
	{
		// Determinants only work for square matrices
		// TODO: Memory assignment here is slow, can be optimised
		ASSERT(id < nrows, "Determinants only for square matrices");
		return dt<ncols, T>::det(*this);
	}

	inline mat<nrows - 1, ncols - 1, T> get_minor(uint row, uint col) const
	{
		mat<nrows - 1, ncols - 1, T> ret;
		for (uint i = nrows - 1; i--;)
			for (uint j = ncols - 1; j--; ret[i][j] = m_rows[i < row ? i : i + 1][j < col ? j : j + 1]);
		return ret;
	}

	// Recursive
	inline T cofactor(uint row, uint col) const
	{
		return get_minor(row, col).det()*((row + col) % 2 ? -1 : 1);
	}

	inline mat<nrows, ncols, T> adjugate() const
	{
		mat<nrows, ncols, T> ret;
		for (uint i = nrows; i--;)
			for (uint j = ncols; j--; ret[i][j] = cofactor(i, j));
		return ret;
	}

	inline mat<nrows, ncols, T> invert_transpose()
	{
		mat<nrows, ncols, T> ret = adjugate();
		T temp = ret[0] * m_rows[0];
		return ret / temp;
	}

	inline mat<nrows, ncols, T> invert()
	{
		return invert_transpose().transpose();
	}

	inline mat<ncols, nrows, T> transpose()
	{
		mat<ncols, nrows, T> ret;
		for (uint i = ncols; i--; ret[i] = this->col(i));
		return ret;
	}
};

template<uint nrows, uint ncols, typename T> 
inline vec<nrows, T> operator*(const mat<nrows, ncols, T>& lhs, const vec<ncols, T>& rhs)
{
	vec<nrows, T> ret;
	// note that lhs[i] is a vector and this is a dot product operation
	for (uint i = nrows; i--; ret[i] = lhs[i] * rhs);
	return ret;
}

template<uint R1, uint C1, uint C2, typename T>
inline mat<R1, C2, T> operator*(const mat<R1, C1, T>& lhs, const mat<C1, C2, T>& rhs)
{
	mat<R1, C2, T> ret;
	for (uint i = R1; i--;)
		for (uint j = C2; j--; ret[i][j] = lhs[i] * rhs.col(j));
	return ret;
}

template<uint nrows, uint ncols, typename T>
inline mat<ncols, nrows, T> operator/(mat<nrows, ncols, T> lhs, const T& rhs)
{
	for (size_t i = nrows; i--; lhs[i] = lhs[i] / rhs);
	return lhs;
}

typedef vec<2, float> vec2;
typedef vec<2, int>   vec2i;
typedef vec<3, float> vec3;
typedef vec<3, int>   vec3i;
typedef vec<4, float> vec4;
typedef mat<4, 4, float> mat4;
typedef mat<3, 3, float> mat3;
typedef vec3 color;

mat4 MakeScale(vec3 Scaling);

mat4 MakeTranslate(vec3 translate);

mat4 MakeRotate(vec3 rotation);

mat4 MakePerspective(float screenWidth, float screenHeight, float Near, float Far, float FOV);

mat4 MakeOrthographic(float left, float right, float bottom, float top, float near, float far);

mat4 MakeViewport(float width, float height);

mat4 MakeLookAt(vec3 Forward, vec3 Up);

void GetAxesFromRotation(vec3 Rotation, vec3 &Forward, vec3 &Right, vec3 & Up);

#endif
