#pragma once

#include <math.h>

struct HyPoint3D
{
    int x;
    int y;
    int z;

    HyPoint3D()
    {
        x = y = z = 0;
    }
    HyPoint3D(const int _x, const int _y, const int _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }

    bool operator==(const HyPoint3D &point) const
    {
        return (x == point.x) && (y == point.y) && (z == point.z);
    }
    bool operator!=(const HyPoint3D &point) const
    {
        return !(*this == point);
    }

    int norm1() const
    {
        return abs(x) + abs(y) + abs(z);
    }
};

inline HyPoint3D hyPoint3D(const int x, const int y, const int z)
{
    return HyPoint3D(x, y, z);
}

struct HyPoint3D32f
{
    float x;
    float y;
    float z;

    HyPoint3D32f()
    {
        x = y = z = 0;
    }
    HyPoint3D32f(const float _x, const float _y, const float _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }
    HyPoint3D32f(const HyPoint3D &point)
    {
        x = (float)point.x;
        y = (float)point.y;
        z = (float)point.z;
    }

    bool operator==(const HyPoint3D32f &point) const
    {
        return (x == point.x) && (y == point.y) && (z == point.z);
    }
    bool operator!=(const HyPoint3D32f &point) const
    {
        return !(*this == point);
    }
};

inline HyPoint3D32f hyPoint3D32f(const float x, const float y, const float z)
{
    return HyPoint3D32f(x, y, z);
}


inline HyPoint3D hyPoint3D(const HyPoint3D32f& p)
{
    HyPoint3D r;
    r.x = (int) p.x;
    r.y = (int) p.y;
    r.z = (int) p.z;
    return r;
}

inline HyPoint3D32f hyPoint3D32f(const HyPoint3D& p)
{
    HyPoint3D32f r;
    r.x = (float) p.x;
    r.y = (float) p.y;
    r.z = (float) p.z;
    return r;
}

#define _MAKE_HYSQUAREDISTANCE3D(_pt3d)                                                \
    inline float hySquareDistance3D(const _pt3d& p1, const _pt3d& p2)                  \
{                                                                                      \
    return float((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y) + (p1.z - p2.z) * (p1.z - p2.z));     \
}

#define _MAKE_HYROOTDISTANCE3D(_pt3d)                                      \
    inline float hyDistance3D(const _pt3d& p1, const _pt3d& p2)            \
{                                                                          \
    return sqrtf(hySquareDistance3D(p1, p2));                              \
}

_MAKE_HYSQUAREDISTANCE3D(HyPoint3D);
_MAKE_HYSQUAREDISTANCE3D(HyPoint3D32f);
_MAKE_HYROOTDISTANCE3D(HyPoint3D);
_MAKE_HYROOTDISTANCE3D(HyPoint3D32f);

#define _MAKE_HYPOINT3D_MIDPOINT(_pt3d)                                \
    inline _pt3d hyMidPoint3D(const _pt3d& p1, const _pt3d& p2)        \
{                                                                      \
    _pt3d pt;                                                          \
    pt.x = (p1.x + p2.x) / 2;                                          \
    pt.y = (p1.y + p2.y) / 2;                                          \
    pt.z = (p1.z + p2.z) / 2;                                          \
    return pt;                                                         \
}

_MAKE_HYPOINT3D_MIDPOINT(HyPoint3D);
_MAKE_HYPOINT3D_MIDPOINT(HyPoint3D32f);

///////////////////////////////////////////////////

#define _MAKE_HYPOINT3D_OPERATOR(_pt3d, _op)                           \
    inline _pt3d operator _op (const _pt3d& p1, const _pt3d& p2)       \
{                                                                      \
    _pt3d pt;                                                          \
    pt.x = p1.x _op p2.x;                                              \
    pt.y = p1.y _op p2.y;                                              \
    pt.z = p1.z _op p2.z;                                              \
    return pt;                                                         \
}

_MAKE_HYPOINT3D_OPERATOR(HyPoint3D, +);
_MAKE_HYPOINT3D_OPERATOR(HyPoint3D32f, +);

