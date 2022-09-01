
#ifndef TIM_MATH
#define TIM_MATH

#include <math.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 2D HEADER

typedef union
{
    struct
    {
        float x;
        float y;
    };
    float elements[2];
} Vec2;

typedef union 
{
    float m[3][3];
    struct {
        float m00, m01, m02;
        float m10, m11, m12;
        float m20, m21, m22;
    };
} Mat3;

static inline Mat3 M3(
        float m00, float m10, float m20,
        float m01, float m11, float m21,
        float m02, float m12, float m22
        )
{
    return (Mat3) {
        .m[0][0] = m00, .m[1][0] = m10, .m[2][0] = m20,
        .m[0][1] = m01, .m[1][1] = m11, .m[2][1] = m21,
        .m[0][2] = m02, .m[1][2] = m12, .m[2][2] = m22 
    };
}

// 3D HEADER

typedef union 
{ 
    struct
    {
        float x, y, z; 
    };

    struct
    {
        float r, g, b;
    };

    struct
    {
        Vec2 xy;
        float _ignored0;
    };

    float elements[3];

} Vec3;

typedef union 
{ 
    struct
    {
        float x, y, z, w;
    };

    struct
    {
        float r, g, b, a;
    };

    struct
    {
        Vec3 xyz;
        float _ignored0;
    };

    struct
    {
        Vec2 xy;
        Vec2 _ignored1;
    };

    float elements[4];

} Vec4;

typedef union {
	float m[4][4];
	struct {
		float m00, m01, m02, m03;
		float m10, m11, m12, m13;
		float m20, m21, m22, m23;
		float m30, m31, m32, m33;
	};
} Mat4;

static inline 
Mat4 M4(
	float m00, float m10, float m20, float m30,
	float m01, float m11, float m21, float m31,
	float m02, float m12, float m22, float m32,
	float m03, float m13, float m23, float m33) 
{
	return (Mat4)
    {
		.m[0][0] = m00, .m[1][0] = m10, .m[2][0] = m20, .m[3][0] = m30,
		.m[0][1] = m01, .m[1][1] = m11, .m[2][1] = m21, .m[3][1] = m31,
		.m[0][2] = m02, .m[1][2] = m12, .m[2][2] = m22, .m[3][2] = m32,
		.m[0][3] = m03, .m[1][3] = m13, .m[2][3] = m23, .m[3][3] = m33
	};
}

static inline Mat4 M4Identity();
static inline Mat4 M4Translation  (Vec3 offset);
static inline Mat4 M4Scaling      (Vec3 scale);
static inline Mat4 M4RotationX   (float angle_in_rad);
static inline Mat4 M4RotationY   (float angle_in_rad);
static inline Mat4 M4RotationZ   (float angle_in_rad);
              Mat4 M4Rotation     (float angle_in_rad, Vec3 axis);

              Mat4 M4Ortho        (float left, float right, float bottom, float top, float back, float front);
              Mat4 M4Perspective  (float vertical_field_of_view_in_deg, float aspect_ratio, float near_view_distance, float far_view_distance);
              Mat4 M4LookAt      (Vec3 from, Vec3 to, Vec3 up);

static inline Mat4 M4Transpose    (Mat4 matrix);
static inline Mat4 M4Mul          (Mat4 a, Mat4 b);
              Mat4 M4InvertAffine(Mat4 matrix);
              Vec3 M4MulPos      (Mat4 matrix, Vec3 position);
              Vec3 M4MulDir      (Mat4 matrix, Vec3 direction);

              void   M4Print        (Mat4 matrix);
              void   M4PrintP       (Mat4 matrix, int width, int precision);
              void   M4FPrint       (FILE* stream, Mat4 matrix);
              void   M4FPrintP      (FILE* stream, Mat4 matrix, int width, int precision);

// 2D Functions

static inline Vec2
V2(float x, float y)
{
    return (Vec2){{x, y}};
}

static inline Vec2  
V2Add(Vec2 a, Vec2 b)    
{ 
    return (Vec2){{a.x+b.x, a.y+b.y}};
}

static inline Vec2  
V2Sub(Vec2 a, Vec2 b)    
{
    return (Vec2){{a.x-b.x, a.y-b.y}};
}

static inline Vec2  
V2Mul(Vec2 a, Vec2 b)    
{ 
    return (Vec2){{a.x*b.x, a.y*b.y}};
}

