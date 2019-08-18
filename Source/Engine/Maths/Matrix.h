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

	inline Matrix GetTranspose() const
	{
		Matrix mat;
		mat.m[0][0] = m[0][0]; mat.m[0][1] = m[1][0]; mat.m[0][2] = m[2][0]; mat.m[0][3] = m[3][0];
		mat.m[1][0] = m[0][1]; mat.m[1][1] = m[1][1]; mat.m[1][2] = m[2][1]; mat.m[1][3] = m[3][1];
		mat.m[2][0] = m[0][2]; mat.m[2][1] = m[1][2]; mat.m[2][2] = m[2][2]; mat.m[2][3] = m[3][2];
		mat.m[3][0] = m[0][3]; mat.m[3][1] = m[1][3]; mat.m[3][2] = m[2][3]; mat.m[3][3] = m[3][3];
		return mat;
	}

	inline T GetDeterminant() const
	{
		return m[0][0] * 
		(m[1][1]*m[2][2]*m[3][3] + m[1][2]*m[2][3]*m[3][1] + m[1][3]*m[2][1]*m[3][2] - m[1][3]*m[2][2]*m[3][1] - m[1][2]*m[2][1]*m[3][3] - m[1][1]*m[2][3]*m[3][2]) 
		 - m[0][1] * 
		(m[1][0]*m[2][2]*m[3][3] + m[1][2]*m[2][3]*m[3][0] + m[1][3]*m[2][0]*m[3][2] - m[1][3]*m[2][2]*m[3][0] - m[1][2]*m[2][0]*m[3][3] - m[1][0]*m[2][3]*m[3][2])
		+ m[0][2] * 
		(m[1][0]*m[2][1]*m[3][3] + m[1][1]*m[2][3]*m[3][0] + m[1][3]*m[2][0]*m[3][1] - m[1][3]*m[2][1]*m[3][0] - m[1][1]*m[2][0]*m[3][3] - m[1][0]*m[2][3]*m[3][1])
		- m[0][3] *
		(m[1][0]*m[2][1]*m[3][2] + m[1][1]*m[2][2]*m[3][0] + m[1][2]*m[2][0]*m[3][1] - m[1][2]*m[2][1]*m[3][0] - m[1][1]*m[2][0]*m[3][2] - m[1][0]*m[2][2]*m[3][1]);
	}

	inline Matrix GetInverse() const
	{
		Matrix res;
		float iDet = 1.0f / GetDeterminant();

		res.m[0][0] = iDet * (	m[1][1]*m[2][2]*m[3][3] + m[1][2]*m[2][3]*m[3][1] + m[1][3]*m[2][1]*m[3][2] - m[1][3]*m[2][2]*m[3][1] - m[1][2]*m[2][1]*m[3][3] - m[1][1]*m[2][3]*m[3][2]);
		res.m[0][1] = iDet * (- m[0][1]*m[2][2]*m[3][3] - m[0][2]*m[2][3]*m[3][1] - m[0][3]*m[2][1]*m[3][2] + m[0][3]*m[2][2]*m[3][1] + m[0][2]*m[2][1]*m[3][3] + m[0][1]*m[2][3]*m[3][2]);
		res.m[0][2] = iDet * (	m[0][1]*m[1][2]*m[3][3] + m[0][2]*m[1][3]*m[3][1] + m[0][3]*m[1][1]*m[3][2] - m[0][3]*m[1][2]*m[3][1] - m[0][2]*m[1][1]*m[3][3] - m[0][1]*m[1][3]*m[3][2]);
		res.m[0][3] = iDet * (- m[0][1]*m[1][2]*m[2][3] - m[0][2]*m[1][3]*m[2][1] - m[0][3]*m[1][1]*m[2][2] + m[0][3]*m[1][2]*m[2][1] + m[0][2]*m[1][1]*m[2][3] + m[0][1]*m[1][3]*m[2][2]);

		res.m[1][0] = iDet * (- m[1][0]*m[2][2]*m[3][3] - m[1][2]*m[2][3]*m[3][0] - m[1][3]*m[2][0]*m[3][2] + m[1][3]*m[2][2]*m[3][0] + m[1][2]*m[2][0]*m[3][3] + m[1][0]*m[2][3]*m[3][2]);
		res.m[1][1] = iDet * (	m[0][0]*m[2][2]*m[3][3] + m[0][2]*m[2][3]*m[3][0] + m[0][3]*m[2][0]*m[3][2] - m[0][3]*m[2][2]*m[3][0] - m[0][2]*m[2][0]*m[3][3] - m[0][0]*m[2][3]*m[3][2]);
		res.m[1][2] = iDet * (- m[0][0]*m[1][2]*m[3][3] - m[0][2]*m[1][3]*m[3][0] - m[0][3]*m[1][0]*m[3][2] + m[0][3]*m[1][2]*m[3][0] + m[0][2]*m[1][0]*m[3][3] + m[0][0]*m[1][3]*m[3][2]);
		res.m[1][3] = iDet * (  m[0][0]*m[1][2]*m[2][3] + m[0][2]*m[1][3]*m[2][0] + m[0][3]*m[1][0]*m[2][2] - m[0][3]*m[1][2]*m[2][0] - m[0][2]*m[1][0]*m[2][3] - m[0][0]*m[1][3]*m[2][2]);

		res.m[2][0] = iDet * (	m[1][0]*m[2][1]*m[3][3] + m[1][1]*m[2][3]*m[3][0] + m[1][3]*m[2][0]*m[3][1] - m[1][3]*m[2][1]*m[3][0] - m[1][1]*m[2][0]*m[3][3] - m[1][0]*m[2][3]*m[3][1]);
		res.m[2][1] = iDet * (- m[0][0]*m[2][1]*m[3][3] - m[0][1]*m[2][3]*m[3][0] - m[0][3]*m[2][0]*m[3][1] + m[0][3]*m[2][1]*m[3][0] + m[0][1]*m[2][0]*m[3][3] + m[0][0]*m[2][3]*m[3][1]);
		res.m[2][2] = iDet * (  m[0][0]*m[1][1]*m[3][3] + m[0][1]*m[1][3]*m[3][0] + m[0][3]*m[1][0]*m[3][1] - m[0][3]*m[1][1]*m[3][0] - m[0][1]*m[1][0]*m[3][3] - m[0][0]*m[1][3]*m[3][1]);
		res.m[2][3] = iDet * (- m[0][0]*m[1][1]*m[2][3] - m[0][1]*m[1][3]*m[2][0] - m[0][3]*m[1][0]*m[2][1] + m[0][3]*m[1][1]*m[2][0] + m[0][1]*m[1][0]*m[2][3] + m[0][0]*m[1][3]*m[2][1]);

		res.m[3][0] = iDet * (- m[1][0]*m[2][1]*m[3][2] - m[1][1]*m[2][2]*m[3][0] - m[1][2]*m[2][0]*m[3][1] + m[1][2]*m[2][1]*m[3][0] + m[1][1]*m[2][0]*m[3][2] + m[1][0]*m[2][2]*m[3][1]);
		res.m[3][1] = iDet * (	m[0][0]*m[2][1]*m[3][2] + m[0][1]*m[2][2]*m[3][0] + m[0][2]*m[2][0]*m[3][1] - m[0][2]*m[2][1]*m[3][0] - m[0][1]*m[2][0]*m[3][2] - m[0][0]*m[2][2]*m[3][1]);
		res.m[3][2] = iDet * (- m[0][0]*m[1][1]*m[3][2] - m[0][1]*m[1][2]*m[3][0] - m[0][2]*m[1][0]*m[3][1] + m[0][2]*m[1][1]*m[3][0] + m[0][1]*m[1][0]*m[3][2] + m[0][0]*m[1][2]*m[3][1]);
		res.m[3][3] = iDet * (	m[0][0]*m[1][1]*m[2][2] + m[0][1]*m[1][2]*m[2][0] + m[0][2]*m[1][0]*m[2][1] - m[0][2]*m[1][1]*m[2][0] - m[0][1]*m[1][0]*m[2][2] - m[0][0]*m[1][2]*m[2][1]);
		return res;
	}

	// Equivalent to Matrix.GetInverse() * Vec, but faster
	inline Vec4<T> InverseTransformVec(const Vec4<T>& rhs) const
	{
		Matrix LU;

		LU.m[0][0] = m[0][0];
		LU.m[1][0] = m[1][0];
		LU.m[2][0] = m[2][0];
		LU.m[3][0] = m[3][0];

		LU.m[0][1] = m[0][1] / LU.m[0][0];
		LU.m[0][2] = m[0][2] / LU.m[0][0];
		LU.m[0][3] = m[0][3] / LU.m[0][0];

		LU.m[1][1] = m[1][1] - LU.m[1][0] * LU.m[0][1];
		LU.m[2][1] = m[2][1] - LU.m[2][0] * LU.m[0][1];
		LU.m[3][1] = m[3][1] - LU.m[3][0] * LU.m[0][1];

		LU.m[1][2] = (m[1][2] - LU.m[1][0] * LU.m[0][2]) / LU.m[1][1]; 
		LU.m[1][3] = (m[1][3] - LU.m[1][0] * LU.m[0][3]) / LU.m[1][1];

		LU.m[2][2] = m[2][2] - (LU.m[2][0] * LU.m[0][2] + LU.m[2][1] * LU.m[1][2]); 
		LU.m[3][2] = m[3][2] - (LU.m[3][0] * LU.m[0][2] + LU.m[3][1] * LU.m[1][2]);

		LU.m[2][3] = (m[2][3] - (LU.m[2][0] * LU.m[0][3] + LU.m[2][1] * LU.m[1][3])) / LU.m[2][2];

		LU.m[3][3] = m[3][3] - (LU.m[3][0] * LU.m[0][3] + LU.m[3][1] * LU.m[1][3] + LU.m[3][2] * LU.m[2][3]); 

		float d_0 = rhs.x / LU.m[0][0];
		float d_1 = (rhs.y - LU.m[1][0] * d_0) / LU.m[1][1];
		float d_2 = (rhs.z - LU.m[2][0] * d_0 - LU.m[2][1] * d_1) / LU.m[2][2];
		float d_3 = (rhs.w - LU.m[3][0] * d_0 - LU.m[3][1] * d_1 - LU.m[3][2] * d_2) / LU.m[3][3];

		Vec4<T> vec;

		vec.w = d_3;
		vec.z = d_2 - (LU.m[2][3] * vec.w);
		vec.y = d_1 - (LU.m[1][2] * vec.z + LU.m[1][3] * vec.w);
		vec.x = d_0 - (LU.m[0][1] * vec.y + LU.m[0][2] * vec.z + LU.m[0][3] * vec.w);

 		return vec;
	}

	inline Vec3<T> GetRightVector() const
	{
		Vec3<T> vec;
		vec.x = m[0][0];
		vec.y = m[1][0];
		vec.z = m[2][0];
		return vec;
	}

	inline Vec3<T> GetUpVector() const
	{
		Vec3<T> vec;
		vec.x = m[0][2]; 
		vec.x = m[1][2]; 
		vec.x = m[2][2];
		return vec; 
	}

	inline Vec3<T> GetForwardVector() const
	{
		Vec3<T> vec;
		vec.x = m[0][1]; 
		vec.x = m[1][1]; 
		vec.x = m[2][1];
		return vec;
	}

	inline std::string ToString() const 
	{
		return StringFormat("{ %.5f, %.5f, %.5f, %.5f } \n { %.5f, %.5f, %.5f, %.5f } \n { %.5f, %.5f, %.5f, %.5f } \n { %.5f, %.5f, %.5f, %.5f }", 
			m[0][0], m[0][1], m[0][2], m[0][3],
			m[1][0], m[1][1], m[1][2], m[1][3],
			m[2][0], m[2][1], m[2][2], m[2][3],
			m[3][0], m[3][1], m[3][2], m[3][3]);
	}

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

	inline static Matrix Perspective(float screenWidth, float screenHeight, float nearPlane, float farPlane, float fov)
	{
		float aspectRatio = screenWidth / screenHeight;
		float viewHeight = 1.0f / tanf(ToRadian(fov/2.0f));
		float viewWidth = viewHeight * aspectRatio;

		Matrix proj;
		proj[0] = vec4(viewWidth,										0.f,										0.f,																						0.f);
		proj[1] = vec4(0.f,													viewHeight,							0.f,																						0.f);
		proj[2] = vec4(0.f,													0.f,										farPlane / (farPlane - nearPlane), 							1.f);
		proj[3] = vec4(0.f,													0.f,										-nearPlane * farPlane / (farPlane - nearPlane),	0.f);

		return proj;
	}

	inline static Matrix Orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane)
	{
		Matrix mat;
		mat.m[0][0] = 2.0f / (right - left);	mat.m[0][1] = 0.0f;										mat.m[0][2] = 0.0f;														mat.m[0][3] = (-right - left) / (right - left);
		mat.m[1][0] = 0.0f;										mat.m[1][1] = 2.0f / (top - bottom);	mat.m[1][2] = 0.0f;														mat.m[1][3] = (-top - bottom) / (top - bottom);
		mat.m[2][0] = 0.0f;										mat.m[2][1] = 0.0f;										mat.m[2][2] = 1.0f / (farPlane - nearPlane);	mat.m[2][3] = (-nearPlane) / (farPlane - nearPlane);
		mat.m[3][0] = 0.0f;										mat.m[3][1] = 0.0f;										mat.m[3][2] = 0.0f;														mat.m[3][3] = 1.0f;
		return mat;
	}
};

typedef Matrix<float> Matrixf;
typedef Matrix<double> Matrixd;