#define _MAKE_HYPOINT3D_OPERATORE(_pt3d, _op)                          \
    inline _pt3d& operator _op (_pt3d& p1, const _pt3d& p2)            \
{                                                                      \
    p1.x _op p2.x;                                                     \
    p1.y _op p2.y;                                                     \
    p1.z _op p2.z;                                                     \
    return p1;                                                         \
}

_MAKE_HYPOINT3D_OPERATORE(HyPoint3D, +=);
_MAKE_HYPOINT3D_OPERATORE(HyPoint3D32f, +=);
_MAKE_HYPOINT3D_OPERATORE(HyPoint3D32f, -=);

#define _MAKE_HYPOINT3D_OPERATOR_V(_pt3d, _op)                         \
    template<class _TYPE>                                              \
    inline _pt3d operator _op (const _pt3d& p, const _TYPE& v)         \
{                                                                      \
    _pt3d pt = p;                                                      \
    pt.x = pt.x _op v;                                                 \
    pt.y = pt.y _op v;                                                 \
    pt.z = pt.z _op v;                                                 \
    return pt;                                                         \
}

_MAKE_HYPOINT3D_OPERATOR_V(HyPoint3D, +);
_MAKE_HYPOINT3D_OPERATOR_V(HyPoint3D, -);
_MAKE_HYPOINT3D_OPERATOR_V(HyPoint3D, *);
_MAKE_HYPOINT3D_OPERATOR_V(HyPoint3D, /);

_MAKE_HYPOINT3D_OPERATOR_V(HyPoint3D32f, +);
_MAKE_HYPOINT3D_OPERATOR_V(HyPoint3D32f, -);
_MAKE_HYPOINT3D_OPERATOR_V(HyPoint3D32f, *);
_MAKE_HYPOINT3D_OPERATOR_V(HyPoint3D32f, /);

///////////////////////////////////////////////////

template<class _TYPE>
inline HyPoint3D& operator += (HyPoint3D& pt, const _TYPE& v)
{
    pt.x = int(pt.x + v);
    pt.y = int(pt.y + v);
    pt.z = int(pt.z + v);
    return pt;
}

template<class _TYPE>
inline HyPoint3D& operator -= (HyPoint3D& pt, const _TYPE& v)
{
    pt.x = int(pt.x - v);
    pt.y = int(pt.y - v);
    pt.z = int(pt.z - v);
    return pt;
}

template<class _TYPE>
inline HyPoint3D& operator *= (HyPoint3D& pt, const _TYPE& v)
{
    pt.x = int(pt.x * v);
    pt.y = int(pt.y * v);
    pt.z = int(pt.z * v);
    return pt;
}

template<class _TYPE>
inline HyPoint3D& operator /= (HyPoint3D& pt, const _TYPE& v)
{
    pt.x = int(pt.x / v);
    pt.y = int(pt.y / v);
    pt.z = int(pt.z / v);
    return pt;
}

template<class _TYPE>
inline HyPoint3D32f& operator += (HyPoint3D32f& pt, const _TYPE& v)
{
    pt.x = float(pt.x + v);
    pt.y = float(pt.y + v);
    pt.z = float(pt.z + v);
    return pt;
}

template<class _TYPE>
inline HyPoint3D32f& operator -= (HyPoint3D32f& pt, const _TYPE& v)
{
    pt.x = float(pt.x - v);
    pt.y = float(pt.y - v);
    pt.z = float(pt.z - v);
    return pt;
}

template<class _TYPE>
inline HyPoint3D32f& operator *= (HyPoint3D32f& pt, const _TYPE& v)
{
    pt.x = float(pt.x * v);
    pt.y = float(pt.y * v);
    pt.z = float(pt.z * v);
    return pt;
}

template<class _TYPE>
inline HyPoint3D32f& operator /= (HyPoint3D32f& pt, const _TYPE& v)
{
    pt.x = float(pt.x / v);
    pt.y = float(pt.y / v);
    pt.z = float(pt.z / v);
    return pt;
}