static inline Vec2  
V2AddS(Vec2 a, float s)    
{ 
    return (Vec2){{a.x+s, a.y+s}};
}

static inline Vec2  
V2SubS(Vec2 a, float s)    
{ 
    return (Vec2){{a.x-s, a.y-s}};
}

static inline Vec2  
V2MulS(Vec2 a, float s)    
{ 
    return (Vec2){{a.x*s, a.y*s}};
}

static inline float   
V2Dot(Vec2 a, Vec2 b)    
{ 
    return a.x*b.x + a.y*b.y; 
}

static inline float   
V2Len(Vec2 v)           
{ 
    return sqrtf(v.x*v.x + v.y*v.y); 
}

static inline float   
V2Len2(Vec2 v)           
{ 
    return v.x*v.x + v.y*v.y; 
}

static inline float   
V2Dist(Vec2 a, Vec2 b)   
{ 
    return V2Len(V2Sub(b, a)); 
}

static inline Vec2  
V2Norm(Vec2 v)             
{ 
    return V2MulS(v, 1.0/V2Len(v)); 
}

static inline Vec2 
V2Rotate(Vec2 v, float t)   
{ 
    float c = cosf(t);
    float s = sinf(t);
    return V2(c*v.x - s*v.y, s*v.x+c*v.y);
}

static inline Vec2 
V2Lerp(Vec2 from, Vec2 to, float s)
{
    Vec2 diff = V2Sub(to, from);
    return V2Add(from, V2MulS(diff, s));
}

Mat3 
M3Translation(Vec2 translation)
{
    return M3(
            1,0,translation.x,
            0,1,translation.y,
            0,0,1
            );
}

Mat3 M3TranslationAndScale(Vec2 translation, float scaleX, float scaleY)
{
    return M3(
            scaleX, 0,translation.x*scaleX,
            0,scaleY, translation.y*scaleY,
            0,0,1
            );
}

Vec2 M3MulPos(Mat3 matrix, Vec2 position)
{
    Vec2 result = V2(
            matrix.m00*position.x + matrix.m10*position.y + matrix.m20,
            matrix.m01*position.x + matrix.m11*position.y + matrix.m21
            );
    float w = matrix.m02 *position.x + matrix.m12*position.y + matrix.m22;
    if(w!=0 && w!=1)
    {
        return V2(result.x/w, result.y/w);
    }
    else
    {
        return result;
    }
}

void 
M3FPrintP(FILE* stream, Mat3 matrix, int width, int precision) 
{
	Mat3 m = matrix;
	int w = width, p = precision;
	for(int r = 0; r < 3; r++) {
		fprintf(stream, "| %*.*f %*.*f %*.*f |\n",
			w, p, m.m[0][r], w, p, m.m[1][r], w, p, m.m[2][r]
		);
	}
}

void
V2FPrintP(FILE *stream, Vec2 v, int width, int precision)
{
    fprintf(stream, "| %*.*f %*.*f |",
            width, precision, v.x, 
            width, precision, v.y);
}

// 3D 

static inline Vec3 
V3(float x, float y, float z) 
{ 
    return (Vec3){{x, y, z}};
}

static inline Vec4 
V4(float x, float y, float z, float w)
{ 
    return (Vec4){{x, y, z, w}};
}


static inline Vec3 
V3Add(Vec3 a, Vec3 b)          
{ 
    return (Vec3){{ a.x + b.x, a.y + b.y, a.z + b.z }};
}

static inline Vec3 
V3AddS(Vec3 a, float s)
{ 
    return (Vec3){{ a.x + s,   a.y + s,   a.z + s }};
}

static inline Vec3 
V3Sub(Vec3 a, Vec3 b)
{ 
    return (Vec3){{ a.x - b.x, a.y - b.y, a.z - b.z }};
}

static inline Vec3 
V3SubS(Vec3 a, float s)
{ 
    return (Vec3){{ a.x - s, a.y - s, a.z - s}};
}

static inline Vec3 
V3Mul(Vec3 a, Vec3 b)          
{ 
    return (Vec3){{a.x * b.x, a.y * b.y, a.z * b.z}};
}

static inline Vec3 
V3MulS(Vec3 a, float s)           
{ 
    return (Vec3){{ a.x * s,   a.y * s,   a.z * s}};
}

static inline Vec3 
V3Div(Vec3 a, Vec3 b)
{ 
    return (Vec3){{ a.x / b.x, a.y / b.y, a.z / b.z }}; 
}

