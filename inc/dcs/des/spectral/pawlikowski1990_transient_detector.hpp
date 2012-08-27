/**
 * \file dcs/des/spectral/pawlikowski1990_transient_detector.hpp
 *
 * \brief Transient phase detector using Schruben test.
 *
 * Copyright (C) 2009-2012  Distributed Computing System (DCS) Group,
 *                          Computer Science Institute,
 *                          Department of Science and Technological Innovation,
 *                          University of Piemonte Orientale,
 *                          Alessandria (Italy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \author Cosimo Anglano (cosimo.anglano@di.unipmn.it)
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */

#ifndef DCS_DES_SPECTRAL_PAWLIKOWSKI1990_TRANSIENT_DETECTOR_HPP
#define DCS_DES_SPECTRAL_PAWLIKOWSKI1990_TRANSIENT_DETECTOR_HPP


#include <algorithm>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/traits.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublasx/operation/size.hpp>
#include <cmath>
#include <cstddef>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/math/constants.hpp>
#include <dcs/math/function/sqr.hpp>
#include <dcs/math/stats/distribution/students_t.hpp>
#include <stdexcept>
#include <limits>
#include <utility>
#include <vector>


namespace dcs { namespace des { namespace spectral {

namespace detail { namespace /*<unnamed>*/ {

template <typename VectorT, typename UIntT, typename RealT>
RealT schruben_statistic(::boost::numeric::ublas::vector_expression<VectorT> const& x, UIntT n_v, RealT var)
{
	typedef RealT real_type;
	typedef typename ::boost::numeric::ublas::vector_traits<VectorT>::size_type size_type;

	size_type n_t = ::boost::numeric::ublasx::size(x);

	real_type mean = 0;
	for (size_type i = 0; i < n_t; ++i)
	{
		mean += x()(i);
	}
	mean /= real_type(n_t);

	real_type sum1 = 0;
	real_type sum2 = 0;
	for (size_type k = 1; k <= n_t; ++k)
	{
		sum2 += x()(k-1);
		sum1 += k * (real_type(1) - real_type(k) / real_type(n_t)) * (mean - sum2 / real_type(k));
	}
	return sum1 * ::std::sqrt(45) / (n_t * ::std::sqrt(n_t * n_v * var));
}


/**
 * \brief Single entry of the look-up table of C1 and C2 values.
 */
template <typename RealT, typename UIntT>
struct k_d_entry
{
	UIntT k;
	UIntT d;
	RealT c1;
	UIntT c2;
};


/**
 * \brief Look-up table of C1 and C2 values indexed by K and d.
 */
template <typename RealT, typename UIntT>
struct k_d_table
{
	typedef RealT real_type;
	typedef UIntT uint_type;