////////////////////////////////////////
//          HyBoundingBox             //
////////////////////////////////////////

struct HyBoundingBox
{
    int x;
    int y;
    int z;
    int width;
    int height;
    int depth;

    HyBoundingBox()
    {
        x = 0;
        y = 0;
        z = 0;
        width  = 0;
        height = 0;
        depth  = 0;
    }
    HyBoundingBox(const int _x, const int _y, const int _z, const int _width, const int _height, const int _depth)
    {
        x = _x;
        y = _y;
        z = _z;
        width  = _width;
        height = _height;
        depth  = _depth;
    }
    HyBoundingBox(const HyBoundingBox &bbox)
    {
        x = bbox.x;
        y = bbox.y;
        z = bbox.z;
        width  = bbox.width;
        height = bbox.height;
        depth  = bbox.depth;
    }

    int Left() const {return x;}
    int Top()  const {return y;}
    int Near() const {return z;}
    int Right()  const {return x + width;}
    int Bottom() const {return y + height;}
    int Far() const {return z + depth;}

    bool IsPtInBoundingBox(const HyPoint3D &point) const
    {
        return ((point.x >= Left()) && (point.x < Right()) && 
                (point.y >= Top()) && (point.y < Bottom()) &&
                (point.z >= Near()) && (point.z < Far()));
    }

    bool IsBoundingBoxInBoundingBox(const HyBoundingBox &bbox) const
    {
        return ((bbox.Left() >= Left()) && (bbox.Right() <= Right()) && 
                (bbox.Top() >= Top())   && (bbox.Bottom() <= Bottom()) &&
                (bbox.Near() >= Near()) && (bbox.Far() <= Far()));
    }

    bool IsEmpty() const
    {
        return (width == 0) && (height == 0) && (depth == 0);
    }

    void OffsetBoundingBox(const HyPoint3D &point)
    {
        x += point.x;
        y += point.y;
        z += point.z;
    }

    bool operator==(const HyBoundingBox &bbox) const
    {
        return ((x == bbox.x) && (y == bbox.y) && (z == bbox.z) && 
                (width == bbox.width) && (height == bbox.height) && (depth == bbox.depth));
    }

    bool operator!=(const HyBoundingBox &bbox) const
    {
        return !(*this == bbox);
    }

    inline HyBoundingBox operator+ (const HyPoint3D &point) const
    {
        HyBoundingBox bbox;

        bbox.x      = x + point.x;
        bbox.y      = y + point.y;
        bbox.y      = z + point.z;
        bbox.width  = width;
        bbox.height = height;
        bbox.depth  = depth;

        return bbox;
    }

    inline HyBoundingBox operator- (const HyPoint3D &point) const
    {
        HyBoundingBox bbox;

        bbox.x      = x - point.x;
        bbox.y      = y - point.y;
        bbox.y      = z - point.z;
        bbox.width  = width;
        bbox.height = height;
        bbox.depth  = depth;

        return bbox;
    }

    inline HyBoundingBox& operator+= (const HyPoint3D &point)
    {
        x += point.x;
        y += point.y;
        z += point.z;
        return *this;
    }

    inline HyBoundingBox& operator-= (const HyPoint3D &point)
    {
        x -= point.x;
        y -= point.y;
        z -= point.z;
        return *this;
    }
};

inline HyBoundingBox hyBoundingBox(const int x, const int y, const int z, const int width, const int height, const int depth)
{
    return HyBoundingBox(x, y, z, width, height, depth);
}

inline HyBoundingBox hyBoundingBox(const HyBoundingBox &bbox)
{
    return HyBoundingBox(bbox);
}

inline HyPoint3D hyBoundingBoxCenter(const HyBoundingBox& bbox)
{
    HyPoint3D p;
    p.x = bbox.x + bbox.width  / 2;
    p.y = bbox.y + bbox.height / 2;
    p.z = bbox.z + bbox.depth  / 2;
    return p;
}

