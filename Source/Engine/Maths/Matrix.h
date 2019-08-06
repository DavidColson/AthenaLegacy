#pragma once

#include "Vec2.h"
#include "Vec3.h"

/**
 * A 4x4 matrix
 */
template<typename T>
struct Matrix
{
	// Wishlist
	T m[4][4]; // row, column

	Matrix() {}

	Matrix(const Matrix& copy) 
	{
		m[0][0] = copy.m[0][0]; m[0][1] = copy.m[0][1]; m[0][2] = copy.m[0][2]; m[0][3] = copy.m[0][3];
		m[1][0] = copy.m[1][0]; m[1][1] = copy.m[1][1]; m[1][2] = copy.m[1][2]; m[1][3] = copy.m[1][3];
		m[2][0] = copy.m[2][0]; m[2][1] = copy.m[2][1]; m[2][2] = copy.m[2][2]; m[2][3] = copy.m[2][3];
		m[3][0] = copy.m[3][0]; m[3][1] = copy.m[3][1]; m[3][2] = copy.m[3][2]; m[3][3] = copy.m[3][3];
	}

	inline static Matrix Identity()
	{
		Matrix mat;
		mat.m[0][0] = 1.0f; mat.m[0][1] = 0.0f; mat.m[0][2] = 0.0f; mat.m[0][3] = 0.0f;
		mat.m[1][0] = 0.0f; mat.m[1][1] = 1.0f; mat.m[1][2] = 0.0f; mat.m[1][3] = 0.0f;
		mat.m[2][0] = 0.0f; mat.m[2][1] = 0.0f; mat.m[2][2] = 1.0f; mat.m[2][3] = 0.0f;
		mat.m[3][0] = 0.0f; mat.m[3][1] = 0.0f; mat.m[3][2] = 0.0f; mat.m[3][3] = 1.0f;
		return mat;
	}

	/**
	* Matrix vs Matrix multiplication
	*
	* @param  rhs The matrix to multiple on the right of this
	* @return The result of the multiplication
	**/
	inline Matrix operator*(const Matrix& rhs) const
	{
		Matrix mat;

		// Row 1
		mat.m[0][0] = m[0][0] * rhs.m[0][0] + m[0][1] * rhs.m[1][0] + m[0][2] * rhs.m[2][0] + m[0][3] * rhs.m[3][0];
		mat.m[0][1] = m[0][0] * rhs.m[0][1] + m[0][1] * rhs.m[1][1] + m[0][2] * rhs.m[2][1] + m[0][3] * rhs.m[3][1];
		mat.m[0][2] = m[0][0] * rhs.m[0][2] + m[0][1] * rhs.m[1][2] + m[0][2] * rhs.m[2][2] + m[0][3] * rhs.m[3][2];
		mat.m[0][3] = m[0][0] * rhs.m[0][3] + m[0][1] * rhs.m[1][3] + m[0][2] * rhs.m[2][3] + m[0][3] * rhs.m[3][3];

		// Row 2
		mat.m[1][0] = m[1][0] * rhs.m[0][0] + m[1][1] * rhs.m[1][0] + m[1][2] * rhs.m[2][0] + m[1][3] * rhs.m[3][0];
		mat.m[1][1] = m[1][0] * rhs.m[0][1] + m[1][1] * rhs.m[1][1] + m[1][2] * rhs.m[2][1] + m[1][3] * rhs.m[3][1];
		mat.m[1][2] = m[1][0] * rhs.m[0][2] + m[1][1] * rhs.m[1][2] + m[1][2] * rhs.m[2][2] + m[1][3] * rhs.m[3][2];
		mat.m[1][3] = m[1][0] * rhs.m[0][3] + m[1][1] * rhs.m[1][3] + m[1][2] * rhs.m[2][3] + m[1][3] * rhs.m[3][3];

		// Row 3
		mat.m[2][0] = m[2][0] * rhs.m[0][0] + m[2][1] * rhs.m[1][0] + m[2][2] * rhs.m[2][0] + m[2][3] * rhs.m[3][0];
		mat.m[2][1] = m[2][0] * rhs.m[0][1] + m[2][1] * rhs.m[1][1] + m[2][2] * rhs.m[2][1] + m[2][3] * rhs.m[3][1];
		mat.m[2][2] = m[2][0] * rhs.m[0][2] + m[2][1] * rhs.m[1][2] + m[2][2] * rhs.m[2][2] + m[2][3] * rhs.m[3][2];
		mat.m[2][3] = m[2][0] * rhs.m[0][3] + m[2][1] * rhs.m[1][3] + m[2][2] * rhs.m[2][3] + m[2][3] * rhs.m[3][3];

		// Row 4
		mat.m[3][0] = m[3][0] * rhs.m[0][0] + m[3][1] * rhs.m[1][0] + m[3][2] * rhs.m[2][0] + m[3][3] * rhs.m[3][0];
		mat.m[3][1] = m[3][0] * rhs.m[0][1] + m[3][1] * rhs.m[1][1] + m[3][2] * rhs.m[2][1] + m[3][3] * rhs.m[3][1];
		mat.m[3][2] = m[3][0] * rhs.m[0][2] + m[3][1] * rhs.m[1][2] + m[3][2] * rhs.m[2][2] + m[3][3] * rhs.m[3][2];
		mat.m[3][3] = m[3][0] * rhs.m[0][3] + m[3][1] * rhs.m[1][3] + m[3][2] * rhs.m[2][3] + m[3][3] * rhs.m[3][3];
		return mat;
	}