	static const k_d_entry<real_type,uint_type> table[9];
//		{uint_type(25), uint_type(0), real_type(0.987),  uint_type(76)},
//		{uint_type(25), uint_type(1), real_type(0.948),  uint_type(18)},
//		{uint_type(25), uint_type(2), real_type(0.882),   uint_type(7)},
//		{uint_type(25), uint_type(3), real_type(0.784),   uint_type(3)},
//		{uint_type(50), uint_type(0), real_type(0.994), uint_type(154)},
//		{uint_type(50), uint_type(1), real_type(0.974),  uint_type(37)},
//		{uint_type(50), uint_type(2), real_type(0.941),  uint_type(16)},
//		{uint_type(50), uint_type(3), real_type(0.895),   uint_type(8)},
//		/*<<<*/{uint_type(5), uint_type(2), real_type(0.882),  uint_type(7)}/*>>>*/
};

template <typename RealT, typename UIntT>
const k_d_entry<RealT,UIntT> k_d_table<RealT,UIntT>::table[] = {
		/* k, d, c1, c2 */
		{UIntT(25), UIntT(0), RealT(0.987),  UIntT(76)},
		{UIntT(25), UIntT(1), RealT(0.948),  UIntT(18)},
		{UIntT(25), UIntT(2), RealT(0.882),   UIntT(7)},
		{UIntT(25), UIntT(3), RealT(0.784),   UIntT(3)},
		{UIntT(50), UIntT(0), RealT(0.994), UIntT(154)},
		{UIntT(50), UIntT(1), RealT(0.974),  UIntT(37)},
		{UIntT(50), UIntT(2), RealT(0.941),  UIntT(16)},
		{UIntT(50), UIntT(3), RealT(0.895),   UIntT(8)},
		/*<<<*/{UIntT(5), UIntT(2), RealT(0.882),  UIntT(7)}/*>>>*/
};


template <typename RealT, typename UIntT>
::std::pair<RealT,UIntT> lookup_periodogram_delta(UIntT k, UIntT d)
{
	::std::size_t n = sizeof(k_d_table<RealT,UIntT>::table) / sizeof(k_d_table<RealT,UIntT>::table[0]);
	k_d_entry<RealT,UIntT> const* p = &(k_d_table<RealT,UIntT>::table[0]);
	while (--n)
	{
		if (p->k == k && p->d == d)
		{
			return ::std::make_pair(p->c1, p->c2);
		}
		++p;
	}

	DCS_DEBUG_TRACE("Heidelberger-Welch: Unrecognised combination of Periodogram Points and Polynomial Degree = (" << k << "," << d << ").");

	throw ::std::runtime_error("[dcs::des::spectral::detail::lookup_periodogram_delta] Unrecognised combination of Periodogram Points and Polynomial Degree in the Heidelberger-Welch method.");
}


/**
 * \brief Given observations x[0],...,x[n-1], calculate periodogram
 *  values p[0],...,p[m-1] where p[j-1] = PI(j/n)
 */
template <typename Vector1T, typename Vector2T>
void periodogram(::boost::numeric::ublas::vector_expression<Vector1T> const& x,
				 ::boost::numeric::ublas::vector_container<Vector2T>& p)
{
	namespace ublas = ::boost::numeric::ublas;
	namespace ublasx = ::boost::numeric::ublasx;

	typedef typename ublas::promote_traits<
				typename ublas::vector_traits<Vector1T>::size_type,
				typename ublas::vector_traits<Vector2T>::size_type
			>::promote_type size_type;
	typedef typename ublas::promote_traits<
				typename ublas::vector_traits<Vector1T>::value_type,
				typename ublas::vector_traits<Vector2T>::value_type
			>::promote_type value_type;
	typedef typename ublas::type_traits<value_type>::real_type real_type;

	size_type x_size = ublasx::size(x);
	size_type p_size = ublasx::size(p);

	for (size_type n = 1; n <= p_size; ++n)
	{
		/* Real and imaginary parts of sum */
		real_type rsum = 0;
		real_type isum = 0;

		for (size_type j = 0; j < x_size; ++j)
		{
			real_type theta = - (real_type(2) * ::dcs::math::constants::pi<real_type>::value * j * n) / real_type(x_size);
			rsum += x()(j) * ::std::cos(theta);
			isum += x()(j) * ::std::sin(theta);
		}
		p()(n-1) = (rsum * rsum + isum * isum) / real_type(x_size);
	}
}


/**
 * \brief Given periodogram values P[0]..P[2K-1], calculate Lfj[0..K-1] 
 *  where Lfj[j] = L(f_{j+1}) = log((P[2j]+P[2j+1])/2)
 */
template <typename Vector1T, typename Vector2T, typename RealT>
void log_average_pairs_and_offset(::boost::numeric::ublas::vector_expression<Vector1T> const& p,
								  ::boost::numeric::ublas::vector_container<Vector2T>& lfj,
								  RealT offset)
{
	namespace ublas = ::boost::numeric::ublas;
	namespace ublasx = ::boost::numeric::ublasx;

	typedef RealT real_type;
	typedef typename ublas::promote_traits<
				typename ublas::vector_traits<Vector1T>::size_type,
				typename ublas::vector_traits<Vector2T>::size_type
			>::promote_type size_type;

	size_type lfj_size = ublasx::size(lfj);

	for (size_type j = 0; j < lfj_size; ++j)
	{
		real_type x = (p()(2*j) + p()(2*j+1)) / real_type(2);

		// It is possible to get zero here, e.g. if X[] are all equal.
		// Substitute a very small number to avoid singularities.
		if (x == real_type(0))
		{
			//x = 1e-38;
			x = ::std::numeric_limits<real_type>::min();
		}
		lfj()(j) = ::std::log(x) + offset;
	}
}


/**
 * \brief Given N points x[j] and a degree k, generate a sequence of k+1
 *  polynomials p[i](x) of degree 0 to k, which are orthonormal on the inner
 *  product
 *  \f[
 *      \langlep_1, p_2\rangle = p_1(x[0])p_2(x[0]) + \cdots + p_1(x[N-1])p_2(x[N-1])
 *
 *   Inputs:	k, N		Dimensions of tables.
 *		x[N]		Values of x on which the polynomials 
 *				P[i](x) are to be orthogonal.
 *
 *   Outputs:	P[k+1][N]	Table of values of the polynomials:
 *				P[i][j] is the value of polynomial i
 *				at the point x[j].
 *		A[k+1], B[k+1]	Auxiliary tables suitable for passing
 *				to OrthogonalPolynomialValues to
 *				calculate P[i](x) for other values of x.
 *
 *   The values are generated using the recurrence relation
 *
 *      p[i+1](x) = a[i](x-b[i])p[i](x) - c[i]p[i-1](x)
 *
 *   where
 *
 *      p[-1] = 0
 *      a[i] = a[i+1][i+1] / a[i][i]
 *      b[i] = <x p[i](x), p[i](x)> / s[i]
 *      c[i] = a[i]S[i] / a[i-1]s[i-1]
 *
 *   and a[i][i] is the leading coefficient of p[i].
 *
 *   To normalise the polynomials we choose the leading 
 *   coefficients a[i][i] so that s[i] = 1 for all i.
 *   To obtain s[0] = 1 we require
 *
 *      p[0](x) = 1/sqrt(N)
 *
 *   If we rewrite the recurrence relation as
 *
 *      p[i+1](x) = a[i]q[i+1](x)
 *      q[i+1](x) = (x-b[i])p[i](x) - (1/a[i-1])p[i-1](x)
 *
 *   then to obtain s[i+1] = 1 we require
 *
 *      a[i] = 1/sqrt(<q[i+1], q[i+1]>)
 *
 *   The tables a[i], b[i] are passed back to the caller to
 *   facilitate calculation of p[i](x) for other values of x
 *   using orthogonal_polynomial_values.
 */

template <typename Vector1T, typename MatrixT, typename Vector2T, typename Vector3T>
static void orthogonal_polynomial_tables(::boost::numeric::ublas::vector_expression<Vector1T> const& x,
										 ::boost::numeric::ublas::matrix_expression<MatrixT>& p,
										 ::boost::numeric::ublasx::vector_container<Vector2T>& a,
										 ::boost::numeric::ublasx::vector_container<Vector3T>& b) 
{
	namespace ublas = ::boost::numeric::ublas;
	namespace ublasx = ::boost::numeric::ublasx;

	typedef typename ublas::promote_traits<
					typename ublas::vector_traits<Vector1T>::size_type,
					typename ublas::promote_traits<
						typename ublas::matrix_traits<MatrixT>::size_type,
						typename ublas::promote_traits<
							typename ublas::vector_traits<Vector2T>::size_type,
							typename ublas::vector_traits<Vector3T>::size_type
						>::promote_type
					>::promote_type
				>::promote_type size_type;
	typedef typename ublas::promote_traits<
					typename ublas::vector_traits<Vector1T>::value_type,
					typename ublas::promote_traits<
						typename ublas::matrix_traits<MatrixT>::value_type,
						typename ublas::promote_traits<
							typename ublas::vector_traits<Vector2T>::value_type,
							typename ublas::vector_traits<Vector3T>::value_type
						>::promote_type
					>::promote_type
				>::promote_type value_type;
	typedef typename ublas::type_traits<value_type>::real_type real_type;

	size_type N = ublasx::size(x);
	size_type k = ublasx::size(b)-1;

	ublas::vector<real_type> q(N);

	/* Calculate p[0](x) */
	real_type p0 = real_type(1)/::std::sqrt(N);
	for (size_type j = 0; j < N; ++j)
	{
		p()(0,j) = p0;
	}

	/* Calculate P[1](x) to P[k](x) */
	for (size_type i = 0; i < k; ++i)
	{
		/* Calculate b[i] */
		b()(i) = 0;
		for (size_type j = 0; j < N; ++j)
		{
			b()(i) += x()(j) * ::dcs::math::sqr(p()(i,j));
		}

		/* Calculate Q[i+1](x) */
		for (size_type j = 0; j < N; ++j)
		{
			q(j) = (x()(j) - b()(i)) * p()(i,j) - (i > 0 ? (p()(i-1,j) / a()(i-1)) : real_type(0));
		}

		/* Calculate A[i] */
		real_type s = 0;
		for (size_type j = 0; j < N; ++j)
		{
			s += ::dcs::math::sqr(q(j));
		}
		a()(i) = real_type(1)/::std::sqrt(s);

		/* Calculate P[i+1](x) */
		for (size_type j = 0; j < N; ++j)
		{
			p()(i+1,j) = a()(i) * q(j);
		}
	}
}


/**
 * \brief Given a set of tables generated by OrthogonalPolynomialTables,
 *  calculate the values of P[i](x) and P'[i](x), i = 0..k, for a given value
 *  of x.
 *
 *   Inputs:	k, N		Dimensions of tables.
 *		A[k+1], B[k+1]	Tables generated by OrthogonalPolynomialTables.
 *		x		Value of x at which to evaluate polynomials.
 *
 *   Outputs:	P[k+1]		P[i] = value of polynomial i at x
 *              dP[k+1]         dP[i] = slope of polynomial i at x
 */
template <typename Vector1T, typename Vector2T, typename RealT, typename UIntT, typename Vector3T, typename Vector4T>
void orthogonal_polynomial_values(::boost::numeric::ublas::vector_expression<Vector1T> const& a,
								  ::boost::numeric::ublas::vector_expression<Vector2T> const& b,
								  UIntT N,
								  RealT x,
								  ::boost::numeric::ublas::vector_container<Vector3T>& p,
								  ::boost::numeric::ublas::vector_container<Vector4T>& dp) 
{
	namespace ublas = ::boost::numeric::ublas;
	namespace ublasx = ::boost::numeric::ublasx;

	typedef typename ublas::promote_traits<
				typename ublas::vector_traits<Vector1T>::size_type,
				typename ublas::promote_traits<
					typename ublas::vector_traits<Vector2T>::size_type,
					typename ublas::promote_traits<
						typename ublas::vector_traits<Vector3T>::size_type,
						typename ublas::vector_traits<Vector4T>::size_type
					>::promote_type
				>::promote_type
			>::promote_type size_type;
	typedef typename ublas::promote_traits<
				typename ublas::vector_traits<Vector1T>::value_type,
				typename ublas::promote_traits<
					typename ublas::vector_traits<Vector2T>::value_type,
					typename ublas::promote_traits<
						typename ublas::vector_traits<Vector3T>::value_type,
						typename ublas::vector_traits<Vector4T>::value_type
					>::promote_type
				>::promote_type
			>::promote_type value_type;
	typedef typename ublas::promote_traits<
				RealT,
				typename ublas::type_traits<value_type>::real_type
			>::promote_type real_type;
	typedef UIntT uint_type;

	size_type k = ublasx::size(a)-1;

	p()(0) = real_type(1)/sqrt(N);
	for (size_type i = 0; i < k; ++i)
	{
		p()(i+1) = a()(i) * ((x - b()(i)) * p()(i) - (i > 0 ? (p()(i-1) / a()(i-1)) : real_type(0)));
	}

	dp()(0) = 0;
	for (size_type i = 0; i < k; ++i)
	{
		dp()(i+1) = a()(i) * (p()(i) + (x - b()(i)) * dp()(i) - (i > 0 ? (dp()(i-1) / a()(i-1)) : real_type(0)));
	}
}


/**
 * \brief Least squares polynomial fitting
 *
 * Given a sequence of N points (x[i], f[i]), 0 <= i = N,
 * fit a polynomial p of degreek k to them using least squares, and
 * return the value of p(0). Also return in dp0 the slope
 * of p at x=0.
 *
 * The general plan, based on Samuel D. Conte and Carl de Boor,
 * Elementary Numerical Analysis, 3rd. Ed, pp. 248-254,
 * is as follows.
 *
 * First we find a sequence of k+1 polynomials 
 *
 *    phi[i](x) = a[i][0] + a[i][1]*x + ... a[i][i]*x^i
 *
 * which are orthonormal on the points of interest. 
 * Then we express p(x) as
 *
 *    p(x) = c[0]*phi[0](x) + ... + c[k]*phi[k](x)
 *
 * and find the coefficients c[0]...c[k] which satisfy
 * the least-squares condition. For orthogonal phi[i],
 * this reduces to solving a diagonal system of linear
 * equations. Then we substitute to find p(0).
 */
template <typename Vector1T, typename Vector2T, typename UIntT, typename RealT>
RealT least_squares_poly_at0(::boost::numeric::ublas::vector_expression<Vector1T> const& x,
							 ::boost::numeric::ublas::vector_expression<Vector2T> const& f,
							 UIntT k,
							 RealT& dp0)
{
	namespace ublas = ::boost::numeric::ublas;
	namespace ublasx = ::boost::numeric::ublasx;

	typedef typename ublas::promote_traits<
				typename ublas::vector_traits<Vector1T>::size_type,
				typename ublas::vector_traits<Vector2T>::size_type
			>::promote_type size_type;
	typedef typename ublas::promote_traits<
				typename ublas::vector_traits<Vector1T>::value_type,
				typename ublas::vector_traits<Vector2T>::value_type
			>::promote_type value_type;
	typedef typename ublas::promote_traits<
				RealT,
				typename ublas::type_traits<value_type>::real_type
			>::promote_type real_type;
	typedef UIntT uint_type;
	typedef ublas::vector<real_type> vector_type;
	typedef ublas::matrix<real_type> matrix_type;

	size_type N = ublasx::size(x);

	vector_type c(k+1);		/* Coefficients of expansion of p(x) in phi's */
	vector_type phi_0(k+1);	/* phi_0[i] = phi[i](0) */
	vector_type dphi_0(k+1);	/* dphi_0[i] = phi'[i](0) */
	matrix_type phi(k+1, N);	/* phi[i][j] = phi[i](x[j]) */
	real_type p0;		/* p0 = p(0) */

	/* Tables used by orthogonal polynomial routines */
	vector_type a(k+1);
	vector_type b(k+1);

//	/* Allocate storage for phi[i][j] */
//	real _phi[k+1][N];
//	for (i = 0; i <= k; i++)
//	{
//		phi[i] = _phi[i];
//	}

	orthogonal_polynomial_tables(x, phi, a, b);

	/* c[i] = f.phi[i] / phi[i].phi[i] */
	for (uint_type i = 0; i <= k; ++i)
	{
		c(i) = 0;
		for (size_type n = 0; n < N; ++n)
		{
			c(i) += f()(n) * phi(i,n);		/* Note: phi[i].phi[i] = 1 */
		}
	}

	/* phi_0[i] = phi[i](0) */
	orthogonal_polynomial_values(a, b, N, 0, phi_0, dphi_0);

	/* p(0) = c . phi_0 */
	p0 = 0;
	for (uint_type i = 0; i <= k; ++i)
	{
		p0 += c(i) * phi_0(i);
	}

	/* p'(0) = c . dphi_0 */
	dp0 = 0;
	for (uint_type i = 0; i <= k; ++i)
	{
		dp0 += c(i) * dphi_0(i);
	}

	return p0;
}


enum slope_protection_category
{
	SLOPE_PROTECTION_OFF = 0,
	SLOPE_PROTECTION_UNCONDITIONAL,
	SLOPE_PROTECTION_CONDITIONAL
};

/**
 * \brief Estimate the variance of a sequence of observations and return the
 *  number of degrees of freedom.
 *
 * \param x The sequence of observation
 *
 * \see Philip Heidelberger and Peter D. Welch. "A spectral method for confidence interval generation and run length control in simulations. Communications of the ACM, vol. 24(4) (1981)
 *
 */
template <typename VectorT, typename UIntT, typename RealT>
bool spectral_anova(::boost::numeric::ublas::vector_expression<VectorT> const& x,
					UIntT num_per_points,
					UIntT delta,
					slope_protection_category slope_protection,
					RealT &var,
					UIntT &kappa)
{
	namespace ublas = ::boost::numeric::ublas;
	namespace ublasx = ::boost::numeric::ublasx;

	typedef typename ublas::vector_traits<VectorT>::size_type size_type;
	typedef typename ublas::promote_traits<
				RealT,
				typename ublas::type_traits<typename ublas::vector_traits<VectorT>::value_type>::real_type
			>::promote_type real_type;
	typedef UIntT uint_type;
	typedef ublas::vector<real_type> vector_type;

	// The number of points used from the periodogram (default: 25)
	//uint_type K = 25;
	// The degree of the polynomial fitted to the periodogram (default: 2).
	//uint_type d = 2;
	//slope_protection_category slope_protection = SLOPE_PROTECTION_OFF;
	real_type c1;	/* Normalising constant = C1(K,d) */
	vector_type p(2*num_per_points); /* P[j] = I(j/n) (periodogram values) */
	vector_type f(num_per_points);	/* f[j-1] = (4j-1)/(2n) */
	vector_type l(num_per_points);	/* L[j] = log((P[2j-1]+P[2j])/2) */

	::std::pair<real_type,uint_type> c1_k;
	c1_k = lookup_periodogram_delta<real_type>(
		num_per_points,
		delta
	);
	c1 = c1_k.first;
	kappa = c1_k.second;

	periodogram(x, p);

	size_type N = ublasx::size(x);

	for (uint_type j = 1; j <= num_per_points; ++j)
	{
		f(j-1) = (4*j - 1) / (real_type(2) * N);
	}

	log_average_pairs_and_offset(p, l, real_type(0.270));
	real_type da0;
	real_type a0 = least_squares_poly_at0(f, l, delta, da0);
	real_type px0 = c1 * ::std::exp(a0);
	var = px0 / real_type(N);

	/* If fitted polynomial is curving the wrong way, fall back on delta = 0 */
	bool slope_corrected = false;
	if (slope_protection != SLOPE_PROTECTION_OFF)
	{
		if (da0 > 0)
		{
    		//DCS_DEBUG_TRACE("Old: p(0) = " << a0 << ", p'(0) = " << da0 << ", variance = " << var);

			delta = 0;
			int kappa_2;
			c1_k = lookup_periodogram_delta<real_type>(num_per_points, delta);
			c1 = c1_k.first;
			kappa_2 = c1_k.second;
			a0 = least_squares_poly_at0(f, l, delta, da0);
			px0 = c1 * ::std::exp(a0);
			real_type var_2 = px0 / real_type(N);

    		//DCS_DEBUG_TRACE("New: p(0) = " << a0 << ", p'(0) = " << da0 << ", variance = " << var_2);

			switch (slope_protection)
			{
				case SLOPE_PROTECTION_UNCONDITIONAL:
					slope_corrected = true;
					break;
				case SLOPE_PROTECTION_CONDITIONAL:
					if (var_2 > var)
					{
						slope_corrected = true;
					}
					break;
				default:
					// quiet the compiler
					break;
			}
			if (slope_corrected)
			{
				var = var_2;
				kappa = kappa_2;

				//DCS_DEBUG_TRACE(">>>> Accepted");
			}
			//else
			//{
			//	DCS_DEBUG_TRACE(">>>> Rejected");
			//}
		}
	}
	if (var < 0)
	{
		//TODO: what to do?
		DCS_DEBUG_TRACE("!!! Spectral: Negative variance: " << var);

		throw ::std::runtime_error("[dcs::des::spectral::detail::spectral_anova] Negative variance.");
	}

	return slope_corrected;
}

}} // Namespace detail::<unnamed>


/**
 * \brief Transient phase detector using Schruben test.
 *
 * \tparam RealT The type used for real numbers.
 * \tparam UIntT The type used for unsigned integral numbers.
 *
 * This transient phase detection is based on methods described in
 *
 *  K. Pawlikowski. "Steady state simulation of queueing processes: A survey of
 *  problems and solutions", ACM Computing Surveys, 22(2):123-170 (1990)
 *
 * \todo Make heuristic customizable; for instance, create a class for each
 *  heuristic; this has the advantage that heuristic parameters are confined
 *  to the class corresponding to the related heuristic.
 *
 * \author Cosimo Anglano (cosimo.anglano@di.unipmn.it)
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename RealT, typename UIntT>
class pawlikowski1990_transient_detector
{
	public: typedef RealT real_type;
	public: typedef UIntT uint_type;
	public: typedef ::std::size_t size_type;
	public: typedef ::std::pair<real_type,real_type> sample_type;
	public: typedef ::std::vector<sample_type> sample_container;
	private: typedef ::boost::numeric::ublas::vector<real_type> vector_type;


	/// Constant for setting the duration of batch size determination
	/// to infinity.
	//Note: the use of a type-dependent value for representing the infinity
	//constant is not a limitation to the simulation length since if we exceed
	//that value the observation counter will overflow
	//public: static const uint_type num_obs_infinity = 0;
	public: static const uint_type num_obs_infinity; // = ::dcs::math::constants::infinity<uint_type>::value;


	// Default values according to [Pawlikowski, 1990].

	//public: static const uint_type default_n0_max = 50000;
	public: static const uint_type default_n0_max;/// = num_obs_infinity;
	public: static const uint_type default_max_heuristic_length;/// = default_n0_max/2;
	public: static const uint_type default_min_num_mean_crossings = 25;
	public: static const real_type default_gamma;// = 0.5;
	public: static const real_type default_gamma_v;// = 2;
	public: static const uint_type default_n_v = 100;
	public: static const real_type default_alpha_t;// = 0.05;
	public: static const real_type default_safety_factor;// = 1;
	public: static const uint_type default_n_ap = 25;
	public: static const uint_type default_delta = 2;
	public: static const real_type default_eps;// = 1e-5;
	public: static const detail::slope_protection_category default_slope_protection = detail::SLOPE_PROTECTION_OFF;
	//private: static const size_type initial_buf_size = 1024;


	/**
	 * \brief A constructor.
	 * \param n0_max The maximum allowed length of the initial transient period.
	 * \param gamma The <em>exchange</em> coefficient, determining the number of
	 *  new observations included in each sequential test for stationarity.
	 *  Must be greater than 0.
	 * \param gamma_v The <em>safety</em> coefficient for the estimator of
	 *  variance to represent the steady state.
	 *  Must be less than or equal to 2.
	 * \param n_v The length of the sequence used for estimating the
	 *  steady-state variance.
	 * \param alpha_t The significance level of the stationarity test.
	 *  Must be between 0 and 1 (extremes excluded).
	 * \param safety_factor
	 * \param n_ap The number of points of the averaged periodogram used to fit
	 *  it to a polynomial by applying the least-squares procedure.
	 *  Must hold that n_ap is less than or equal to \a n_v / 4.
	 * \param delta The degree of the polynomial fitted to the logarithm of the
	 *  averaged periodogram.
	 *  Must be greater than 0.
	 * \param eps The tolerance used for floating point comparisons.
	 */
	public: explicit pawlikowski1990_transient_detector(uint_type n0_max=default_n0_max,
														real_type gamma=default_gamma,
														real_type gamma_v=default_gamma_v,
														uint_type n_v=default_n_v,
														real_type alpha_t=default_alpha_t,
														real_type safety_factor=default_safety_factor,
														uint_type n_ap=default_n_ap,
														uint_type delta=default_delta, 
														real_type eps = default_eps)
		: num_obs_(0),
		  detect_aborted_(false),
		  detected_trans_(false),
		  min_num_mean_crossings_(default_min_num_mean_crossings),
		  sum_(0),
		  n0_star_(0),
		  n0_(0),
		  n0_max_(n0_max),
		  max_heuristic_len_(default_max_heuristic_length),
		  obs_(n_v),
		  weights_(n_v),
		  num_buf_obs_(0),
		  gamma_(gamma),
		  gamma_v_(gamma_v),
		  alpha_t_(alpha_t),
		  n_v_(n_v),
		  n_t_(0),
		  gamma_n0_star_(0),
		  //delta_n_(0),
		  safety_factor_(safety_factor),
		  safe_num_obs_(0),
		  n_ap_(n_ap),
		  delta_(delta),
		  slope_protection_(default_slope_protection),
		  eps_(eps)
	{
		// post-conditions
		DCS_ASSERT(
			alpha_t_ > 0 && alpha_t_ < 1,
			throw ::std::range_error("Significance level is out of range.")
		);
		DCS_ASSERT(
			gamma_ > 0,
			throw ::std::range_error("Exchange coefficient is out of range.")
		);
		DCS_ASSERT(
			gamma_v_ <= 2,
			throw ::std::range_error("Safety coefficient is out of range.")
		);
		DCS_ASSERT(
			n_ap_ <= n_v_/4,
			throw ::std::range_error("Number of periodogram points is out of range.")
		);
		DCS_ASSERT(
			delta > 0,
			throw ::std::range_error("Polynomial degree is out of range.")
		);

		// empty
	}