inline bool hyEmptyBoundingBox(const HyBoundingBox& bbox)
{
    return bbox.IsEmpty();
}

inline int hyVolume(const HyBoundingBox& bbox)
{
    return bbox.width * bbox.height * bbox.depth;
}

inline HyBoundingBox hyIntersectBoundingBox(const HyBoundingBox &bbox1, const HyBoundingBox &bbox2)
{
    int max_left = ch_Max(bbox1.x, bbox2.x);
    int min_right = ch_Min(bbox1.x + bbox1.width, bbox2.x + bbox2.width);
    int max_top = ch_Max(bbox1.y, bbox2.y);
    int min_bottom = ch_Min(bbox1.y + bbox1.height, bbox2.y + bbox2.height);
    int max_near = ch_Max(bbox1.z, bbox2.z);
    int min_far = ch_Min(bbox1.z + bbox1.depth, bbox2.z + bbox2.depth);

    HyBoundingBox new_bbox;
    new_bbox.x = max_left;
    new_bbox.y = max_top;
    new_bbox.z = max_near;
    new_bbox.width = ch_Max(min_right - max_left, 0);    
    new_bbox.height = ch_Max(min_bottom - max_top, 0);
    new_bbox.depth = ch_Max(min_far - max_near, 0);

    return new_bbox;
}

inline bool hyIntersectBoundingBox(HyBoundingBox &result, const HyBoundingBox &bbox1, const HyBoundingBox &bbox2) 
{
    result = hyIntersectBoundingBox(bbox1, bbox2);
    return result.width > 0 && result.height > 0 && result.depth > 0;
}

inline HyBoundingBox hyUnionRect(const HyBoundingBox &bbox1, const HyBoundingBox &bbox2)
{
    int min_left = ch_Min(bbox1.x, bbox2.x);
    int max_right = ch_Max(bbox1.x + bbox1.width, bbox2.x + bbox2.width);
    int min_top = ch_Min(bbox1.y, bbox2.y);
    int max_bottom = ch_Max(bbox1.y + bbox1.height, bbox2.y + bbox2.height);
    int min_near = ch_Min(bbox1.z, bbox2.z);
    int max_far = ch_Max(bbox1.z + bbox1.depth, bbox2.z + bbox2.depth);

    HyBoundingBox new_bbox;
    new_bbox.x = min_left;
    new_bbox.y = min_top;
    new_bbox.z = min_near;
    new_bbox.width = ch_Max(max_right - min_left, 0);    
    new_bbox.height = ch_Max(max_bottom - min_top, 0);
    new_bbox.depth = ch_Max(max_far - min_near, 0);

    return new_bbox;
}

////////////////////////////////////////
//          HyVector3D32f             //
////////////////////////////////////////

struct HyVector3D32f
{
    float x;
    float y;
    float z;

    HyVector3D32f()
    {
        x = y = z = 0;
    }
    HyVector3D32f(const float _x, const float _y, const float _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }

    explicit HyVector3D32f(const HyPoint3D32f &p) : x(p.x), y(p.y), z(p.z) 
    {

    }

    bool operator==(const HyVector3D32f &v) const
    {
        return (x == v.x) && (y == v.y) && (z == v.z);
    }
    bool operator!=(const HyVector3D32f &v) const
    {
        return !(*this == v);
    }

    float SquareLength() const
    {
        return (x * x + y * y + z * z);
    }

    float Length() const
    {
        return sqrtf(SquareLength());
    }
    
    HyVector3D32f Normalize() const
    {
        float length = Length();
        HyVector3D32f normalized_vector(0.0f, 0.0f, 0.0f);
        if (Length() > 0.0000001)
        {
            float inv_length = 1.0f / length;
            normalized_vector = HyVector3D32f(x * inv_length, y * inv_length, z * inv_length);
        }
        return normalized_vector;
    };

    float Dot(const HyVector3D32f &v) const
    {
        return x * v.x + y * v.y + z * v.z;
    }

