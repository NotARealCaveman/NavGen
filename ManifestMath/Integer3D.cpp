#include "Integer3D.h"

using namespace Manifest_Math;

MFullong Manifest_Math::MagnitudeSquared(const Integer3D& a)
{
    return a.i * a.i + a.j * a.j + a.k * a.k;
}
Integer3D Manifest_Math::Cross(const Integer3D& a, const Integer3D& b)
{
    return { a.j * b.k - a.k * b.j,a.k * b.i - a.i * b.k,a.i * b.j - a.j * b.i };
}