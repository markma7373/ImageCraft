#pragma  once

#ifdef UNIX_OS
#include <stdint.h>
#else
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
#endif

#define _SINGLE_SIGN_MASK         0x80000000
#define _SINGLE_EXPONENT_MASK     0x7F800000
#define _SINGLE_SIGINIFICAND_MASK 0x007FFFFF
#define _SINGLE_EXPONENT_BIAS     127

#define _HALF_SIGN_MASK           0x8000
#define _HALF_EXPONENT_MASK       0x7C00
#define _HALF_SIGINIFICAND_MASK   0x03FF
#define _HALF_EXPONENT_BIAS       15


#define _FLOAT12_SIGN_MASK           0x0800
#define _FLOAT12_EXPONENT_MASK       0x0780
#define _FLOAT12_SIGINIFICAND_MASK   0x007F
#define _FLOAT12_EXPONENT_BIAS       7

class half
{
    //////////////////////////////////////////////////////////////////////////////
    // Follow IEEE 754-1985 standard
    //
    // half structure:
    // [15 - 15] (1 bit)     sign bits
    // [14 - 10] (5 bits)    exponent     < bias = 15 >
    // [ 9 -  0] (10 bits)   significand

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    //
    //  Notice!! We have some assumption for the converted float type as follows 
    //   - follow IEEE 754-1985 standard, 
    //   - adopt "LITTLE-ENDIAN" memory design
    //

public:
    half()
    {
        m_value = 0; 
    }

    half(const float float_value)
    {
        m_value = float_to_half(float_value);
    }

    operator float() const
    {
        return half_to_float(m_value);
    }

    // for load/save the value to the file
    void set_value(uint16_t byte_value)
    {
        m_value = byte_value;
    }
    uint16_t get_value() const
    {
        return m_value;
    }

private:

    inline uint16_t float_to_half(float float_value) const
    {
        union
        {
            float f;
            uint32_t ui32; 
        } compact_value;

        compact_value.f = float_value;

        int32_t sign         = (compact_value.ui32 & _SINGLE_SIGN_MASK)     >> 16;
        int32_t exponent     = (compact_value.ui32 & _SINGLE_EXPONENT_MASK) >> 23;
        int32_t siginificand = compact_value.ui32 & _SINGLE_SIGINIFICAND_MASK;
        
        if (exponent != 255) 
        {
            //  normal case (exponent < 255)

            exponent = exponent - _SINGLE_EXPONENT_BIAS + _HALF_EXPONENT_BIAS;

            // normalized representation only can handle 0 < exponent < 30
            // do corner case handling here
            if (exponent < -10)
            {
                // too small, no way to represent, return 0.0f/-0.0f
                exponent = siginificand = 0;
            }
            else if (exponent <= 0)
            {
                // underflow, use de-normalized representation
                siginificand = ((siginificand | 0x00800000) >> (1 - exponent)) >> 13;
                exponent = 0;
            }
            else if (exponent >= 31)
            {
                // overflow, set the value as INFINITE
                exponent = 31;
                siginificand = 0;
            }
            else
            {
                exponent = exponent;
                siginificand = siginificand >> 13;
            }
        }
        else
        {
            // handle INIFINITE and NaN (exponent = 255)

            if (siginificand == 0)
            {
                // INFINITE
                exponent = 31;
                siginificand = 0;
            }
            else
            {
                // NaN (Not a Number)
                exponent = 31;
                siginificand = (siginificand >> 13)| 0x00000001; // make sure siginificand is not zero
            }
        }

        return (uint16_t)( sign | (exponent << 10) | siginificand );
    }