	/**
	* Matrix vs Vector multiplication
	*
	* @param  rhs The matrix to multiple on the right of this
	* @return The result of the multiplication
	**/
	inline Vec4<T> operator*(const Vec4<T>& rhs) const
	{
		Vec4<T> vec;

		vec.x = m[0][0] * rhs.x + m[0][1] * rhs.y + m[0][2] * rhs.z + m[0][3] * rhs.w;
		vec.y = m[1][0] * rhs.x + m[1][1] * rhs.y + m[1][2] * rhs.z + m[1][3] * rhs.w;
		vec.z = m[2][0] * rhs.x + m[2][1] * rhs.y + m[2][2] * rhs.z + m[2][3] * rhs.w;
		vec.w = m[3][0] * rhs.x + m[3][1] * rhs.y + m[3][2] * rhs.z + m[3][3] * rhs.w;

		return vec;
	}

	/**
	 * GetColumnXX(Maybe written as GetUpVector etc)
	 * Determinant
	 * GetInverse
	 * transpose
	 */

	inline static Matrix Translate(Vec3<T> translate)
	{
		Matrix mat;
		mat.m[0][0] = 1.0f; mat.m[0][1] = 0.0f; mat.m[0][2] = 0.0f; mat.m[0][3] = translate.x;
		mat.m[1][0] = 0.0f; mat.m[1][1] = 1.0f; mat.m[1][2] = 0.0f; mat.m[1][3] = translate.y;
		mat.m[2][0] = 0.0f; mat.m[2][1] = 0.0f; mat.m[2][2] = 1.0f; mat.m[2][3] = translate.z;
		mat.m[3][0] = 0.0f; mat.m[3][1] = 0.0f; mat.m[3][2] = 0.0f; mat.m[3][3] = 1.0f;
		return mat;
	}

