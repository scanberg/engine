/* Copyright (c) <2009> <Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely
*/

#include "dStdAfxMath.h"
#include "dMathDefines.h"
#include "dVector.h"
#include "dMatrix.h"
#include "dQuaternion.h"

// calculate an orthonormal matrix with the front vector pointing on the 
// dir direction, and the up and right are determined by using the GramSchidth procedure
dMatrix dgGrammSchmidt(const dVector& dir)
{
	dVector up;
	dVector right;
	dVector front (dir); 

	front = front.Scale(1.0f / dSqrt (front % front));
	if (dAbs (front.m_z) > 0.577f) {
		right = front * dVector (-front.m_y, front.m_z, 0.0f);
	} else {
		right = front * dVector (-front.m_y, front.m_x, 0.0f);
	}
	right = right.Scale (1.0f / dSqrt (right % right));
	up = right * front;

	front.m_w = 0.0f;
	up.m_w = 0.0f;
	right.m_w = 0.0f;
	return dMatrix (front, up, right, dVector (0.0f, 0.0f, 0.0f, 1.0f));
}


dMatrix dPitchMatrix(dFloat ang)
{
	dFloat cosAng;
	dFloat sinAng;
	sinAng = dSin (ang);
	cosAng = dCos (ang);
	return dMatrix (dVector (1.0f,    0.0f,    0.0f, 0.0f), 
					dVector (0.0f,  cosAng,  sinAng, 0.0f),
					dVector (0.0f, -sinAng,  cosAng, 0.0f), 
					dVector (0.0f,    0.0f,    0.0f, 1.0f)); 

}

dMatrix dYawMatrix(dFloat ang)
{
	dFloat cosAng;
	dFloat sinAng;
	sinAng = dSin (ang);
	cosAng = dCos (ang);
	return dMatrix (dVector (cosAng, 0.0f, -sinAng, 0.0f), 
					dVector (0.0f,   1.0f,    0.0f, 0.0f), 
					dVector (sinAng, 0.0f,  cosAng, 0.0f), 
					dVector (0.0f,   0.0f,    0.0f, 1.0f)); 
}

dMatrix dRollMatrix(dFloat ang)
{
	dFloat cosAng;
	dFloat sinAng;
	sinAng = dSin (ang);
	cosAng = dCos (ang);
	return dMatrix (dVector ( cosAng, sinAng, 0.0f, 0.0f), 
					dVector (-sinAng, cosAng, 0.0f, 0.0f),
					dVector (   0.0f,   0.0f, 1.0f, 0.0f), 
					dVector (   0.0f,   0.0f, 0.0f, 1.0f)); 
}																		 



dMatrix::dMatrix (const dQuaternion &rotation, const dVector &position)
{
	dFloat x2;
	dFloat y2;
	dFloat z2;
	dFloat w2;
	dFloat xy;
	dFloat xz;
	dFloat xw;
	dFloat yz;
	dFloat yw;
	dFloat zw;

	w2 = dFloat (2.0f) * rotation.m_q0 * rotation.m_q0;
	x2 = dFloat (2.0f) * rotation.m_q1 * rotation.m_q1;
	y2 = dFloat (2.0f) * rotation.m_q2 * rotation.m_q2;
	z2 = dFloat (2.0f) * rotation.m_q3 * rotation.m_q3;
	_ASSERTE (dAbs (w2 + x2 + y2 + z2 - dFloat(2.0f)) < dFloat (1.e-2f));

	xy = dFloat (2.0f) * rotation.m_q1 * rotation.m_q2;
	xz = dFloat (2.0f) * rotation.m_q1 * rotation.m_q3;
	xw = dFloat (2.0f) * rotation.m_q1 * rotation.m_q0;
	yz = dFloat (2.0f) * rotation.m_q2 * rotation.m_q3;
	yw = dFloat (2.0f) * rotation.m_q2 * rotation.m_q0;
	zw = dFloat (2.0f) * rotation.m_q3 * rotation.m_q0;

	m_front = dVector (dFloat(1.0f) - y2 - z2, xy + zw				 , xz - yw				  , dFloat(0.0f));
	m_up    = dVector (xy - zw				 , dFloat(1.0f) - x2 - z2, yz + xw				  , dFloat(0.0f));
	m_right = dVector (xz + yw				 , yz - xw				 , dFloat(1.0f) - x2 - y2 , dFloat(0.0f));

	m_posit.m_x = position.m_x;
	m_posit.m_y = position.m_y;
	m_posit.m_z = position.m_z;
	m_posit.m_w = dFloat(1.0f);
}