static inline Vec3 
V3DivS(Vec3 a, float s)           
{ 
    return (Vec3){{ a.x / s,   a.y / s,   a.z / s}};
}

static inline float  
V3Len(Vec3 v)                    
{ 
    return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);          
}

static inline float  
V3Dot(Vec3 a, Vec3 b)          
{ 
    return a.x*b.x + a.y*b.y + a.z*b.z; 
}

static inline Vec3 
V3Norm(Vec3 v) 
{
	float len = V3Len(v);
	if (len > 0)
		return (Vec3){{ v.x / len, v.y / len, v.z / len }};
	else
		return (Vec3){{ 0, 0, 0}};
}

static inline Vec3 
V3Proj(Vec3 v, Vec3 onto) 
{
	return V3MulS(onto, V3Dot(v, onto) / V3Dot(onto, onto));
}

static inline Vec3 
V3Cross(Vec3 a, Vec3 b) 
{
	return (Vec3){{
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	}};
}

static inline float 
V3AngleBetween(Vec3 a, Vec3 b) 
{
	return acosf( V3Dot(a, b) / (V3Len(a) * V3Len(b)) );
}


static inline Mat4 
M4Identity() 
{
	return M4(
		 1,  0,  0,  0,
		 0,  1,  0,  0,
		 0,  0,  1,  0,
		 0,  0,  0,  1
	);
}

static inline Mat4 
M4Translation(Vec3 offset) 
{
	return M4(
		 1,  0,  0,  offset.x,
		 0,  1,  0,  offset.y,
		 0,  0,  1,  offset.z,
		 0,  0,  0,  1
	);
}

static inline Mat4 
M4Scaling(Vec3 scale) 
{
	float x = scale.x, y = scale.y, z = scale.z;
	return M4(
		 x,  0,  0,  0,
		 0,  y,  0,  0,
		 0,  0,  z,  0,
		 0,  0,  0,  1
	);
}

static inline Mat4 
M4RotationX(float angle_in_rad) 
{
	float s = sinf(angle_in_rad), c = cosf(angle_in_rad);
	return M4(
		1,  0,  0,  0,
		0,  c, -s,  0,
		0,  s,  c,  0,
		0,  0,  0,  1
	);
}

static inline Mat4 
M4RotationY (float angle_in_rad) 
{
	float s = sinf(angle_in_rad), c = cosf(angle_in_rad);
	return M4(
		 c,  0,  s,  0,
		 0,  1,  0,  0,
		-s,  0,  c,  0,
		 0,  0,  0,  1
	);
}

static inline Mat4 
M4RotationZ(float angle_in_rad) 
{
	float s = sinf(angle_in_rad), c = cosf(angle_in_rad);
	return M4(
		 c, -s,  0,  0,
		 s,  c,  0,  0,
		 0,  0,  1,  0,
		 0,  0,  0,  1
	);
}

static inline Mat4 
M4Transpose(Mat4 matrix) 
{
	return M4(
		matrix.m00, matrix.m01, matrix.m02, matrix.m03,
		matrix.m10, matrix.m11, matrix.m12, matrix.m13,
		matrix.m20, matrix.m21, matrix.m22, matrix.m23,
		matrix.m30, matrix.m31, matrix.m32, matrix.m33
	);
}

/**
 * Multiplication of two 4x4 matrices.
 * 
 * Implemented by following the row times column rule and illustrating it on a
 * whiteboard with the proper indices in mind.
 * 
 * Further reading: https://en.wikipedia.org/wiki/Matrix_multiplication
 * But note that the article use the first index for rows and the second for
 * columns.
 */
static inline Mat4 
M4Mul(Mat4 a, Mat4 b) 
{
	Mat4 result;
	
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++) {
			float sum = 0;
			for(int k = 0; k < 4; k++) {
				sum += a.m[k][j] * b.m[i][k];
			}
			result.m[i][j] = sum;
		}
	}
	
	return result;
}

#endif // MATH_3D_HEADER

#ifdef TIM_MATH_IMPLEMENTATION

/**
 * Creates a matrix to rotate around an axis by a given angle. The axis doesn't
 * need to be normalized.
 * 
 * Sources:
 * 
 * https://en.wikipedia.org/wiki/Rotation_matrix#Rotation_matrix_from_axis_and_angle
 */
