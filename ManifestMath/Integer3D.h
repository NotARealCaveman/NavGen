#pragma once
#include "Vector3.h"
#include "Typenames.h"

namespace Manifest_Math
{
    
    struct Integer3D
    {
        MFint32 i;
        MFint32 j;
        MFint32 k;
        MFint32& operator[](const MFu32& index) { return ((&i)[index]); };
        const MFint32& operator[](const MFu32& index)const { return ((&i)[index]); };
        const Integer3D& operator-=(const MFvec3& diff)
        {
            i -= diff.x;
            j -= diff.y;
            k -= diff.z;
            return *this;
        }
        explicit operator MFvec3()
        {
            return { static_cast<MFfloat>(i),static_cast<MFfloat>(j),static_cast<MFfloat>(k) };
        }
        explicit operator MFvec3() const
        {
            return { static_cast<MFfloat>(i),static_cast<MFfloat>(j),static_cast<MFfloat>(k) };
        } 
        friend std::ostream& operator<<(std::ostream& os, const Integer3D& i3D) {
            os << i3D.i << "i " << i3D.j << "j " << i3D.k << "k";

            return os;
        };
    };
    inline Integer3D operator*(const Integer3D& i3D, const MFint32& scale)
    {
        return Integer3D{ i3D.i * scale, i3D.j * scale, i3D.k * scale };
    }
    inline Integer3D operator+(const Integer3D& a, const Integer3D& b)
    {
        return Integer3D{ a.i + b.i,a.j + b.j,a.k + b.k };
    }
    inline MFvec3 operator+(const Integer3D& a, const MFvec3& b)
    {
        return MFvec3{ a.i + b.x,a.j + b.y,a.k + b.z };
    }
    inline Integer3D operator-(const Integer3D& a, const Integer3D& b)
    {
        return Integer3D{ a.i - b.i,a.j - b.j,a.k - b.k };
    }
    inline Integer3D operator<<(const Integer3D& a, const MFu8& shift)
    {
        return Integer3D{ a.i << shift,a.j << shift,a.k << shift };
    }
    inline Integer3D operator>>(const Integer3D& a, const MFu8& shift)
    {
        return Integer3D{ a.i >> shift,a.j >> shift,a.k >> shift };
    }
    inline Integer3D operator<<(const Integer3D& a, const Integer3D& shift)
    {
        return Integer3D{ a.i << shift.i,a.j << shift.j,a.k << shift.k };
    }
    inline Integer3D operator>>(const Integer3D& a, const Integer3D& shift)
    {
        return Integer3D{ a.i >> shift.i,a.j >> shift.j,a.k >> shift.k };
    }
    inline MFbool operator!=(const Integer3D& a, const Integer3D& b)
    {
        return !(a.i == b.i && a.j == b.j && a.k == b.k);
    }
    
    MFullong MagnitudeSquared(const Integer3D& a);
    Integer3D Cross(const Integer3D& a, const Integer3D& b);
}