dMatrix::dMatrix (dFloat pitch, dFloat yaw, dFloat roll, const dVector& location)
{
	dMatrix& me = *this;
	me = dPitchMatrix(pitch) * dYawMatrix(yaw) * dRollMatrix(roll);
	me.m_posit = location;
	me.m_posit.m_w = 1.0f;
}


dVector dMatrix::GetXYZ_EulerAngles () const
{
#if 1
	dFloat yaw;
	dFloat roll;
	dFloat pitch;
	const dFloat minSin = 0.99995f;

	const dMatrix& matrix = *this;

	roll = dFloat(0.0f);
	pitch  = dFloat(0.0f);

	yaw = dAsin (- min (max (matrix[0][2], dFloat(-0.999999f)), dFloat(0.999999f)));
	if (matrix[0][2] < minSin) {
		if (matrix[0][2] > (-minSin)) {
			roll = dAtan2 (matrix[0][1], matrix[0][0]);
			pitch = dAtan2 (matrix[1][2], matrix[2][2]);
		} else {
			pitch = dAtan2 (matrix[1][0], matrix[1][1]);
		}
	} else {
		pitch = -dAtan2 (matrix[1][0], matrix[1][1]);
	}

#ifdef _DEBUG
	dMatrix m (dPitchMatrix (pitch) * dYawMatrix(yaw) * dRollMatrix(roll));
	for (int i = 0; i < 3; i ++) {
		for (int j = 0; j < 3; j ++) {
			dFloat error = dAbs (m[i][j] - matrix[i][j]);
			_ASSERTE (error < 5.0e-2f);
		}
	}
#endif
	return dVector (pitch, yaw, roll, dFloat(0.0f));

#else
	dQuaternion quat (*this);
	return quat.GetXYZ_EulerAngles();
#endif
}



dMatrix dMatrix::Inverse () const
{
	return dMatrix (dVector (m_front.m_x, m_up.m_x, m_right.m_x, 0.0f),
					dVector (m_front.m_y, m_up.m_y, m_right.m_y, 0.0f),
		            dVector (m_front.m_z, m_up.m_z, m_right.m_z, 0.0f),
		            dVector (- (m_posit % m_front), - (m_posit % m_up), - (m_posit % m_right), 1.0f));
}

dMatrix dMatrix::Transpose () const
{
	return dMatrix (dVector (m_front.m_x, m_up.m_x, m_right.m_x, 0.0f),
					dVector (m_front.m_y, m_up.m_y, m_right.m_y, 0.0f),
					dVector (m_front.m_z, m_up.m_z, m_right.m_z, 0.0f),
					dVector (0.0f, 0.0f, 0.0f, 1.0f));
}

dMatrix dMatrix::Transpose4X4 () const
{
	return dMatrix (dVector (m_front.m_x, m_up.m_x, m_right.m_x, m_posit.m_x),
					dVector (m_front.m_y, m_up.m_y, m_right.m_y, m_posit.m_y),
					dVector (m_front.m_z, m_up.m_z, m_right.m_z, m_posit.m_z),
					dVector (m_front.m_w, m_up.m_w, m_right.m_w, m_posit.m_w));
							
}

dVector dMatrix::RotateVector (const dVector &v) const
{
	return dVector (v.m_x * m_front.m_x + v.m_y * m_up.m_x + v.m_z * m_right.m_x,
					v.m_x * m_front.m_y + v.m_y * m_up.m_y + v.m_z * m_right.m_y,
					v.m_x * m_front.m_z + v.m_y * m_up.m_z + v.m_z * m_right.m_z);
}