	inline static Matrix Rotate(Vec3<T> rotation)
	{
		float x = rotation.x;
		float y = rotation.y;
		float z = rotation.z;

		Matrix rx;
		rx.m[0][0] = 1.0f; rx.m[0][1] = 0.0f;	rx.m[0][2] = 0.0f;		rx.m[0][3] = 0.0f;
		rx.m[1][0] = 0.0f; rx.m[1][1] = cos(x); rx.m[1][2] = -sin(x);	rx.m[1][3] = 0.0f;
		rx.m[2][0] = 0.0f; rx.m[2][1] = sin(x); rx.m[2][2] = cos(x);	rx.m[2][3] = 0.0f;
		rx.m[3][0] = 0.0f; rx.m[3][1] = 0.0f;	rx.m[3][2] = 0.0f;		rx.m[3][3] = 1.0f;

		Matrix ry;
		ry.m[0][0] = cos(y);	ry.m[0][1] = 0.0f; ry.m[0][2] = -sin(y);	ry.m[0][3] = 0.0f;
		ry.m[1][0] = 0.0f;		ry.m[1][1] = 1.0f; ry.m[1][2] = 0.0f;		ry.m[1][3] = 0.0f;
		ry.m[2][0] = sin(y);	ry.m[2][1] = 0.0f; ry.m[2][2] = cos(y);		ry.m[2][3] = 0.0f;
		ry.m[3][0] = 0.0f;		ry.m[3][1] = 0.0f; ry.m[3][2] = 0.0f;		ry.m[3][3] = 1.0f;

		Matrix rz;
		rz.m[0][0] = cos(z);	rz.m[0][1] = -sin(z);	rz.m[0][2] = 0.0f; rz.m[0][3] = 0.0f;
		rz.m[1][0] = sin(z);	rz.m[1][1] = cos(z);	rz.m[1][2] = 0.0f; rz.m[1][3] = 0.0f;
		rz.m[2][0] = 0.0f;		rz.m[2][1] = 0.0f;		rz.m[2][2] = 1.0f; rz.m[2][3] = 0.0f;
		rz.m[3][0] = 0.0f;		rz.m[3][1] = 0.0f;		rz.m[3][2] = 0.0f; rz.m[3][3] = 1.0f;

		return rz * ry * rx;
	}

	inline static Matrix Scale(Vec3<T> scale)
	{
		Matrix mat;
		mat.m[0][0] = scale.x;	mat.m[0][1] = 0.0f;		mat.m[0][2] = 0.0f;		mat.m[0][3] = 0.0f;
		mat.m[1][0] = 0.0f;		mat.m[1][1] = scale.y;	mat.m[1][2] = 0.0f;		mat.m[1][3] = 0.0f;
		mat.m[2][0] = 0.0f;		mat.m[2][1] = 0.0f;		mat.m[2][2] = scale.z;	mat.m[2][3] = 0.0f;
		mat.m[3][0] = 0.0f;		mat.m[3][1] = 0.0f;		mat.m[3][2] = 0.0f;		mat.m[3][3] = 1.0f;
		return mat;
	}

	inline static Matrix Perspective(float screenWidth, float screenHeight, float Near, float Far, float FOV)
	{

	}

	inline static Matrix Orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane)
	{
		Matrix mat;
		mat.m[0][0] = 2.0f / (right - left);	mat.m[0][1] = 0.0f;						mat.m[0][2] = 0.0f;					mat.m[0][3] = (-right - left) / (right - left);
		mat.m[1][0] = 0.0f;						mat.m[1][1] = 2.0f / (top - bottom);	mat.m[1][2] = 0.0f;					mat.m[1][3] = (-top - bottom) / (top - bottom);
		mat.m[2][0] = 0.0f;						mat.m[2][1] = 0.0f;						mat.m[2][2] = 1.0f / (farPlane - nearPlane);	mat.m[2][3] = (-nearPlane) / (farPlane - nearPlane);
		mat.m[3][0] = 0.0f;						mat.m[3][1] = 0.0f;						mat.m[3][2] = 0.0f;					mat.m[3][3] = 1.0f;
		return mat;
	}

	inline static Matrix Viewport(float left, float right, float bottom, float top, float nearPlane, float farPlane)
	{

	}
};

typedef Matrix<float> Matrixf;
typedef Matrix<double> Matrixd;