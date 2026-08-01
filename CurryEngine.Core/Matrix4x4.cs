
using System;
using System.Runtime.InteropServices;

namespace CurryEngine;

[StructLayout(LayoutKind.Sequential)]
public struct Matrix4x4
{
    public float m00, m01, m02, m03;
    public float m10, m11, m12, m13;
    public float m20, m21, m22, m23;
    public float m30, m31, m32, m33;

    public static readonly Matrix4x4 identity = new()
    {
        m00 = 1, m11 = 1, m22 = 1, m33 = 1
    };

    public static readonly Matrix4x4 zero = new();


    public void SetIdentity()
    {
        m00 = 1; m01 = 0; m02 = 0; m03 = 0;
        m10 = 0; m11 = 1; m12 = 0; m13 = 0;
        m20 = 0; m21 = 0; m22 = 1; m23 = 0;
        m30 = 0; m31 = 0; m32 = 0; m33 = 1;
    }

    public static Matrix4x4 Identity()
    {
        return new Matrix4x4
        {
            m00 = 1, m11 = 1, m22 = 1, m33 = 1
        };
    }

    public static Matrix4x4 Zero()
    {
        return new Matrix4x4();
    }

    /// <summary>
    /// 行列を転置する(行と列を入れ替える)。
    /// </summary>
    /// <param name="matrix">転置する行列</param>
    /// <returns>転置された行列</returns>
    public static Matrix4x4 Transpose(Matrix4x4 matrix)
    {
        return new Matrix4x4
        {
            m00 = matrix.m00,
            m01 = matrix.m10,
            m02 = matrix.m20,
            m03 = matrix.m30,
            m10 = matrix.m01,
            m11 = matrix.m11,
            m12 = matrix.m21,
            m13 = matrix.m31,
            m20 = matrix.m02,
            m21 = matrix.m12,
            m22 = matrix.m22,
            m23 = matrix.m32,
            m30 = matrix.m03,
            m31 = matrix.m13,
            m32 = matrix.m23,
            m33 = matrix.m33
        };
    }

