#pragma once

#include <iostream>
#include <array>

#include <math.h>

namespace eng {

    template<typename T>
    class ComplexNumber {

    public:

        T r, i;

        ComplexNumber(T r, T i) : r(r), i(i) { }

        inline T length() const {
            return sqrt(r * r + i * i);
        }

        inline T angle() const {
            return atan((i / r));
        }

        inline ComplexNumber normalize() const {
            return (*this) * (1 / this->length());
        }

        inline ComplexNumber operator+(const ComplexNumber &b) const {
            return ComplexNumber(r + b.r, i + b.i);
        }

        inline ComplexNumber operator+(T b) const {
            return ComplexNumber(r + b, i);
        }

        inline ComplexNumber operator-(const ComplexNumber &b) const {
            return ComplexNumber(r - b.r, i - b.i);
        }

        inline ComplexNumber operator-(const T &b) const {
            return ComplexNumber(r - b, i);
        }

        inline ComplexNumber operator*(const ComplexNumber &b) const {
            return ComplexNumber(r * b.r - i * b.i, r * b.i + i * b.r);
        }

        inline ComplexNumber operator*(T b) const {
            return ComplexNumber(r * b, i * b);
        }

        inline ComplexNumber operator/(const ComplexNumber &b) const {
            T d = b.r * b.r + b.i * b.i;
            return ComplexNumber(
                (r * b.r + i * b.i) / d,
                (i * b.r - r * b.i) / d
            );
        }

        inline ComplexNumber operator/(T b) const {
            T d = b * b;
            return ComplexNumber((r * b) / d, (i * b) / d);
        }

        friend std::ostream& operator<<(std::ostream &stream, const ComplexNumber &a) {
            stream << a.r << (a.i >= 0 ? " + " : " - ") << abs(a.i) << 'i';
            return stream;
        }

    };



    template<int D, typename T>
    class Vector {
        static_assert(D > 0, "Number of dimensions must be greater than 0");

    public:

        T data[D];

        template <typename... Args>
        Vector(Args... args) : data{ T(args)... } {
            static_assert(sizeof...(Args) == D, "Wrong number of arguments");
        }

        Vector() { }


        int getDimensions() const {
            return D;
        }

        inline T length() const {
            T output = 0;
            for (int i = 0; i < D; i++)
                output += data[i] * data[i];
            return sqrt(output);
        }

        inline Vector normalize() {
            Vector output;
            T d = 1 / this->length();
            for (int i = 0; i < D; i++)
                output[i] = data[i] * d;
            return output;
        }

        inline T operator[](int i) const {
            return data[i];
        }

        inline T & operator[](int i) {
            return data[i];
        }

        inline Vector operator+(const Vector &b) const {
            Vector output = Vector();
            for (int i = 0; i < D; i++)
                output[i] = (*this)[i] + b[i];
            return output;
        }

        inline Vector operator-(const Vector &b) const {
            Vector output = Vector();
            for (int i = 0; i < D; i++)
                output[i] = (*this)[i] - b[i];
            return output;
        }

        inline Vector operator-() const {
            Vector output = Vector();
            for (int i = 0; i < D; i++)
                output[i] = -(*this)[i];
            return output;
        }

        inline Vector operator*(const Vector &b) const {
            Vector output = Vector();
            for (int i = 0; i < D; i++)
                output[i] = (*this)[i] * b[i];
            return output;
        }

        inline Vector operator*(T b) const {
            Vector output = Vector();
            for (int i = 0; i < D; i++)
                output[i] = (*this)[i] * b;
            return output;
        }

        inline bool operator==(const Vector &b) const {
            for (int i = 0; i < D; i++)
                if ((*this)[i] != b[i])
                    return false;
            return true;
        }

        friend std::ostream& operator<<(std::ostream &stream, const Vector &a) {
            stream << "[ " << a[0];
            for (int i = 1; i < D; i++)
                stream << ", "  << a[i];
            stream << " ]";
            return stream;
        }

    };


    template<typename T>
    class Vector3 : public Vector<3, T> {

    public:

        template <typename... Args>
        Vector3(Args... args) : Vector<3, T>(args...) { }

        Vector3() { }