	/**
	 * \brief Try to detect the transient period with the given new observation.
	 * \return \c true if the transient phase is detected; \c false otherwise.
	 */
	public: bool detect(real_type value, real_type weight)
	{
		// Check if transient phase already done.
		if (detect_aborted_)
		{
			return false;
		}
		if (detected_trans_)
		{
			return true;
		}

		if (n0_max_ != num_obs_infinity && (n0_+n_t_) > n0_max_)
		{
			// Transient phase not detected but reached maximum check length

			DCS_DEBUG_TRACE("Failed to detect transient phase after " << num_buf_obs_ << " observations: maximum allowed transient phase length reached.");

			detect_aborted_ = true;
			detected_trans_ = false;
			return false;
		}

		++num_obs_;

		// Buffer this observation
		if (num_buf_obs_ == obs_.size())
		{
			// enlarge the observations vector
			obs_.resize(obs_.size() * 1.5);
			//obs_.resize(obs_.size() * 2);
			weights_.resize(weights_.size() * 1.5);
		}
		obs_[num_buf_obs_] = value;
		weights_[num_buf_obs_] = weight;
		++num_buf_obs_;

		//if (n0_ == 0)
		if (n0_star_ == 0)
		{
			// STEP 1: find a first approximation of the transient length by
			// applying a specific heuristic

			// Still in transient
			heuristic_phase(value);

			return false;
		}

		if (safe_num_obs_ == 0)
		{
			schruben_phase();

			if (!detected_trans_)
			{
				// Still in transient
				return false;
			}

			// If we are here it means that transient phase has been detected
			// (i.e., detected_trans_==true).
			// Determine the minimum number of observations to collect.
			// As default the minimum number is equal to the number of
			// observations just needed to detect the transient phase (i.e., to
			// come here).

			safe_num_obs_ = static_cast<uint_type>(safety_factor_*n0_star_);
		}

		//if ((n0_star_ > 0) && (num_obs_ >= safe_num_obs_))
		//if ((n0_ > 0) && (num_obs_ >= safe_num_obs_))
		if (!detected_trans_ && (num_obs_ >= safe_num_obs_))
		{
			// force leaving transient detection.

			DCS_DEBUG_TRACE("Assuming transient has been detected after " << safe_num_obs_ << " observations: max safe number reached.");

			// FIXME: should throw an exception of treat this case differently?
			//detect_aborted_ = true;
			detected_trans_ = true;
			n0_ = safe_num_obs_;
		}

#ifdef DCS_DEBUG
		if (detected_trans_)
		{
			DCS_DEBUG_TRACE("Detected transient after " << num_obs_ << " observations.");
		}
#endif // DCS_DEBUG

		return detected_trans_;
	}