Mat4 
M4Rotation(float angle_in_rad, Vec3 axis) 
{
	Vec3 normalized_axis = V3Norm(axis);
	float x = normalized_axis.x, y = normalized_axis.y, z = normalized_axis.z;
	float c = cosf(angle_in_rad), s = sinf(angle_in_rad);
	
	return M4(
		c + x*x*(1-c),            x*y*(1-c) - z*s,      x*z*(1-c) + y*s,  0,
		    y*x*(1-c) + z*s,  c + y*y*(1-c),            y*z*(1-c) - x*s,  0,
		    z*x*(1-c) - y*s,      z*y*(1-c) + x*s,  c + z*z*(1-c),        0,
		    0,                        0,                    0,            1
	);
}

/**
 * Creates an orthographic projection matrix. It maps the right handed cube
 * defined by left, right, bottom, top, back and front onto the screen and
 * z-buffer. You can think of it as a cube you move through world or camera
 * space and everything inside is visible.
 * 
 * This is slightly different from the traditional glOrtho() and from the linked
 * sources. These functions require the user to negate the last two arguments
 * (creating a left-handed coordinate system). We avoid that here so you can
 * think of this function as moving a right-handed cube through world space.
 * 
 * The arguments are ordered in a way that for each axis you specify the minimum
 * followed by the maximum. Thats why it's bottom to top and back to front.
 * 
 * Implementation details:
 * 
 * To be more exact the right-handed cube is mapped into normalized device
 * coordinates, a left-handed cube where (-1 -1) is the lower left corner,
 * (1, 1) the upper right corner and a z-value of -1 is the nearest point and
 * 1 the furthest point. OpenGL takes it from there and puts it on the screen
 * and into the z-buffer.
 * 
 * Sources:
 * 
 * https://msdn.microsoft.com/en-us/library/windows/desktop/dd373965(v=vs.85).aspx
 * https://unspecified.wordpress.com/2012/06/21/calculating-the-gluperspective-matrix-and-other-opengl-matrix-maths/
 */
Mat4 
M4Ortho(float left, float right, float bottom, float top, float back, float front) 
{
	float l = left, r = right, b = bottom, t = top, n = front, f = back;
	float tx = -(r + l) / (r - l);
	float ty = -(t + b) / (t - b);
	float tz = -(f + n) / (f - n);
	return M4(
		 2 / (r - l),  0,            0,            tx,
		 0,            2 / (t - b),  0,            ty,
		 0,            0,            2 / (f - n),  tz,
		 0,            0,            0,            1
	);
}

/**
 * Creates a perspective projection matrix for a camera.
 * 
 * The camera is at the origin and looks in the direction of the negative Z axis.
 * `near_view_distance` and `far_view_distance` have to be positive and > 0.
 * They are distances from the camera eye, not values on an axis.
 * 
 * `near_view_distance` can be small but not 0. 0 breaks the projection and
 * everything ends up at the max value (far end) of the z-buffer. Making the
 * z-buffer useless.
 * 
 * The matrix is the same as `gluPerspective()` builds. The view distance is
 * mapped to the z-buffer with a reciprocal function (1/x). Therefore the z-buffer
 * resolution for near objects is very good while resolution for far objects is
 * limited.
 * 
 * Sources:
 * 
 * https://unspecified.wordpress.com/2012/06/21/calculating-the-gluperspective-matrix-and-other-opengl-matrix-maths/
 */
Mat4 
M4Perspective(float vertical_field_of_view_in_deg, float aspect_ratio, float near_view_distance, float far_view_distance) 
{
	float fovy_in_rad = vertical_field_of_view_in_deg / 180 * M_PI;
	float f = 1.0f / tanf(fovy_in_rad / 2.0f);
	float ar = aspect_ratio;
	float nd = near_view_distance, fd = far_view_distance;
	
	return M4(
		 f / ar,           0,                0,                0,
		 0,                f,                0,                0,
		 0,                0,               (fd+nd)/(nd-fd),  (2*fd*nd)/(nd-fd),
		 0,                0,               -1,                0
	);
}