        inline Vector3 cross(const Vector3 &b) const {
            const Vector3 &a = (*this);
            return Vector3(
                a[1] * b[2] - a[2] * b[1],
                a[2] * b[0] - a[0] * b[2],
                a[0] * b[1] - a[1] * b[0]
            );
        }

    };


    template<typename T>
    class QuaternionNumber {

    public:

        T r, i, j, k;

        QuaternionNumber(T r, T i, T j, T k) :
            r(r), i(i), j(j), k(k) { }

        QuaternionNumber(T r, const T *v) :
            r(r), i(v[0]), j(v[1]), k(v[2]) { }
        
        QuaternionNumber() { }

        inline T length() const {
            return sqrt(r * r + i * i + j * j + k * k);
        }

        inline QuaternionNumber normalize() const {
            return (*this) * (1 / this->length());
        }

        inline QuaternionNumber conjugate() const {
            return QuaternionNumber(
                 (*this).r, -(*this).i, -(*this).j, -(*this).k
            );
        }

        inline QuaternionNumber operator+(const QuaternionNumber &b) const {
            return QuaternionNumber(r + b.r, i + b.i, j + b.j, k + b.k);
        }

        inline QuaternionNumber operator-(const QuaternionNumber &b) const {
            return QuaternionNumber(r - b.r, i - b.i, j - b.j, k - b.k);
        }

        inline QuaternionNumber operator*(const QuaternionNumber &b) const {
            return QuaternionNumber(
                r * b.r - i * b.i - j * b.j - k * b.k,
                r * b.i + i * b.r + j * b.k - k * b.j,
                r * b.j - i * b.k + j * b.r + k * b.i,
                r * b.k + i * b.j - j * b.i + k * b.r
            );
        }

        template<typename CT>
        inline QuaternionNumber operator*(const ComplexNumber<CT> &b) const {
            return QuaternionNumber(
                r * b.r - i * b.i,
                r * b.i + i * b.r,
                j * b.r + k * b.i,
                j * b.i + k * b.r
            );
        }

        inline QuaternionNumber operator*(T b) const {
            return QuaternionNumber(
                r * b, i * b, j * b, k * b
            );
        }

        friend std::ostream& operator<<(std::ostream &stream, const QuaternionNumber &a) {
            stream << a.r << (a.i >= 0 ? " + " : " - ") << abs(a.i) << 'i'
                          << (a.j >= 0 ? " + " : " - ") << abs(a.j) << 'j'
                          << (a.k >= 0 ? " + " : " - ") << abs(a.k) << 'k';
            return stream;
        }

    };



    template<int D, typename T>
    class Matrix {
        static_assert(D > 0, "Number of dimensions must be greater than 0");

    public:

        T data[D * D];

        Matrix(bool identity) {
            for (int i = 0; i < D * D; i++)
                data[i] = 0;
            if (identity)
                for (int i = 0; i < D; i++)
                    data[i * D + i] = 1;
        }

        template <typename... Args>
        Matrix(Args... args) : data{ T(args)... } {
            static_assert(sizeof...(Args) == D * D, "Wrong number of arguments");
        }

        Matrix() { }


        inline const T* operator[](int i) const {
            return &(data[i * D]);
        }

        inline T* operator[](int i) {
            return &(data[i * D]);
        }

        inline Matrix operator*(const Matrix &b) const {
            Matrix output = Matrix();
            for (int i = 0; i < D; i++) {
                for (int j = 0; j < D; j++) {
                    T value = 0;
                    for (int k = 0; k < D; k++)
                        value += b[j][k] * (*this)[k][i];
                    output[j][i] = value;
                }
            }
            return output;
        }

        template<typename VT>
        inline Vector<D, T> operator*(const Vector<D, VT> &v) const {
            Vector<D, VT> output;
            for (int i = 0; i < D; i++) {
                T sum = 0;
                for (int j = 0; j < D; j++)
                    sum += (*this)[j][i] * v[j];
                output[i] = sum;
            }
            return output;
        }

        friend std::ostream& operator<<(std::ostream &stream, Matrix &a) {
            for (int j = 0; j < D; j++) {
                stream << "[ " << (a[j][0]);
                for (int i = 1; i < D; i++)
                    stream << ", "  << (a[j][i]);
                stream << " ]\n";
            }
            return stream;
        }

    };


    template<typename T>
    class Matrix4x4 : public Matrix<4, T> {

    public:

        Matrix4x4(bool identity) : Matrix<4, T>(identity) { }

        template<typename... Args>
        Matrix4x4(Args... args) : Matrix<4, T>(args...) { }

        Matrix4x4() { }


        static Matrix4x4 translation(T x, T y, T z) {
            Matrix4x4 output = Matrix4x4(true);
            output[3][0] = x;
            output[3][1] = y;
            output[3][2] = z;
            return output;
        }

        static Matrix4x4 translation(const T *v) {
            Matrix4x4 output = Matrix4x4(true);
            for (int i = 0; i < 3; i++)
                output[3][i] = v[i];
            return output;
        }

        static Matrix4x4 scale(T x, T y, T z) {
            Matrix4x4 output = Matrix4x4(false);
            output[0][0] = x;
            output[1][1] = y;
            output[2][2] = z;
            output[3][3] = 1;
            return output;
        }

        static Matrix4x4 scale(const T *v) {
            Matrix4x4 output = Matrix4x4(false);
            for (int i = 0; i < 3; i++)
                output[i][i] = v[i];
            output[3][3] = 1;
            return output;
        }

        static Matrix4x4 xRotation(T angle) {
            T si = sin(angle);
            T co = cos(angle);
            Matrix4x4 output = Matrix4x4(true);
            output[1][1] = co;
            output[2][1] = -si;
            output[1][2] = si;
            output[2][2] = co;
            return output;
        }

        static Matrix4x4 yRotation(T angle) {
            T si = sin(angle);
            T co = cos(angle);
            Matrix4x4 output = Matrix4x4(true);
            output[0][0] = co;
            output[0][2] = -si;
            output[2][0] = si;
            output[2][2] = co;
            return output;
        }

        static Matrix4x4 zRotation(T angle) {
            T si = sin(angle);
            T co = cos(angle);
            Matrix4x4 output = Matrix4x4(true);
            output[0][0] = co;
            output[1][0] = -si;
            output[0][1] = si;
            output[1][1] = co;
            return output;
        }

        template<typename QT>
        static Matrix4x4 rotation(const QuaternionNumber<QT> &q) {
            T ii = q.i * q.i;
            T ij = q.i * q.j;
            T ik = q.i * q.k;
            T ir = q.i * q.r;
            T jj = q.j * q.j;
            T jk = q.j * q.k;
            T jr = q.j * q.r;
            T kk = q.k * q.k;
            T kr = q.k * q.r;
            return Matrix4x4(
                1 - 2 * (jj + kk), 2 * (ij - kr), 2 * (ik + jr), 0,
                2 * (ij + kr), 1 - 2 * (ii + kk), 2 * (jk - ir), 0,
                2 * (ik - jr), 2 * (jk + ir), 1 - 2 * (ii + jj), 0,
                0, 0, 0, 1
            );
        }

        static Matrix4x4 GL_Projection(
            T fov, T width, T height,
            T nPlane, T fPlane
        ) {
            T aspectRatio = width / height;
            T yScale = (T)((1.0 / tan((fov / 2.0) * M_PI / 180)) * aspectRatio);
            T xScale = yScale / aspectRatio;
            T frustumLength = fPlane - nPlane;
            Matrix4x4 output = Matrix4x4(true);
            output[0][0] = xScale;
            output[1][1] = yScale;
            output[2][2] = -((fPlane + nPlane) / frustumLength);
            output[2][3] = -1;
            output[3][2] = -((2 * nPlane * fPlane) / frustumLength);
            return output;
        }

    };

    typedef ComplexNumber<double> Complexd;
    typedef QuaternionNumber<double> Quaternion;
    typedef ComplexNumber<float> Complexf;
    typedef QuaternionNumber<float> Quaternionf;

    typedef Vector<2, float> Vec2f;
    typedef Vector3<float> Vec3f;
    typedef Vector<4, float> Vec4f;
    typedef Vector<2, double> Vec2;
    typedef Vector3<double> Vec3;
    typedef Vector<4, double> Vec4;

    typedef Matrix<2, float> Mat2f;
    typedef Matrix<3, float> Mat3f;
    typedef Matrix4x4<float> Mat4f;
    typedef Matrix<2, double> Mat2;
    typedef Matrix4x4<double> Mat3;
    typedef Matrix<4, double> Mat4;

}