	/**
	 * \brief Tells if the transient phase is done.
	 * \return \c true if the transient phase is done; \c false otherwise.
	 *
	 * \note Transient phase might be done even if transient has not been
	 *  detected. This happens, for instance, when the number of observations
	 *  overpasses the maximum acceptable transient length.
	 */
	public: bool aborted() const
	{
		return detect_aborted_;
	}


	/**
	 * \brief Tells if initial transient period has been successfully detected.
	 * \return \c true if transient period is detected; \c false otherwise.
	 */
	public: bool detected() const
	{
		return detected_trans_;
	}


	/**
	 * \brief Return the estimated length of the initial transient period.
	 * \return The estimated length of the initial transient period.
	 *
	 * \note In order to get a meaningful value make sure to call this method
	 *  only when method \c detected() returns \c true.
	 *
	 * \deprecated Use estimated_size
	 */
	public: uint_type estimated_transient_length() const
	{
		// TODO
		// What to do when the \c estimated_transient_length method is called
		//  when transient phase is not finished yet?
		//  To throw an exception?
		//  To return a special value?
		//  ...

		return n0_;
	}


	/**
	 * \brief Return the estimated length of the initial transient period.
	 * \return The estimated length of the initial transient period.
	 *
	 * \note In order to get a meaningful value make sure to call this method
	 *  only when method \c detected() returns \c true.
	 */
	public: uint_type estimated_size() const
	{
		// TODO
		// What to do when the \c estimated_transient_length method is called
		//  when transient phase is not finished yet?
		//  To throw an exception?
		//  To return a special value?
		//  ...

		return n0_;
	}