dVector dMatrix::UnrotateVector (const dVector &v) const
{
	return dVector (v % m_front, v % m_up, v % m_right);
}

dVector dMatrix::RotateVector4x4 (const dVector &v) const
{
	dVector tmp;
	const dMatrix& me = *this;
	for (int i = 0; i < 4; i ++) {
		tmp[i] = v[0] * me[0][i] + v[1] * me[1][i] +  v[2] * me[2][i] +  v[3] * me[3][i];
	}
	return tmp;
}

/*
dVector dMatrix::UnrotateVector4x4 (const dVector &v) const
{
	dVector tmp;
	const dMatrix& me = *this;
	for (int i = 0; i < 4; i ++) {
		tmp[i] = v[0] * me[i][0] + v[1] * me[i][1] +  v[2] * me[i][2] +  v[3] * me[i][3];
	}
	return tmp;
}
*/


dVector dMatrix::TransformVector (const dVector &v) const
{
	return m_posit + RotateVector(v);
}

dVector dMatrix::UntransformVector (const dVector &v) const
{
	return UnrotateVector(v - m_posit);
}


void dMatrix::TransformTriplex (dFloat* const dst, int dstStrideInBytes, dFloat* const src, int srcStrideInBytes, int count) const
{
	dstStrideInBytes /= sizeof (dFloat);
	srcStrideInBytes /= sizeof (dFloat);
	for (int i = 0 ; i < count; i ++ ) {
		dFloat x = src[srcStrideInBytes * i + 0];
		dFloat y = src[srcStrideInBytes * i + 1];
		dFloat z = src[srcStrideInBytes * i + 2];
		dst[dstStrideInBytes * i + 0] = x * m_front.m_x + y * m_up.m_x + z * m_right.m_x + m_posit.m_x;
		dst[dstStrideInBytes * i + 1] = x * m_front.m_y + y * m_up.m_y + z * m_right.m_y + m_posit.m_y;
		dst[dstStrideInBytes * i + 2] = x * m_front.m_z + y * m_up.m_z + z * m_right.m_z + m_posit.m_z;
	}
}

void dMatrix::TransformTriplex (dFloat64* const dst, int dstStrideInBytes, dFloat64* const src, int srcStrideInBytes, int count) const
{
	dstStrideInBytes /= sizeof (dFloat64);
	srcStrideInBytes /= sizeof (dFloat64);
	for (int i = 0 ; i < count; i ++ ) {
		dFloat64 x = src[srcStrideInBytes * i + 0];
		dFloat64 y = src[srcStrideInBytes * i + 1];
		dFloat64 z = src[srcStrideInBytes * i + 2];
		dst[dstStrideInBytes * i + 0] = x * m_front.m_x + y * m_up.m_x + z * m_right.m_x + m_posit.m_x;
		dst[dstStrideInBytes * i + 1] = x * m_front.m_y + y * m_up.m_y + z * m_right.m_y + m_posit.m_y;
		dst[dstStrideInBytes * i + 2] = x * m_front.m_z + y * m_up.m_z + z * m_right.m_z + m_posit.m_z;
	}
}