    public static Matrix4x4 operator +(Matrix4x4 a, Matrix4x4 b)
    {
        Matrix4x4 result = new();
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                result[i, j] = a[i, j] + b[i, j];
            }
        }
        return result;
    }

    public static Matrix4x4 operator -(Matrix4x4 a, Matrix4x4 b)
    {
        Matrix4x4 result = new();
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                result[i, j] = a[i, j] - b[i, j];
            }
        }
        return result;
    }

    public static Matrix4x4 operator *(Matrix4x4 a, Matrix4x4 b)
    {
        Matrix4x4 result = new();
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                float sum = 0;
                for (int k = 0; k < 4; k++)
                {
                    sum += a[i, k] * b[k, j];
                }
                result[i, j] = sum;
            }
        }
        return result;
    }

    public float this[int row, int col]
    {
        readonly get
        {
            return row switch
            {
                0 => col switch
                {
                    0 => m00,
                    1 => m01,
                    2 => m02,
                    3 => m03,
                    _ => throw new IndexOutOfRangeException()
                },
                1 => col switch
                {
                    0 => m10,
                    1 => m11,
                    2 => m12,
                    3 => m13,
                    _ => throw new IndexOutOfRangeException()
                },
                2 => col switch
                {
                    0 => m20,
                    1 => m21,
                    2 => m22,
                    3 => m23,
                    _ => throw new IndexOutOfRangeException()
                },
                3 => col switch
                {
                    0 => m30,
                    1 => m31,
                    2 => m32,
                    3 => m33,
                    _ => throw new IndexOutOfRangeException()
                },
                _ => throw new IndexOutOfRangeException()
            };
        }
        set
        {
            switch (row)
            {
                case 0:
                    switch (col)
                    {
                        case 0: m00 = value; break;
                        case 1: m01 = value; break;
                        case 2: m02 = value; break;
                        case 3: m03 = value; break;
                        default: throw new IndexOutOfRangeException();
                    }
                    break;
                case 1:
                    switch (col)
                    {
                        case 0: m10 = value; break;
                        case 1: m11 = value; break;
                        case 2: m12 = value; break;
                        case 3: m13 = value; break;
                        default: throw new IndexOutOfRangeException();
                    }
                    break;
                case 2:
                    switch (col)
                    {
                        case 0: m20 = value; break;
                        case 1: m21 = value; break;
                        case 2: m22 = value; break;
                        case 3: m23 = value; break;
                        default: throw new IndexOutOfRangeException();
                    }
                    break;
                case 3:
                    switch (col)
                    {
                        case 0: m30 = value; break;
                        case 1: m31 = value; break;
                        case 2: m32 = value; break;
                        case 3: m33 = value; break;
                        default: throw new IndexOutOfRangeException();
                    }
                    break;
                default:
                    throw new IndexOutOfRangeException();
            }
        }
    }



    public void SetSRT(Vector3 scale, Quaternion rotation, Vector3 translation)
    {
        // スケーリング
        m00 = scale.x;
        m11 = scale.y;
        m22 = scale.z;
        // 回転
        float x2 = rotation.x + rotation.x;
        float y2 = rotation.y + rotation.y;
        float z2 = rotation.z + rotation.z;
        float wx = rotation.w * x2;
        float wy = rotation.w * y2;
        float wz = rotation.w * z2;
        float xx = rotation.x * x2;
        float xy = rotation.x * y2;
        float xz = rotation.x * z2;
        float yy = rotation.y * y2;
        float yz = rotation.y * z2;
        float zz = rotation.z * z2;
        m00 *= 1.0f - yy - zz;
        m01 = xy - wz;
        m02 = xz + wy;
        m10 = xy + wz;
        m11 *= 1.0f - xx - zz;
        m12 = yz - wx;
        m20 = xz - wy;
        m21 = yz + wx;
        m22 *= 1.0f - xx - yy;
        // 平行移動
        m03 = translation.x;
        m13 = translation.y;
        m23 = translation.z;
        // 最後の行
        m30 = 0; m31 = 0; m32 = 0; m33 = 1;
    }

    public void SetSRT(Vector3 scale, Vector3 eulerRotation, Vector3 translation)
    {
        Quaternion rotation = Quaternion.Euler(eulerRotation);
        SetSRT(scale, rotation, translation);
    }

    public readonly void Decompose(out Vector3 scale, out Quaternion rotation, out Vector3 translation)
    {
        // 平行移動
        translation = new Vector3(m03, m13, m23);
        // スケーリング
        scale = new Vector3(
            (float)MathF.Sqrt(m00 * m00 + m10 * m10 + m20 * m20),
            (float)MathF.Sqrt(m01 * m01 + m11 * m11 + m21 * m21),
            (float)MathF.Sqrt(m02 * m02 + m12 * m12 + m22 * m22)
        );
        // 回転
        float invScaleX = 1.0f / scale.x;
        float invScaleY = 1.0f / scale.y;
        float invScaleZ = 1.0f / scale.z;
        float r00 = m00 * invScaleX;
        float r01 = m01 * invScaleY;
        float r02 = m02 * invScaleZ;
        float r10 = m10 * invScaleX;
        float r11 = m11 * invScaleY;
        float r12 = m12 * invScaleZ;
        float r20 = m20 * invScaleX;
        float r21 = m21 * invScaleY;
        float r22 = m22 * invScaleZ;
        float trace = r00 + r11 + r22;
        if (trace > 0)
        {
            float s = 0.5f / (float)MathF.Sqrt(trace + 1.0f);
            rotation.w = 0.25f / s;
            rotation.x = (r21 - r12) * s;
            rotation.y = (r02 - r20) * s;
            rotation.z = (r10 - r01) * s;
        }
        else if ((r00 > r11) && (r00 > r22))
        {
            float s = 2.0f * (float)MathF.Sqrt(1.0f + r00 - r11 - r22);
            rotation.w = (r21 - r12) / s;
            rotation.x = 0.25f * s;
            rotation.y = (r01 + r10) / s;
            rotation.z = (r02 + r20) / s;
        }
        else if (r11 > r22)
        {             
            float s = 2.0f * (float)MathF.Sqrt(1.0f + r11 - r00 - r22);
            rotation.w = (r02 - r20) / s;
            rotation.x = (r01 + r10) / s;
            rotation.y = 0.25f * s;
            rotation.z = (r12 + r21) / s;
        }
        else
        {
            float s = 2.0f * (float)MathF.Sqrt(1.0f + r22 - r00 - r11);
            rotation.w = (r10 - r01) / s;
            rotation.x = (r02 + r20) / s;
            rotation.y = (r12 + r21) / s;
            rotation.z = 0.25f * s;
        }
    }
}