	/// Reset the internal state and prepare the detection engine to run a
	/// new detection session.
	public: void reset()
	{
		detect_aborted_ = detected_trans_
						= false;

		num_obs_ = n0_
				 = num_buf_obs_
				 = n_t_
				 = gamma_n0_star_
				 //= delta_n_
				 = safe_num_obs_
				 = uint_type(0);

		sum_ = real_type(0);

		//obs_.resize(initial_buf_size, false);
		obs_.resize(n_v_, false);
		weights_.resize(n_v_, false);
	}


	/**
	 * \brief Return the steady-state observations possibly used during
	 *  transient phase detection.
	 * \return The steady-state observations possibly used during transient
	 *  phase detection.
	 */
	public: sample_container steady_state_observations() const
	{
		const size_type n(obs_.size());

		sample_container samples(n);

		for (size_type i = 0; i < n; ++i)
		{
			samples[i] = ::std::make_pair(obs_(i), weights_(i));
		}

		//return obs_;
		return samples;
	}


	/**
	 * \brief Apply heuristic rule R5 for estimating the initial transient
	 *  length.
	 *
	 * Rule R5 states that:
	 * <em>
	 * The initial transient period is over after \f$n_0\f$ observations if the
	 * time series \f$x_1,x_2,\ldots,x_n\f$ crosses the mean \f$\bar{X}(n_0)\f$
	 * \f$k\f$ times.
	 * </em>
	 *
	 * \todo: Maybe use a specific exception in order to signal users that
	 *  maximum phase length has been reached and to enable them to explicitly
	 *  catch it.
	 */
	private: void heuristic_phase(real_type value)
	{
		if (max_heuristic_len_ != num_obs_infinity && num_buf_obs_ > max_heuristic_len_)
		{
			DCS_DEBUG_TRACE("Failed to leave heuristic phase of Schruben test after " << num_buf_obs_ << " observations: maximum heuristic phase length reached.");
			//throw ::std::runtime_error("Maximum heuristic phase length of Schruben test reached.");
			detect_aborted_ = true;
			detected_trans_ = false;
			return;
		}

		DCS_DEBUG_TRACE("Performing heuristic phase on " << num_obs_ << " observations");

		sum_ += value;
		real_type mean = sum_/real_type(num_obs_);
		uint_type num_crossings = 0;
		//for (size_type i = 1; i < num_buf_obs_ && num_crossings < min_num_mean_crossings_; ++i)
		for (size_type i = 1; i < num_obs_ && num_crossings < min_num_mean_crossings_; ++i)
		{
			//if (
			//	(((obs_[i-1]-mean) <= eps_) && ((mean-obs_[i]) <= eps_))
			//	|| (((mean-obs_[i-1]) <= eps_) && ((obs_[i]-mean) <= eps_))
			//)
			// This is used in AKAROA 2
			if (
				(obs_[i-1] < mean && mean < obs_[i]) // increasing
				|| (obs_[i-1] > mean && mean > obs_[i]) // decreasing
				|| ((::std::abs(obs_[i-1]-mean) <= eps_) && (::std::abs(obs_[i]-mean) <= eps_)) // equality
			) {
				// cross found
				++num_crossings;
			}
		}
		if (num_crossings == min_num_mean_crossings_)
		{
			// Heuristic phase has finished

			////n0_ = n0_star_ = num_crossings_;
			n0_ = n0_star_ = num_obs_;
			//n0_ = num_obs_;
			//gamma_n0_star_ = uint_type(gamma_ * n0_star_);
			gamma_n0_star_ = uint_type(gamma_ * n0_star_);
			n_t_ = ::std::max(gamma_n0_star_, uint_type(gamma_v_ * n_v_));
			//delta_n_ = n_t_;
			// clear and resize the buffer
			obs_.resize(n_t_, false);
			weights_.resize(n_t_, false);
			//obs_.clear();
			num_buf_obs_ = 0;
			//obs_ = ::std::vector(n_t_);

			DCS_DEBUG_TRACE("Initial approximation of transient length " << n0_star_ << " (n_t: " << n_t_ << ")");
		}
	}