dMatrix dMatrix::operator* (const dMatrix &B) const
{
	const dMatrix& A = *this;
	return dMatrix (dVector (A[0][0] * B[0][0] + A[0][1] * B[1][0] + A[0][2] * B[2][0] + A[0][3] * B[3][0],
							 A[0][0] * B[0][1] + A[0][1] * B[1][1] + A[0][2] * B[2][1] + A[0][3] * B[3][1],
							 A[0][0] * B[0][2] + A[0][1] * B[1][2] + A[0][2] * B[2][2] + A[0][3] * B[3][2],
	                         A[0][0] * B[0][3] + A[0][1] * B[1][3] + A[0][2] * B[2][3] + A[0][3] * B[3][3]),
					dVector (A[1][0] * B[0][0] + A[1][1] * B[1][0] + A[1][2] * B[2][0] + A[1][3] * B[3][0],
						     A[1][0] * B[0][1] + A[1][1] * B[1][1] + A[1][2] * B[2][1] + A[1][3] * B[3][1],
							 A[1][0] * B[0][2] + A[1][1] * B[1][2] + A[1][2] * B[2][2] + A[1][3] * B[3][2],
							 A[1][0] * B[0][3] + A[1][1] * B[1][3] + A[1][2] * B[2][3] + A[1][3] * B[3][3]),
					dVector (A[2][0] * B[0][0] + A[2][1] * B[1][0] + A[2][2] * B[2][0] + A[2][3] * B[3][0],
							 A[2][0] * B[0][1] + A[2][1] * B[1][1] + A[2][2] * B[2][1] + A[2][3] * B[3][1],
							 A[2][0] * B[0][2] + A[2][1] * B[1][2] + A[2][2] * B[2][2] + A[2][3] * B[3][2],
							 A[2][0] * B[0][3] + A[2][1] * B[1][3] + A[2][2] * B[2][3] + A[2][3] * B[3][3]),
					dVector (A[3][0] * B[0][0] + A[3][1] * B[1][0] + A[3][2] * B[2][0] + A[3][3] * B[3][0],
							 A[3][0] * B[0][1] + A[3][1] * B[1][1] + A[3][2] * B[2][1] + A[3][3] * B[3][1],
							 A[3][0] * B[0][2] + A[3][1] * B[1][2] + A[3][2] * B[2][2] + A[3][3] * B[3][2],
							 A[3][0] * B[0][3] + A[3][1] * B[1][3] + A[3][2] * B[2][3] + A[3][3] * B[3][3]));
}





dVector dMatrix::TransformPlane (const dVector &localPlane) const
{
	dVector tmp (RotateVector (localPlane));  
	tmp.m_w = localPlane.m_w - (localPlane % UnrotateVector (m_posit));  
	return tmp;  
}

dVector dMatrix::UntransformPlane (const dVector &globalPlane) const
{
	dVector tmp (UnrotateVector (globalPlane));
	tmp.m_w = globalPlane % m_posit + globalPlane.m_w;
	return tmp;
}

bool dMatrix::SanityCheck() const
{
	dVector right (m_front * m_up);
	if (dAbs (right % m_right) < 0.9999f) {
		return false;
	}
	if (dAbs (m_right.m_w) > 0.0f) {
		return false;
	}
	if (dAbs (m_up.m_w) > 0.0f) {
		return false;
	}
	if (dAbs (m_right.m_w) > 0.0f) {
		return false;
	}

	if (dAbs (m_posit.m_w) != 1.0f) {
		return false;
	}

	return true;
}


dMatrix dMatrix::Inverse4x4 () const
{
	const dFloat tol = 1.0e-4f;
	dMatrix tmp (*this);
	dMatrix inv (GetIdentityMatrix());
	for (int i = 0; i < 4; i ++) {
		dFloat diag = tmp[i][i];
		if (dAbs (diag) < tol) {
			int j = 0;
			for (j = i + 1; j < 4; j ++) {
				dFloat val = tmp[j][i];
				if (dAbs (val) > tol) {
					break;
				}
			}
			_ASSERTE (j < 4);
			for (int k = 0; k < 4; k ++) {
				tmp[i][k] += tmp[j][k];
				inv[i][k] += inv[j][k];
			}
			diag = tmp[i][i];
		}
		dFloat invDiag = 1.0f / diag;
		for (int j = 0; j < 4; j ++) {
			tmp[i][j] *= invDiag;
			inv[i][j] *= invDiag;
		}
		tmp[i][i] = 1.0f;

		
		for (int j = 0; j < 4; j ++) {
			if (j != i) {
				dFloat pivot = tmp[j][i];
				for (int k = 0; k < 4; k ++) {
					tmp[j][k] -= pivot * tmp[i][k];
					inv[j][k] -= pivot * inv[i][k];
				}
				tmp[j][i] = 0.0f;
			}
		}
	}
	return inv;
}