    HyVector3D32f Cross(const HyVector3D32f &v) const
    {
        return HyVector3D32f(y * v.z - z * v.y,
                             z * v.x - x * v.z,
                             x * v.y - y * v.x);
    }
};

inline HyVector3D32f hyVector3D32f(const float x, const float y, const float z)
{
    return HyVector3D32f(x, y, z);
}

inline HyVector3D32f hyVector3D32f(const HyPoint3D &p)
{
    return HyVector3D32f((float)p.x, (float)p.y, (float)p.z);
}

inline HyVector3D32f hyVector3D32f(const HyPoint3D32f &p)
{
    return HyVector3D32f(p.x, p.y, p.z);
}

inline HyVector3D32f operator - (const HyPoint3D32f& p1, const HyPoint3D32f& p2)
{
    HyVector3D32f pt;                                                          
    pt.x = p1.x - p2.x;                                              
    pt.y = p1.y - p2.y;                                             
    pt.z = p1.z - p2.z;                                           
    return pt; 
}

inline HyPoint3D32f operator + (const HyPoint3D32f& p1, const HyVector3D32f& p2)
{
    HyPoint3D32f pt;                                                          
    pt.x = p1.x + p2.x;                                              
    pt.y = p1.y + p2.y;                                             
    pt.z = p1.z + p2.z;                                           
    return pt; 
}

inline HyPoint3D32f operator - (const HyPoint3D32f& p1, const HyVector3D32f& p2)
{
    HyPoint3D32f pt;                                                          
    pt.x = p1.x - p2.x;                                              
    pt.y = p1.y - p2.y;                                             
    pt.z = p1.z - p2.z;                                           
    return pt; 
}

inline HyPoint3D32f& operator += (HyPoint3D32f& pt, const HyVector3D32f& v)
{
    pt.x = pt.x + v.x;
    pt.y = pt.y + v.y;
    pt.z = pt.z + v.z;
    return pt;
}

inline HyPoint3D32f& operator -= (HyPoint3D32f& pt, const HyVector3D32f& v)
{
    pt.x = pt.x - v.x;
    pt.y = pt.y - v.y;
    pt.z = pt.z - v.z;
    return pt;
}

#define _MAKE_HYVECTOR3D_OPERATOR(_vec3d, _op)                          \
    inline _vec3d operator _op (const _vec3d& v1, const _vec3d& v2)     \
{                                                                       \
    _vec3d vec;                                                         \
    vec.x = v1.x _op v2.x;                                              \
    vec.y = v1.y _op v2.y;                                              \
    vec.z = v1.z _op v2.z;                                              \
    return vec;                                                         \
}

_MAKE_HYVECTOR3D_OPERATOR(HyVector3D32f, +);
_MAKE_HYVECTOR3D_OPERATOR(HyVector3D32f, -);

#define _MAKE_HYVECTOR3D_OPERATORE(_vec3d, _op)                        \
    inline _vec3d& operator _op (_vec3d& v1, const _vec3d& v2)         \
{                                                                      \
    v1.x _op v2.x;                                                     \
    v1.y _op v2.y;                                                     \
    v1.z _op v2.z;                                                     \
    return v1;                                                         \
}

_MAKE_HYVECTOR3D_OPERATORE(HyVector3D32f, +=);
_MAKE_HYVECTOR3D_OPERATORE(HyVector3D32f, -=);

#define _MAKE_HYVECTOR3D_OPERATOR_V(_vec3d, _op)                        \
    template<class _TYPE>                                               \
    inline _vec3d operator _op (const _vec3d& v, const _TYPE& scalar)   \
{                                                                       \
    _vec3d vec = v;                                                     \
    vec.x = vec.x _op scalar;                                           \
    vec.y = vec.y _op scalar;                                           \
    vec.z = vec.z _op scalar;                                           \
    return vec;                                                         \
}

_MAKE_HYVECTOR3D_OPERATOR_V(HyVector3D32f, *);
_MAKE_HYVECTOR3D_OPERATOR_V(HyVector3D32f, /);