	private: void schruben_phase()
	{
		// STEP 2

		if (num_buf_obs_ == n_t_)
		{
			// STEP 3

			DCS_DEBUG_TRACE("Performing Schruben test on " << num_buf_obs_ << " observations");

			real_type variance;
			uint_type kappa;

			// Estimates the variance
			detail::spectral_anova(
				::boost::numeric::ublas::subrange(obs_, num_buf_obs_-n_v_, num_buf_obs_),
				n_ap_,
				delta_,
				slope_protection_,
				variance,
				kappa
			);

			// Computes Schruben statistic
			real_type schruben_stat = ::std::abs(
				detail::schruben_statistic(
					::boost::numeric::ublas::subrange(obs_, 0, n_t_),
					n_v_,
					variance
				)
			);

			// Performs the hypothesis testing
			::dcs::math::stats::students_t_distribution<real_type> t_dist(kappa);
			real_type t = t_dist.quantile(real_type(1) - alpha_t_/real_type(2));
			if (schruben_stat <= t)
			{
				// Out of transient

				DCS_DEBUG_TRACE("The initial transient period is no longer than " << n0_ << " observations.");

				detected_trans_ = true;

				// The observation vector now contains only steady-state
				// observations
				obs_.resize(num_buf_obs_, true);
				weights_.resize(num_buf_obs_, true);
			}
			else
			{
//				for (::std::size_t i = 0; i < num_buf_obs_; ++i)
//				{
//					obs_[i] = obs_[i + gamma_n0_star_];
//				}
//				obs_.resize(obs_.size() - gamma_n0_star_);

				::std::copy(
						obs_.begin() + gamma_n0_star_,
						obs_.begin() + num_buf_obs_,
						obs_.begin()
					);
				::std::copy(
						weights_.begin() + gamma_n0_star_,
						weights_.begin() + num_buf_obs_,
						weights_.begin()
					);

				num_buf_obs_ -= gamma_n0_star_;
				n0_ += gamma_n0_star_;
			}
		}
//		else
//		{
//			// Append delta_n_ observations to the tested sequence
//		}
	}


