#pragma once

#include "TypeSystem.h"

#include "Vec3.h"
#include "Matrix.h"
#include "Quat.h"

template<typename T>
struct Transform
{
    REFLECT()

    // Translation
    Vec3<T> trans;
    // Rotation
    Quat<T> rot;
    // Scale
    Vec3<T> sca;

    // Constructors from matrix
    Transform() : trans(Vec3<T>()), rot(Quat<T>()), sca(Vec3<T>(1.0)) {}
    Transform(Vec3<T> translation) : trans(translation), rot(Quat<T>()), sca(Vec3<T>(1.0)) {}
    Transform(Vec3<T> translation, Quat<T> rotation) : trans(translation), rot(rotation), sca(Vec3<T>(1.0)) {}
    Transform(Vec3<T> translation, Quat<T> rotation, Vec3<T> scale) : trans(translation), rot(rotation), sca(scale) {}

    Transform(Matrix<T>& affineTransform)
    {
        Matrix<T> m = affineTransform;
        // Extract scaling first from each row
        sca = m.ExtractScaling();

        // Special case for negative scaling
		if (affineTransform.GetDeterminant() < 0.f)
		{
            // choice of axis to flip makes no difference
			sca.x *= -1.f;
            m.m[0][0] = -m.m[0][0]; m.m[0][1] = -m.m[0][1]; m.m[0][2] = -m.m[0][2];
		}

        rot = m.ToQuat();
        trans.x = m.m[0][3];
        trans.y = m.m[1][3];
        trans.z = m.m[2][3];
    }

    static Transform Identity() { return Transform(); }

    Matrix<T> ToMatrix() const
    {
        Matrix<T> mat;
		mat.m[0][0] = (1.0f - 2.0f*rot.y*rot.y - 2.0f*rot.z*rot.z) * sca.x;
		mat.m[1][0] = (2.0f*rot.x*rot.y + 2.0f*rot.z*rot.w) * sca.x;
		mat.m[2][0] = (2.0f*rot.x*rot.z - 2.0f*rot.y*rot.w) * sca.x;
        mat.m[3][0] = 0.0f;
		
        mat.m[0][1] = (2.0f*rot.x*rot.y - 2.0f*rot.z*rot.w) * sca.x;
        mat.m[1][1] = (1.0f - 2.0f*rot.x*rot.x - 2.0f*rot.z*rot.z) * sca.x;
        mat.m[2][1] = (2.0f*rot.y*rot.z + 2.0f*rot.x*rot.w) * sca.x;
        mat.m[3][1] = 0.0f;

        mat.m[0][2] = (2.0f*rot.x*rot.z + 2.0f*rot.y*rot.w) * sca.x;
        mat.m[1][2] = (2.0f*rot.y*rot.z - 2.0f*rot.x*rot.w) * sca.x;
        mat.m[2][2] = (1.0f - 2.0f*rot.x*rot.x - 2.0f*rot.y*rot.y) * sca.x;
        mat.m[3][2] = 0.0f;

        mat.m[0][3] = trans.x;
        mat.m[1][3] = trans.y;
        mat.m[2][3] = trans.z;
        mat.m[3][3] = 1.0f;		                           			                           
        return mat;
    }

    inline eastl::string ToString() const 
	{
		return eastl::string().sprintf("T { %.5f, %.5f, %.5f } \nR { %.5f, %.5f, %.5f, %.5f } \nS { %.5f, %.5f, %.5f }", 
			trans.x, trans.y, trans.z,
			rot.x, rot.y, rot.z, rot.w,
			sca.x, sca.y, sca.z);
	}

    // Transform * Transform
    inline Transform operator*(Transform& rhs) const
    {
        // Rotation is easy
        Transform res;
        res.rot = rot * rhs.rot;

        // scale is easy
        res.sca = Vec3<T>::CompMul(sca, rhs.sca);

        // translation is tricker
        res.trans = rot * Vec3<T>::CompMul(sca, rhs.trans) + trans;

        return res;
    }

    // Transform * vector

    // Transform inverse

    // Get relative transform
};

typedef Transform<float> Transformf;
typedef Transform<double> Transformd;