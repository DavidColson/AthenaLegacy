#pragma once

#include "Vec2.h"
#include "Vec3.h"
#include "Maths.h"
#include "Quat.h"

// @Improvement: properly comment/document the rest of this

/**
 * A 4x4 matrix
 */
template<typename T>
struct Matrix
{
	T m[4][4]; // row, column (row major)

	Matrix() 
	{
		m[0][0] = T(1.0); m[0][1] = T(0.0); m[0][2] = T(0.0); m[0][3] = T(0.0);
		m[1][0] = T(0.0); m[1][1] = T(1.0); m[1][2] = T(0.0); m[1][3] = T(0.0);
		m[2][0] = T(0.0); m[2][1] = T(0.0); m[2][2] = T(1.0); m[2][3] = T(0.0);
		m[3][0] = T(0.0); m[3][1] = T(0.0); m[3][2] = T(0.0); m[3][3] = T(1.0);
	}

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
		mat.m[0][0] = T(1.0); mat.m[0][1] = T(0.0); mat.m[0][2] = T(0.0); mat.m[0][3] = T(0.0);
		mat.m[1][0] = T(0.0); mat.m[1][1] = T(1.0); mat.m[1][2] = T(0.0); mat.m[1][3] = T(0.0);
		mat.m[2][0] = T(0.0); mat.m[2][1] = T(0.0); mat.m[2][2] = T(1.0); mat.m[2][3] = T(0.0);
		mat.m[3][0] = T(0.0); mat.m[3][1] = T(0.0); mat.m[3][2] = T(0.0); mat.m[3][3] = T(1.0);
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
		vec.x = m[0][1]; 
		vec.y = m[1][1]; 
		vec.z = m[2][1];
		return vec; 
	}

	inline Vec3<T> GetForwardVector() const
	{
		Vec3<T> vec;
		vec.x = m[0][2]; 
		vec.y = m[1][2]; 
		vec.z = m[2][2];
		return vec;
	}

	inline bool IsRotationOrthonormal() const 
	{
		bool a = Vec3<T>::IsEquivalent( GetRightVector(), 	Vec3<T>::Cross(GetUpVector(), GetForwardVector()) );
		bool b = Vec3<T>::IsEquivalent( GetUpVector(), 		Vec3<T>::Cross(GetForwardVector(), GetRightVector()) );
		bool c = Vec3<T>::IsEquivalent( GetForwardVector(), Vec3<T>::Cross(GetRightVector(), GetUpVector()) );
		return a && b && c;
	}

	inline Vec3<T> ExtractScaling()
	{
		// Completely removes scaling from this matrix and returns the scale vector
		Vec3<T> sca;
		sca.x = sqrt(m[0][0]*m[0][0] + m[0][1]*m[0][1] + m[0][2]*m[0][2]);
        sca.y = sqrt(m[1][0]*m[1][0] + m[1][1]*m[1][1] + m[1][2]*m[1][2]);
        sca.z = sqrt(m[2][0]*m[2][0] + m[2][1]*m[2][1] + m[2][2]*m[2][2]);

		T invScaX = T(1.0) / sca.x;
		m[0][0] *= invScaX;
		m[0][1] *= invScaX;
		m[0][2] *= invScaX;

		T invScaY = T(1.0) / sca.y;
		m[1][0] *= invScaY;
		m[1][1] *= invScaY;
		m[1][2] *= invScaY;

		T invScaZ = T(1.0) / sca.z;
		m[2][0] *= invScaZ;
		m[2][1] *= invScaZ;
		m[2][2] *= invScaZ;

		return sca;
	}

	inline Quat<T> ToQuat() const
    {
        // http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/

        ASSERT(IsRotationOrthonormal(), "Cannot convert a non-orthonormal matrix to a quat");

        Quat<T> q;
        T trace = m[0][0] + m[1][1] + m[2][2];
        if (trace > 0)
        {
            T s = T(0.5) / sqrt(trace + T(1.0)); // 4 * q.w
            q.w = T(0.25) / s;
            q.x = (m[2][1] - m[1][2]) * s;
            q.y = (m[0][2] - m[2][0]) * s;
            q.z = (m[1][0] - m[0][1]) * s;
        }
        else if (m[0][0] > m[1][1] && m[0][0] > m[2][2])
        {
            T s = T(2.0) * sqrt( T(1.0) + m[0][0] - m[1][1] - m[2][2]);
            q.w = (m[2][1] - m[1][2] ) / s;
            q.x = T(0.25) * s;
            q.y = (m[0][1] + m[1][0] ) / s;
            q.z = (m[0][2] + m[2][0] ) / s;
        }
        else if (m[1][1] > m[2][2])
        {
            T s = T(2.0) * sqrt( T(1.0) + m[1][1] - m[0][0] - m[2][2]);
            q.w = (m[0][2] - m[2][0] ) / s;
            q.x = (m[0][1] + m[1][0] ) / s;
            q.y = T(0.25) * s;
            q.z = (m[1][2] + m[2][1] ) / s;
        }
        else
        {
            T s = T(2.0) * sqrt( T(1.0) + m[2][2] - m[0][0] - m[1][1] );
            q.w = (m[1][0] - m[0][1] ) / s;
            q.x = (m[0][2] + m[1][0] ) / s;
            q.y = (m[1][2] + m[2][1] ) / s;
            q.z = T(0.25) * s;
        }
        return q;
    }

	inline eastl::string ToString() const 
	{
		return eastl::string().sprintf("{ %.5f, %.5f, %.5f, %.5f } \n{ %.5f, %.5f, %.5f, %.5f } \n{ %.5f, %.5f, %.5f, %.5f } \n{ %.5f, %.5f, %.5f, %.5f }", 
			m[0][0], m[0][1], m[0][2], m[0][3],
			m[1][0], m[1][1], m[1][2], m[1][3],
			m[2][0], m[2][1], m[2][2], m[2][3],
			m[3][0], m[3][1], m[3][2], m[3][3]);
	}

	inline static Matrix Translate(Vec3<T> translate)
	{
		Matrix mat;
		mat.m[0][0] = T(1.0); mat.m[0][1] = T(0.0); mat.m[0][2] = T(0.0); mat.m[0][3] = translate.x;
		mat.m[1][0] = T(0.0); mat.m[1][1] = T(1.0); mat.m[1][2] = T(0.0); mat.m[1][3] = translate.y;
		mat.m[2][0] = T(0.0); mat.m[2][1] = T(0.0); mat.m[2][2] = T(1.0); mat.m[2][3] = translate.z;
		mat.m[3][0] = T(0.0); mat.m[3][1] = T(0.0); mat.m[3][2] = T(0.0); mat.m[3][3] = T(1.0);
		return mat;
	}

	inline static Matrix Rotate(Vec3<T> rotation)
	{
		// This is a body 3-2-1 (z, then y, then x) rotation
		const T cx = cos(rotation.x);
		const T sx = sin(rotation.x);
		const T cy = cos(rotation.y);
		const T sy = sin(rotation.y);
		const T cz = cos(rotation.z);
		const T sz = sin(rotation.z);

		Matrix res;
		res.m[0][0] = cy*cz;	res.m[0][1] = -cx*sz + sx*sy*cz;	res.m[0][2] =  sx*sz + cx*sy*cz;	res.m[0][3] = T(0.0);
		res.m[1][0] = cy*sz;	res.m[1][1] =  cx*cz + sx*sy*sz;	res.m[1][2] = -sx*cz + cx*sy*sz; 	res.m[1][3] = T(0.0);
		res.m[2][0] = -sy;		res.m[2][1] = sx*cy;				res.m[2][2] = cx*cy; 				res.m[2][3] = T(0.0);
		res.m[3][0] = T(0.0);	res.m[3][1] = T(0.0);				res.m[3][2] = T(0.0); 				res.m[3][3] = T(1.0);

		return res;
	}

	inline static Matrix Scale(Vec3<T> scale)
	{
		Matrix mat;
		mat.m[0][0] = scale.x; 	mat.m[0][1] = T(0.0); 	mat.m[0][2] = T(0.0); 	mat.m[0][3] = T(0.0);
		mat.m[1][0] = T(0.0); 	mat.m[1][1] = scale.y; 	mat.m[1][2] = T(0.0); 	mat.m[1][3] = T(0.0);
		mat.m[2][0] = T(0.0); 	mat.m[2][1] = T(0.0); 	mat.m[2][2] = scale.z; 	mat.m[2][3] = T(0.0);
		mat.m[3][0] = T(0.0); 	mat.m[3][1] = T(0.0); 	mat.m[3][2] = T(0.0); 	mat.m[3][3] = T(1.0);
		return mat;
	}

	inline static Matrix Perspective(T screenWidth, T screenHeight, T nearPlane, T farPlane, T fov)
	{
		// @Improvement: Disallow nearplane 0
		T ar = screenWidth / screenHeight;
		T zRange = nearPlane - farPlane;
		T tanHalfFOV = tan(ToRadian(fov * T(0.5)));

		Matrix mat;
		mat.m[0][0] = T(1.0) / (tanHalfFOV * ar); 	mat.m[0][1] = T(0.0); 				mat.m[0][2] = T(0.0); 								mat.m[0][3] = T(0.0);
		mat.m[1][0] = T(0.0); 						mat.m[1][1] = T(1.0)/tanHalfFOV; 	mat.m[1][2] = T(0.0); 								mat.m[1][3] = T(0.0);
		mat.m[2][0] = T(0.0); 						mat.m[2][1] = T(0.0); 				mat.m[2][2] = (-nearPlane - farPlane) / zRange; 	mat.m[2][3] = (T(2.0) * farPlane * nearPlane)/zRange;
		mat.m[3][0] = T(0.0); 						mat.m[3][1] = T(0.0); 				mat.m[3][2] = T(1.0); 								mat.m[3][3] = T(0.0);
		return mat;
	}

	inline static Matrix Orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane)
	{
		Matrix mat;
		mat.m[0][0] = T(2.0) / (right - left); 		mat.m[0][1] = T(0.0); 					mat.m[0][2] = T(0.0); 								mat.m[0][3] = (-right - left) / (right - left);
		mat.m[1][0] = T(0.0); 						mat.m[1][1] = T(2.0) / (top - bottom); 	mat.m[1][2] = T(0.0); 								mat.m[1][3] = (-top - bottom) / (top - bottom);
		mat.m[2][0] = T(0.0); 						mat.m[2][1] = T(0.0); 					mat.m[2][2] = T(1.0) / (farPlane - nearPlane); 		mat.m[2][3] = (-nearPlane) / (farPlane - nearPlane);
		mat.m[3][0] = T(0.0); 						mat.m[3][1] = T(0.0); 					mat.m[3][2] = T(0.0); 								mat.m[3][3] = T(1.0);
		return mat;
	}

	inline static Matrix MakeLookAt(Vec3<T> Forward, Vec3<T> Up)
    {
        Vec3<T> N = Forward.GetNormalized();
        Vec3<T> U = Up.GetNormalized();
        U = Vec3<T>::Cross(U, N);
        Vec3<T> V = Vec3<T>::Cross(N, U);
    
		Matrix mat;
		mat.m[0][0] = U.x;		mat.m[0][1] = U.y;		mat.m[0][2] = U.z;		mat.m[0][3] = T(0.0);
		mat.m[1][0] = V.x;		mat.m[1][1] = V.y;		mat.m[1][2] = V.z;		mat.m[1][3] = T(0.0);
		mat.m[2][0] = N.x;		mat.m[2][1] = N.y;		mat.m[2][2] = N.z;		mat.m[2][3] = T(0.0);
		mat.m[3][0] = T(0.0);	mat.m[3][1] = T(0.0);	mat.m[3][2] = T(0.0);	mat.m[3][3] = T(1.0);
        return mat;
    }
};

typedef Matrix<float> Matrixf;
typedef Matrix<double> Matrixd;