    inline float half_to_float(uint16_t half_value) const
    {
        int32_t sign         = (half_value & _HALF_SIGN_MASK)     << 16;
        int32_t exponent     = (half_value & _HALF_EXPONENT_MASK) >> 10;
        int32_t siginificand = (half_value & _HALF_SIGINIFICAND_MASK);

        if (exponent != 31)
        {
            // normal case (exponent != 31)

            if (exponent == 0)
            {
                // de-normalized representation

                if (siginificand == 0)
                {
                    // 0.0f / -0.0f
                    exponent = siginificand = 0;
                }
                else
                {
                    exponent += 1;
                    exponent = exponent - _HALF_EXPONENT_BIAS + _SINGLE_EXPONENT_BIAS;

                    while (!(siginificand & 0x00000400))
                    {
                        siginificand = siginificand << 1;
                        exponent -= 1;
                    }

                    siginificand = (siginificand & 0x000003FF) << 13;
                }
            }
            else
            {
                // normalized representation

                exponent = exponent - _HALF_EXPONENT_BIAS + _SINGLE_EXPONENT_BIAS;
                siginificand = siginificand << 13;
            }
            
        }
        else
        {
            // handle INIFINITE and NaN (exponent = 31)

            exponent = 255;
            siginificand = siginificand << 13;
        }

        union
        {
            float f;
            uint32_t ui32; 
        } compact_value;

        compact_value.ui32 = ( sign | (exponent << 23) | siginificand );

        return compact_value.f;
    }

    uint16_t m_value;
};

class float12
{

public:
    float12()
    {
        m_value = 0;
    }

    float12(const float float_value)
    {
        m_value = float_to_float12(float_value);
    }

    operator float() const
    {
        return float12_to_float(m_value);
    }

    // for load/save the value to the file
    void set_value(uint16_t byte_value)
    {
        m_value = byte_value;
    }
    uint16_t get_value() const
    {
        return m_value;
    }

private:

    

    __forceinline uint16_t float_to_float12(float float_value) const
    {
        union
        {
            float f;
            uint32_t ui32; 
        } compact_value;

        compact_value.f = float_value;

        int32_t sign         = (compact_value.ui32 & _SINGLE_SIGN_MASK)     >> 20;
        int32_t exponent     = (compact_value.ui32 & _SINGLE_EXPONENT_MASK) >> 23;
        int32_t siginificand = compact_value.ui32 & _SINGLE_SIGINIFICAND_MASK;

        if (exponent != 255) 
        {

            exponent = exponent - _SINGLE_EXPONENT_BIAS + _FLOAT12_EXPONENT_BIAS;
            if (exponent < -10)
            {
                exponent = siginificand = 0;
            }
            else if (exponent <= 0)
            {
                siginificand = ((siginificand | 0x00800000) >> (1 - exponent)) >> 16;
                exponent = 0;
            }
            else if (exponent >= 15)
            {
                exponent = 15;
                siginificand = 0;
            }
            else
            {
                exponent = exponent;
                siginificand = siginificand >> 16;
            }
        }
        else
        {
            if (siginificand == 0)
            {
                exponent = 15;
                siginificand = 0;
            }
            else
            {
                exponent = 15;
                siginificand = (siginificand >> 16)| 0x00000001;
            }
        }

        return (uint16_t)( sign | (exponent << 7) | siginificand );
    }

    __forceinline float float12_to_float(uint16_t half_value) const
    {
        int32_t sign         = (half_value & _FLOAT12_SIGN_MASK)     << 20;
        int32_t exponent     = (half_value & _FLOAT12_EXPONENT_MASK) >> 7;
        int32_t siginificand = (half_value & _FLOAT12_SIGINIFICAND_MASK);

        if (exponent != 15)
        {
            if (exponent == 0)
            {
                if (siginificand == 0)
                {
                    exponent = siginificand = 0;
                }
                else
                {
                    exponent += 1;
                    exponent = exponent - _FLOAT12_EXPONENT_BIAS + _SINGLE_EXPONENT_BIAS;

                    while (!(siginificand & 0x00000080))
                    {
                        siginificand = siginificand << 1;
                        exponent -= 1;
                    }

                    siginificand = (siginificand & 0x0000007F) << 16;
                }
            }
            else
            {
                exponent = exponent - _FLOAT12_EXPONENT_BIAS + _SINGLE_EXPONENT_BIAS;
                siginificand = siginificand << 16;
            }

        }
        else
        {
            exponent = 255;
            siginificand = siginificand << 16;
        }

        union
        {
            float f;
            uint32_t ui32; 
        } compact_value;

        compact_value.ui32 = ( sign | (exponent << 23) | siginificand );

        return compact_value.f;
    }

    uint16_t m_value;
};