#if 0
class XXXX 
{
public:
	XXXX ()
	{
		dMatrix s;
		dFloat m = 2.0f;
		for (int i = 0; i < 3; i ++) {
			for (int j = 0; j < 3; j ++) {
				s[i][j] = m; 
				m += (i + 1) + j;
			}
		}
		s.m_posit = dVector (1, 2, 3, 1);
		dMatrix matrix;
		dVector scale;
		dMatrix stretch;
		s.PolarDecomposition (matrix, scale, stretch);
		dMatrix s1 (matrix, scale, stretch);

		dMatrix xxx (dPitchMatrix(30.0f * 3.14159f/180.0f) * dRollMatrix(30.0f * 3.14159f/180.0f));
		dMatrix xxxx (GetIdentityMatrix());
		xxx[0] = xxx[0].Scale (-1.0f);
		dFloat mmm = (xxx[0] * xxx[1]) % xxx[2];
		xxxx[0][0] = 3.0f;
		xxxx[1][1] = 3.0f;
		xxxx[2][2] = 4.0f;

		dMatrix xxx2 (xxx * xxxx);
		mmm = (xxx2[0] * xxx2[1]) % xxx2[2];
		xxx2.PolarDecomposition (matrix, scale, stretch);

		s1 = dMatrix (matrix, scale, stretch);
		s1 = dMatrix (matrix, scale, stretch);

	}
};
XXXX xxx;
#endif


static inline void ROT(dMatrix &a, int i, int j, int k, int l, dFloat s, dFloat tau) 
{
	dFloat g;
	dFloat h;
	g = a[i][j]; 
	h = a[k][l]; 
	a[i][j] = g - s * (h + g * tau); 
	a[k][l] = h + s * (g - h * tau);
}