/**
 * Builds a transformation matrix for a camera that looks from `from` towards
 * `to`. `up` defines the direction that's upwards for the camera. All three
 * vectors are given in world space and `up` doesn't need to be normalized.
 * 
 * Sources: Derived on whiteboard.
 * 
 * Implementation details:
 * 
 * x, y and z are the right-handed base vectors of the cameras subspace.
 * x has to be normalized because the cross product only produces a normalized
 *   output vector if both input vectors are orthogonal to each other. And up
 *   probably isn't orthogonal to z.
 * 
 * These vectors are then used to build a 3x3 rotation matrix. This matrix
 * rotates a vector by the same amount the camera is rotated. But instead we
 * need to rotate all incoming vertices backwards by that amount. That's what a
 * camera matrix is for: To move the world so that the camera is in the origin.
 * So we take the inverse of that rotation matrix and in case of an rotation
 * matrix this is just the transposed matrix. That's why the 3x3 part of the
 * matrix are the x, y and z vectors but written horizontally instead of
 * vertically.
 * 
 * The translation is derived by creating a translation matrix to move the world
 * into the origin (thats translate by minus `from`). The complete lookat matrix
 * is then this translation followed by the rotation. Written as matrix
 * multiplication:
 * 
 *   lookat = rotation * translation
 * 
 * Since we're right-handed this equals to first doing the translation and after
 * that doing the rotation. During that multiplication the rotation 3x3 part
 * doesn't change but the translation vector is multiplied with each rotation
 * axis. The dot product is just a more compact way to write the actual
 * multiplications.
 */
Mat4 
M4LookAt(Vec3 from, Vec3 to, Vec3 up) 
{
	Vec3 z = V3MulS(V3Norm(V3Sub(to, from)), -1);
	Vec3 x = V3Norm(V3Cross(up, z));
	Vec3 y = V3Cross(z, x);
	
	return M4(
		x.x, x.y, x.z, -V3Dot(from, x),
		y.x, y.y, y.z, -V3Dot(from, y),
		z.x, z.y, z.z, -V3Dot(from, z),
		0,   0,   0,    1
	);
}


/**
 * Inverts an affine transformation matrix. That are translation, scaling,
 * mirroring, reflection, rotation and shearing matrices or any combination of
 * them.
 * 
 * Implementation details:
 * 
 * - Invert the 3x3 part of the 4x4 matrix to handle rotation, scaling, etc.
 *   correctly (see source).
 * - Invert the translation part of the 4x4 matrix by multiplying it with the
 *   inverted rotation matrix and negating it.
 * 
 * When a 3D point is multiplied with a transformation matrix it is first
 * rotated and then translated. The inverted transformation matrix is the
 * inverse translation followed by the inverse rotation. Written as a matrix
 * multiplication (remember, the effect applies right to left):
 * 
 *   inv(matrix) = inv(rotation) * inv(translation)
 * 
 * The inverse translation is a translation into the opposite direction, just
 * the negative translation. The rotation part isn't changed by that
 * multiplication but the translation part is multiplied by the inverse rotation
 * matrix. It's the same situation as with `m4_look_at()`. But since we don't
 * store the rotation matrix as 3D vectors we can't use the dot product and have
 * to write the matrix multiplication operations by hand.
 * 
 * Sources for 3x3 matrix inversion:
 * 
 * https://www.khanacademy.org/math/precalculus/precalc-matrices/determinants-and-inverses-of-large-matrices/v/inverting-3x3-part-2-determinant-and-adjugate-of-a-matrix
 */
Mat4 
M4InvertAffine(Mat4 matrix) {
	// Create shorthands to access matrix members
	float m00 = matrix.m00,  m10 = matrix.m10,  m20 = matrix.m20,  m30 = matrix.m30;
	float m01 = matrix.m01,  m11 = matrix.m11,  m21 = matrix.m21,  m31 = matrix.m31;
	float m02 = matrix.m02,  m12 = matrix.m12,  m22 = matrix.m22,  m32 = matrix.m32;
	
	// Invert 3x3 part of the 4x4 matrix that contains the rotation, etc.
	// That part is called R from here on.
		
		// Calculate cofactor matrix of R
		float c00 =   m11*m22 - m12*m21,   c10 = -(m01*m22 - m02*m21),  c20 =   m01*m12 - m02*m11;
		float c01 = -(m10*m22 - m12*m20),  c11 =   m00*m22 - m02*m20,   c21 = -(m00*m12 - m02*m10);
		float c02 =   m10*m21 - m11*m20,   c12 = -(m00*m21 - m01*m20),  c22 =   m00*m11 - m01*m10;
		
		// Caclculate the determinant by using the already calculated determinants
		// in the cofactor matrix.
		// Second sign is already minus from the cofactor matrix.
		float det = m00*c00 + m10*c10 + m20 * c20;
		if (fabsf(det) < 0.00001)
			return M4Identity();
		
		// Calcuate inverse of R by dividing the transposed cofactor matrix by the
		// determinant.
		float i00 = c00 / det,  i10 = c01 / det,  i20 = c02 / det;
		float i01 = c10 / det,  i11 = c11 / det,  i21 = c12 / det;
		float i02 = c20 / det,  i12 = c21 / det,  i22 = c22 / det;
	
	// Combine the inverted R with the inverted translation
	return M4(
		i00, i10, i20,  -(i00*m30 + i10*m31 + i20*m32),
		i01, i11, i21,  -(i01*m30 + i11*m31 + i21*m32),
		i02, i12, i22,  -(i02*m30 + i12*m31 + i22*m32),
		0,   0,   0,      1
	);
}