	/// Total number of observations since beginning.
	private: uint_type num_obs_;
	/// Tells if transient phase has been aborted.
	private: bool detect_aborted_;
	/// Tells if transient phase has been detected.
	private: bool detected_trans_;

	/* Used during heuristic phase: */
	/// Minimum number of mean crossings for initial estimate.
	private: /*const*/ uint_type min_num_mean_crossings_;
	/// Sum of observations for initial estimate.
	private: real_type sum_;
	/// Initial estimate of transient length.
	private: uint_type n0_star_; // useless (it seems)
	/// Estimate of transient length.
	private: uint_type n0_;
	/// Maximum allowed number of observations during the whole transient phase.
	private: /*const*/ uint_type n0_max_;
	/// Maximum allowed number of observations during the heuristic phase.
	private: /*const*/ uint_type max_heuristic_len_;

	/* Used during both phases: */
	/// Vector holding recent observations.
	private: vector_type obs_;
	/// Vector holding recent observation weights.
	private: vector_type weights_;
//	/// Size of the buffer.
//	private: size_type buf_size_;
	/// Number of observations inserted in the buffer.
	private: uint_type num_buf_obs_;

	/* Used during Schruben testing phase: */
	/// Exchange factor for calculating step size between tests.
	private: /*const*/ real_type gamma_;
	/// Safety factor for variance estimation sequential length.
	private: /*const*/ real_type gamma_v_;
	/// Significance level for stationarity test.
	private: /*const*/ real_type alpha_t_;
	/// Length of sequence used to estimate variance.
	private: /*const*/ uint_type n_v_;
	/// Number of observations to test for stationarity.
	private: uint_type n_t_;
	/// Step Length: Number of observations between stationarity tests.
	private: uint_type gamma_n0_star_;
	///// Additional number of observations to discard from the beginning of the
	/// test sequence.
	//private: uint_type delta_n_; // useless