// from numerical recipes in c
// Jacobian method for computing the eigenvectors of a symmetric matrix
dMatrix dMatrix::JacobiDiagonalization (dVector &eigenValues, const dMatrix& initialMatrix) const
{
	int i;
	int nrot;
	dFloat t;
	dFloat s;
	dFloat h;
	dFloat g;
	dFloat c;
	dFloat sm;
	dFloat tau;
	dFloat theta;
	dFloat thresh;
	dFloat b[3];
	dFloat z[3];
	dFloat d[3];
	dFloat EPSILON = 1.0e-5f;

	dMatrix mat (*this);
	dMatrix eigenVectors (initialMatrix);
	
	b[0] = mat[0][0]; 
	b[1] = mat[1][1];
	b[2] = mat[2][2];

	d[0] = mat[0][0]; 
	d[1] = mat[1][1]; 
	d[2] = mat[2][2]; 

	z[0] = 0.0f;
	z[1] = 0.0f;
	z[2] = 0.0f;

	nrot = 0;
	for (i = 0; i < 50; i++) {
		sm = dAbs(mat[0][1]) + dAbs(mat[0][2]) + dAbs(mat[1][2]);

		if (sm < EPSILON * 1e-5) {
			_ASSERTE (dAbs((eigenVectors.m_front % eigenVectors.m_front) - 1.0f) <EPSILON);
			_ASSERTE (dAbs((eigenVectors.m_up % eigenVectors.m_up) - 1.0f) < EPSILON);
			_ASSERTE (dAbs((eigenVectors.m_right % eigenVectors.m_right) - 1.0f) < EPSILON);

			// order the eigenvalue vectors	
//			eigenVectors = eigenVectors.Inverse();
//			dFloat tmp = (eigenVectors.m_front * eigenVectors.m_up) % eigenVectors.m_right;
//			if (tmp < dFloat(0.0f)) {
//				eigenVectors.m_right = eigenVectors.m_right.Scale (-1.0f);
//			}
			eigenValues = dVector (d[0], d[1], d[2], dFloat (0.0f));
			return eigenVectors.Inverse();
		}

		if (i < 3) {
			thresh = (dFloat)(0.2f / 9.0f) * sm;
		}	else {
			thresh = 0.0;
		}


		// First row
		g = 100.0f * dAbs(mat[0][1]);
		if ((i > 3) && (dAbs(d[0]) + g == dAbs(d[0])) && (dAbs(d[1]) + g == dAbs(d[1]))) {
			mat[0][1] = 0.0f;
		} else if (dAbs(mat[0][1]) > thresh) {
			h = d[1] - d[0];
			if (dAbs(h) + g == dAbs(h)) {
				t = mat[0][1] / h;
			} else {
				theta = dFloat (0.5f) * h / mat[0][1];
				t = dFloat(1.0f) / (dAbs(theta) + dSqrt(dFloat(1.0f) + theta * theta));
				if (theta < 0.0f) {
					t = -t;
				}
			}
			c = dFloat(1.0f) / dSqrt (1.0f + t * t); 
			s = t * c; 
			tau = s / (dFloat(1.0f) + c); 
			h = t * mat[0][1];
			z[0] -= h; 
			z[1] += h; 
			d[0] -= h; 
			d[1] += h;
			mat[0][1] = 0.0f;
			ROT (mat, 0, 2, 1, 2, s, tau); 
			ROT (eigenVectors, 0, 0, 0, 1, s, tau); 
			ROT (eigenVectors, 1, 0, 1, 1, s, tau); 
			ROT (eigenVectors, 2, 0, 2, 1, s, tau); 

			nrot++;
		}


		// second row
		g = 100.0f * dAbs(mat[0][2]);
		if ((i > 3) && (dAbs(d[0]) + g == dAbs(d[0])) && (dAbs(d[2]) + g == dAbs(d[2]))) {
			mat[0][2] = 0.0f;
		} else if (dAbs(mat[0][2]) > thresh) {
			h = d[2] - d[0];
			if (dAbs(h) + g == dAbs(h)) {
				t = (mat[0][2]) / h;
			}	else {
				theta = dFloat (0.5f) * h / mat[0][2];
				t = dFloat(1.0f) / (dAbs(theta) + dSqrt(dFloat(1.0f) + theta * theta));
				if (theta < 0.0f) {
					t = -t;
				}
			}
			c = dFloat(1.0f) / dSqrt(1 + t * t); 
			s = t * c; 
			tau = s / (dFloat(1.0f) + c); 
			h = t * mat[0][2];
			z[0] -= h; 
			z[2] += h; 
			d[0] -= h; 
			d[2] += h;
			mat[0][2]=0.0;
			ROT (mat, 0, 1, 1, 2, s, tau); 
			ROT (eigenVectors, 0, 0, 0, 2, s, tau); 
			ROT (eigenVectors, 1, 0, 1, 2, s, tau); 
			ROT (eigenVectors, 2, 0, 2, 2, s, tau); 
		}

		// trird row
		g = 100.0f * dAbs(mat[1][2]);
		if ((i > 3) && (dAbs(d[1]) + g == dAbs(d[1])) && (dAbs(d[2]) + g == dAbs(d[2]))) {
			mat[1][2] = 0.0f;
		} else if (dAbs(mat[1][2]) > thresh) {
			h = d[2] - d[1];
			if (dAbs(h) + g == dAbs(h)) {
				t = mat[1][2] / h;
			}	else {
				theta = dFloat (0.5f) * h / mat[1][2];
				t = dFloat(1.0f) / (dAbs(theta) + dSqrt(dFloat(1.0f) + theta * theta));
				if (theta < 0.0f) {
					t = -t;
				}
			}
			c = dFloat(1.0f) / dSqrt(1 + t*t); 
			s = t * c; 
			tau = s / (dFloat(1.0f) + c); 

			h = t * mat[1][2];
			z[1] -= h; 
			z[2] += h; 
			d[1] -= h; 
			d[2] += h;
			mat[1][2] = 0.0f;
			ROT (mat, 0, 1, 0, 2, s, tau); 
			ROT (eigenVectors, 0, 1, 0, 2, s, tau); 
			ROT (eigenVectors, 1, 1, 1, 2, s, tau); 
			ROT (eigenVectors, 2, 1, 2, 2, s, tau); 
			nrot++;
		}

		b[0] += z[0]; d[0] = b[0]; z[0] = 0.0f;
		b[1] += z[1]; d[1] = b[1]; z[1] = 0.0f;
		b[2] += z[2]; d[2] = b[2]; z[2] = 0.0f;
	}

	_ASSERTE (0);
	eigenValues = dVector (d[0], d[1], d[2], dFloat (0.0f));
	return GetIdentityMatrix();
} 	