/**
 * Multiplies a 4x4 matrix with a 3D vector representing a point in 3D space.
 * 
 * Before the matrix multiplication the vector is first expanded to a 4D vector
 * (x, y, z, 1). After the multiplication the vector is reduced to 3D again by
 * dividing through the 4th component (if it's not 0 or 1).
 */
Vec3 
M4MulPos(Mat4 matrix, Vec3 position) 
{
	Vec3 result = V3(
		matrix.m00 * position.x + matrix.m10 * position.y + matrix.m20 * position.z + matrix.m30,
		matrix.m01 * position.x + matrix.m11 * position.y + matrix.m21 * position.z + matrix.m31,
		matrix.m02 * position.x + matrix.m12 * position.y + matrix.m22 * position.z + matrix.m32
	);
	
	float w = matrix.m03 * position.x + matrix.m13 * position.y + matrix.m23 * position.z + matrix.m33;
	if (w != 0 && w != 1)
		return V3(result.x / w, result.y / w, result.z / w);
	
	return result;
}

/**
 * Multiplies a 4x4 matrix with a 3D vector representing a direction in 3D space.
 * 
 * Before the matrix multiplication the vector is first expanded to a 4D vector
 * (x, y, z, 0). For directions the 4th component is set to 0 because directions
 * are only rotated, not translated. After the multiplication the vector is
 * reduced to 3D again by dividing through the 4th component (if it's not 0 or
 * 1). This is necessary because the matrix might contains something other than
 * (0, 0, 0, 1) in the bottom row which might set w to something other than 0
 * or 1.
 */
Vec3 
M4MulDir(Mat4 matrix, Vec3 direction) 
{
	Vec3 result = V3(
		matrix.m00 * direction.x + matrix.m10 * direction.y + matrix.m20 * direction.z,
		matrix.m01 * direction.x + matrix.m11 * direction.y + matrix.m21 * direction.z,
		matrix.m02 * direction.x + matrix.m12 * direction.y + matrix.m22 * direction.z
	);
	
	float w = matrix.m03 * direction.x + matrix.m13 * direction.y + matrix.m23 * direction.z;
	if (w != 0 && w != 1)
		return V3(result.x / w, result.y / w, result.z / w);
	
	return result;
}

void 
M4Print(Mat4 matrix) 
{
	M4FPrintP(stdout, matrix, 6, 2);
}

void 
M4PrintP(Mat4 matrix, int width, int precision) 
{
	M4FPrintP(stdout, matrix, width, precision);
}

void 
M4FPrint(FILE* stream, Mat4 matrix) 
{
	M4FPrintP(stream, matrix, 6, 2);
}

void 
M4FPrintP(FILE* stream, Mat4 matrix, int width, int precision) 
{
	Mat4 m = matrix;
	int w = width, p = precision;
	for(int r = 0; r < 4; r++) {
		fprintf(stream, "| %*.*f %*.*f %*.*f %*.*f |\n",
			w, p, m.m[0][r], w, p, m.m[1][r], w, p, m.m[2][r], w, p, m.m[3][r]
		);
	}
}

void
V3FPrintP(FILE *stream, Vec3 v, int width, int precision)
{
    fprintf(stream, "| %*.*f %*.*f %*.*f |",
            width, precision, v.x, 
            width, precision, v.y,
            width, precision, v.z);

}

void
V4FPrintP(FILE *stream, Vec4 v, int width, int precision)
{
    fprintf(stream, "| %*.*f %*.*f %*.*f %*.*f |",
            width, precision, v.x, 
            width, precision, v.y,
            width, precision, v.z,
            width, precision, v.w);
}

#endif // TIM_MATH
