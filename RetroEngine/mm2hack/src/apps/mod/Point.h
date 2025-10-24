//==============================================================================
// 
//  Project: mm2hack
//  Point.h
// 
//  A unified structure for handling coordinate information.
// 
//==============================================================================
#pragma once

namespace mm2hack::apps::mod
{
    // Floating-point coordinate
    class CoordinateF
    {
    public:
        CoordinateF() : value(0) {}
        explicit CoordinateF(double val) : value(val) {}

        // Assignment operator
        CoordinateF& operator=(double v)
        {
            value = v;
            return *this;
        }

        // Addition operator
        CoordinateF operator+(const CoordinateF& other) const
        {
            return CoordinateF(value + other.value);
        }

        // Subtraction operator
        CoordinateF operator-(const CoordinateF& other) const
        {
            return CoordinateF(value - other.value);
        }

        // Compound assignment operators
        CoordinateF& operator+=(const CoordinateF& other)
        {
            value += other.value;
            return *this;
        }

        // Compound assignment operators
        CoordinateF& operator-=(const CoordinateF& other)
        {
            value -= other.value;
            return *this;
        }

        // Compound assignment operators with double
        CoordinateF& operator+=(const double& other)
        {
            value += other;
            return *this;
        }

        // Compound assignment operators with double
        CoordinateF& operator-=(const double& other)
        {
            value -= other;
            return *this;
        }

        operator double() const { return value; }

    private:
        double value;   // Underlying floating-point value
    };

    // 3D Point with floating-point coordinates
    class PointF
    {
    public:
        CoordinateF X;
        CoordinateF Y;
        CoordinateF Z;

        PointF() { X = 0.0; Y = 0.0; Z = 0.0; }
        PointF(double x, double y, double z) : X(x), Y(y), Z(z) {}
        PointF(const PointF& c) : X(c.X), Y(c.Y), Z(c.Z) {}
        ~PointF() {}

        // Assignment operator
        PointF& operator+=(const PointF& other)
        {
            X += other.X;
            Y += other.Y;
            return *this;
        }

        // Assignment operator
        PointF& operator-=(const PointF& other)
        {
            X -= other.X;
            Y -= other.Y;
            return *this;
        }

        // Addition operator
        PointF operator+(const PointF& other) const
        {
            return PointF(X + other.X, Y + other.Y, Z);
        }

        // Subtraction operator
        PointF operator-(const PointF& other) const
        {
            return PointF(X - other.X, Y - other.Y, Z);
        }
    };

    // 3D Point with differential evaluation
    enum class DiffEval
    {
        X, Y, Z
    };

    // Differential PointF
    class DiffPointF : public PointF
    {
    public:
        DiffPointF() {}
        DiffPointF(double x, double y, double z) : PointF(x, y, z) {}
        ~DiffPointF() {}

        // Check if any coordinate differs from zero
        bool IsDiffer() const
        {
            return X != 0.0 || Y != 0.0 || Z != 0.0;
        }

        // Reset all coordinates to zero
        void Reset()
        {
            X = 0.0;
            Y = 0.0;
            Z = 0.0;
        }

        // Set specific coordinate
        void Set(DiffEval d, double value)
        {
            switch (d)
            {
            case DiffEval::X:
                X = value;
                break;
            case DiffEval::Y:
                Y = value;
                break;
            case DiffEval::Z:
                Z = value;
                break;
            }
        }
    };
}