//void dMatrix::PolarDecomposition (dMatrix& orthogonal, dMatrix& symetric) const
void dMatrix::PolarDecomposition (dMatrix& transformMatrix, dVector& scale, dMatrix& stretchAxis, const dMatrix& initialStretchAxis) const
{
	// a polar decomposition decompose matrix A = O * S
	// where S = sqrt (transpose (L) * L)

	// calculate transpose (L) * L 
	dMatrix LL ((*this) * Transpose());


	// check is this si a pure uniformScale * rotation * translation
	dFloat det2 = (LL[0][0] + LL[1][1] + LL[2][2]) * (1.0f / 3.0f);

	dFloat invdet2 = 1.0f / det2;

	dMatrix pureRotation (LL);
	pureRotation[0] = pureRotation[0].Scale (invdet2);
	pureRotation[1] = pureRotation[1].Scale (invdet2);
	pureRotation[2] = pureRotation[2].Scale (invdet2);

	dFloat det = (pureRotation[0] * pureRotation[1]) % pureRotation[2];
	if (dAbs (det - 1.0f) < 1.e-5f) {
		// this is a pure scale * rotation * translation
		det = dSqrt (det2);
		scale[0] = det;
		scale[1] = det;
		scale[2] = det;
		det = 1.0f/ det;
		transformMatrix.m_front = m_front.Scale (det);
		transformMatrix.m_up = m_up.Scale (det);
		transformMatrix.m_right = m_right.Scale (det);
		transformMatrix[0][3] = 0.0f;
		transformMatrix[1][3] = 0.0f;
		transformMatrix[2][3] = 0.0f;
		transformMatrix.m_posit = m_posit;
		stretchAxis = GetIdentityMatrix();
		
	} else {
		stretchAxis = LL.JacobiDiagonalization(scale, initialStretchAxis);

		// I need to deal with buy seeing of some of the Scale are duplicated
		// do this later (maybe by a given rotation around the non uniform axis but I do not know if it will work)
		// for now just us the matrix

		scale[0] = dSqrt (scale[0]);
		scale[1] = dSqrt (scale[1]);
		scale[2] = dSqrt (scale[2]);
		scale[3] = 1.0f;


//		scaledAxis[0] = stretchAxis[0].Scale (scale[0]);
//		scaledAxis[1] = stretchAxis[1].Scale (scale[1]);
//		scaledAxis[2] = stretchAxis[2].Scale (scale[2]);
//		scaledAxis[3] = stretchAxis[3];
//		dMatrix symetric (stretchAxis.Transpose() * scaledAxis);
//		transformMatrix = symetric.Inverse4x4 () * (*this);

		dMatrix scaledAxis;
		scaledAxis[0] = stretchAxis[0].Scale (1.0f / scale[0]);
		scaledAxis[1] = stretchAxis[1].Scale (1.0f / scale[1]);
		scaledAxis[2] = stretchAxis[2].Scale (1.0f / scale[2]);
		scaledAxis[3] = stretchAxis[3];
		dMatrix symetricInv (stretchAxis.Transpose() * scaledAxis);

		transformMatrix = symetricInv * (*this);
		transformMatrix.m_posit = m_posit;
	}
}

dMatrix::dMatrix (const dMatrix& transformMatrix, const dVector& scale, const dMatrix& stretchAxis)
{
	dMatrix scaledAxis;
	scaledAxis[0] = stretchAxis[0].Scale (scale[0]);
	scaledAxis[1] = stretchAxis[1].Scale (scale[1]);
	scaledAxis[2] = stretchAxis[2].Scale (scale[2]);
	scaledAxis[3] = stretchAxis[3];

	*this = stretchAxis.Transpose() * scaledAxis * transformMatrix;
}