	/* Used during safety factor phase: */
	/// Multiplier for estimated transient length.
	private: /*const*/ real_type safety_factor_;
	/// The maximum number of observations to collect before leaving the
	/// transient phase.
	private: uint_type safe_num_obs_;
	private: /*const*/ uint_type n_ap_;
	private: /*const*/ uint_type delta_;
	private: /*const*/ detail::slope_protection_category slope_protection_;
	/// Tolerance for floating-point equality test.
	private: /*const*/ real_type eps_;
}; // pawlikowski1990_transient_detector

template <typename RealT, typename UIntT>
const RealT pawlikowski1990_transient_detector<RealT,UIntT>::default_gamma = RealT(0.5);

template <typename RealT, typename UIntT>
const RealT pawlikowski1990_transient_detector<RealT,UIntT>::default_gamma_v = RealT(2);

template <typename RealT, typename UIntT>
const RealT pawlikowski1990_transient_detector<RealT,UIntT>::default_alpha_t = RealT(0.05);

template <typename RealT, typename UIntT>
const RealT pawlikowski1990_transient_detector<RealT,UIntT>::default_safety_factor = RealT(1);

template <typename RealT, typename UIntT>
const RealT pawlikowski1990_transient_detector<RealT,UIntT>::default_eps = RealT(1e-5);

template <typename RealT, typename UIntT>
const UIntT pawlikowski1990_transient_detector<RealT,UIntT>::num_obs_infinity = ::dcs::math::constants::infinity<UIntT>::value;

template <typename RealT, typename UIntT>
const UIntT pawlikowski1990_transient_detector<RealT,UIntT>::default_n0_max = pawlikowski1990_transient_detector<RealT,UIntT>::num_obs_infinity;

template <typename RealT, typename UIntT>
const UIntT pawlikowski1990_transient_detector<RealT,UIntT>::default_max_heuristic_length = pawlikowski1990_transient_detector<RealT,UIntT>::default_n0_max/2;

}}} // Namespace dcs::des::spectral


#endif // DCS_DES_SPECTRAL_PAWLIKOWSKI1990_TRANSIENT_DETECTOR_HPP
