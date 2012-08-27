/**
 * \file dcs/des/utility.hpp
 *
 * \brief Utility for Independent Replication output analysis.
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
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */

#ifndef DCS_DES_UTILITY_HPP 
#define DCS_DES_UTILITY_HPP 


#include <algorithm>
#include <cmath>
#include <dcs/iterator/any_forward_iterator.hpp>
#include <dcs/iterator/iterator_range.hpp>
#include <dcs/math/stats/distribution/normal.hpp>
#include <dcs/math/stats/distribution/students_t.hpp>
#include <limits>


namespace dcs { namespace des {

template <typename RealT, typename UIntT>
UIntT num_replications_initial(RealT, RealT, RealT);

template <typename RealT, typename UIntT>
UIntT num_replications_initial(RealT, ::dcs::iterator::iterator_range< ::dcs::iterator::any_forward_iterator<RealT> > const&, RealT level);

template <typename RealT, typename UIntT>
UIntT num_replications(RealT, UIntT, RealT, RealT, UIntT);

template <typename RealT, typename UIntT>
UIntT num_replications(RealT, UIntT, ::dcs::iterator::iterator_range< ::dcs::iterator::any_forward_iterator<RealT> > const&, RealT, UIntT);

template <typename RealT, typename UIntT>
UIntT num_replications(RealT, RealT, RealT, UIntT);

template <typename RealT, typename UIntT>
UIntT num_replications(RealT, ::dcs::iterator::iterator_range< ::dcs::iterator::any_forward_iterator<RealT> > const&, RealT, UIntT);


/**
 * \brief Compute an initial estimate for the number of replications.
 * \param eps The precision of the confidence interval for the interested
 *  output statistics.
 * \param s0 The initial estimate of the standard deviation of the interested
 *  output statistics.
 * \param level The level of the confidence interval (must be in \f$[0,1]\f$).
 * \return The initial estimate of the minimum number of replications needed
 *  to reach the desired precision \a eps.
 *
 * For computing the initial estimate \f$R\f$ of the number of replications,
 * the following formula is used:
 * \f[
 *   R \ge \left(\frac{z_{1-\alpha/2} S_0}{\epsilon}\right)^2
 * \f]
 * where:
 * - \f$\alpha\f$ is the one minus the confidence level \a level (e.g., for a
 *   \a level equals to 0.95, \f$\alpha\f$ would be 0.05).
 * - \f$z_{1-\alpha/2}\f$ is the \f$(1-\alpha/2)\f$-th quantile of the
 *   standard Normal distribution.
 * - \f$S_0\f$ is an estimate of the standard deviation of the interested
 *   output statistics.
 * - \f$\epsilon\f$ is the precision given by \a eps.
 * .
 */
template <typename RealT, typename UIntT>
UIntT num_replications_initial(RealT eps, RealT s0, RealT level=0.95)
{
	::dcs::math::stats::normal_distribution<RealT> dist;

	RealT z = dist.quantile((1+level)/RealT(2));
	RealT r0 = z*s0/eps;
	return static_cast<UIntT>(::std::ceil(r0*r0));
}


/**
 * \brief Compute an initial estimate for the number of replications.
 * \param eps The precision of the confidence interval for the interested
 *  output statistics.
 * \param s0_range A range of iterators to a sequence of initial estimates of
 *  the standard deviations of all of the interested output statistics.
 * \param level The level of the confidence interval (must be in \f$[0,1]\f$).
 * \return The maximum initial estimate of the minimum number of replications
 *  needed to reach the desired precision \a eps for all of interested output
 *  statistics.
 *
 * For computing the initial estimate \f$R\f$ of the number of replications,
 * the following formula is used:
 * \f[
 *   R \ge \left(\frac{z_{1-\alpha/2} S_0}{\epsilon}\right)^2
 * \f]
 * where:
 * - \f$\alpha\f$ is the one minus the confidence level \a level (e.g., for a
 *   \a level equals to 0.95, \f$\alpha\f$ would be 0.05).
 * - \f$z_{1-\alpha/2}\f$ is the \f$(1-\alpha/2)\f$-th quantile of the
 *   standard Normal distribution.
 * - \f$S_0\f$ is an estimate of the standard deviation of the interested
 *   output statistics.
 * - \f$\epsilon\f$ is the precision given by \a eps.
 * .
 */
template <typename RealT, typename UIntT>
UIntT num_replications_initial(RealT eps, ::dcs::iterator::iterator_range< ::dcs::iterator::any_forward_iterator<RealT> > const& s0_range, RealT level=0.95)
{
	::dcs::math::stats::normal_distribution<RealT> dist;

	UIntT r_max(0);

	for (
		::dcs::iterator::any_forward_iterator<RealT> it = s0_range.begin();
		it != s0_range.end();
		++it
	) {
//		RealT z = dist.quantile((1+level/RealT(2)));
//		RealT r0 = z**(*it)/eps;
//		r_max = ::std::max(static_cast<UIntT>(::std::ceil(r0*r0)), r_max);
		r_max = ::std::max(num_replications_initial(eps, *it, level), r_max);
	}

	return r_max;
}


/**
 * \brief Compute an estimate for the number of replications.
 * \param eps The precision of the confidence interval for the interested
 *  output statistics.
 * \param r0 The actual number of replications.
 * \param s0 The estimate of the standard deviation of the interested
 *  output statistics.
 * \param level The level of the confidence interval (must be in \f$[0,1]\f$).
 * \return The estimate of the minimum number of replications needed
 *  to reach the desired precision \a eps, if it can be reached in a number of
 *  replications less than the sum of \a r0 and \a max_trials; otherwise, the
 *  number of replications obtained at the trial \a max_trials.
 *
 * For computing the initial estimate \f$R\f$ of the number of replications,
 * the following formula is used:
 * \f[
 *   R \ge \left(\frac{t_{1-\alpha/2,R-1} S_0}{\epsilon}\right)^2
 * \f]
 * where:
 * - \f$\alpha\f$ is the one minus the confidence level \a level (e.g., for a
 *   \a level equals to 0.95, \f$\alpha\f$ would be 0.05).
 * - \f$t_{1-\alpha/2}\f$ is the \f$(1-\alpha/2)\f$-th quantile of the
 *   Student's t distribution with \f$R-1\f$ degrees of freedom.
 * - \f$S_0\f$ is an estimate of the standard deviation of the interested
 *   output statistics.
 * - \f$\epsilon\f$ is the precision given by \a eps.
 * .
 */
template <typename RealT, typename UIntT>
UIntT num_replications(RealT eps, UIntT r0, RealT s0, RealT level=0.95, UIntT max_trials=::std::numeric_limits<UIntT>::max())
{
	if (r0 < 2)
	{
		return r0;
	}

	RealT r(0);
	UIntT trial(0);

	while (r0 < r && trial < max_trials)
	{
		::dcs::math::stats::students_t_distribution<RealT> dist(r0-1);

		RealT t = dist.quantile((1+level)/RealT(2));
		r = t*s0/eps;
		r *= r;

		++trial;
		++r0;
	}

	return r0;
}


/**
 * \brief Compute an estimate for the number of replications.
 *
 * \param eps The precision of the confidence interval for the interested
 *  output statistics.
 * \param r0 The actual number of replications.
 * \param s0_range A range of iterators to a sequence of estimates of the
 *  standard deviation of all of the interested output statistics.
 * \param level The level of the confidence interval (must be in \f$[0,1]\f$).
 * \param max_trials The maximum number of trials to perform before giving-up.
 *
 * \return The maximum estimate of the minimum number of replications
 *  needed to reach the desired precision \a eps for all of the interested
 *  output statistics, if it can be reached in a number of replications less
 *  than the sum of \a r0 and \a max_trials; otherwise, the number of
 *  replications obtained at the trial \a max_trials.
 *
 * The estimate \f$R\f$ of the number of replications, is the smallest integer
 * satisfying the following formula:
 * \f[
 *   R \ge \left(\frac{t_{1-\alpha/2,R-1} S_0}{\epsilon}\right)^2
 * \f]
 * where:
 * - \f$\alpha\f$ is the one minus the confidence level \a level (e.g., for a
 *   \a level equals to 0.95, \f$\alpha\f$ would be 0.05).
 * - \f$t_{1-\alpha/2}\f$ is the \f$(1-\alpha/2)\f$-th quantile of the
 *   Student's t distribution with \f$R-1\f$ degrees of freedom.
 * - \f$S_0\f$ is an estimate of the standard deviation of the interested
 *   output statistics.
 * - \f$\epsilon\f$ is the precision given by \a eps.
 * .
 * The parameter \a max_trials is used for limiting the number of application
 * of the above formula.
 */
template <typename RealT, typename UIntT>
UIntT num_replications(RealT eps, UIntT r0, ::dcs::iterator::iterator_range< ::dcs::iterator::any_forward_iterator<RealT> > const& s0_range, RealT level=0.95, UIntT max_trials=::std::numeric_limits<UIntT>::max())
{
	if (r0 < 2)
	{
		return r0;
	}

	UIntT r_max(0);

	for (
		::dcs::iterator::any_forward_iterator<RealT> it = s0_range.begin();
		it != s0_range.end();
		++it
	) {
		RealT r(0);
		UIntT trial(0);

		do
		{
			++r0;

			::dcs::math::stats::students_t_distribution<RealT> dist(r0-1);

			RealT t = dist.quantile((1+level)/RealT(2));
			r = t*(*it)/eps;
			r *= r;

			++trial;
		}
		while (r0 < r && trial < max_trials);

		r_max = ::std::max(r0, r_max);
	}

	return r_max;
}

/**
 * \brief Compute an estimate for the number of replications.
 *
 * \param eps The precision of the confidence interval for the interested
 *  output statistics.
 * \param s0_range A range of iterators to a sequence of estimates of the
 *  standard deviation of all of the interested output statistics.
 * \param s0 The initial estimate of the standard deviation of the interested
 *  output statistics.
 * \param level The level of the confidence interval (must be in \f$[0,1]\f$).
 * \param max_trials The maximum number of trials to perform before giving-up.
 *
 * \return The estimate of the minimum number of replications needed
 *  to reach the desired precision \a eps.
 *
 * For computing the estimate \f$R\f$ of the number of replications, first an
 * initial estimate \f$R_0\f$ is computed (using Normal quantiles) , and then,
 * from \f$R_0\f$ the final estimate \f$R\f$ is computed (using Student's
 * quantiles).
 */
template <typename RealT, typename UIntT>
UIntT num_replications(RealT eps, RealT s0, RealT level=0.95, UIntT max_trials=::std::numeric_limits<UIntT>::max())
{
	UIntT r0(0);
	r0 = num_replications_initial<RealT,UIntT>(eps, s0, level);

	return num_replications(eps, r0, s0, level, max_trials);
}


/**
 * \brief Compute an estimate for the number of replications.
 *
 * \param eps The precision of the confidence interval for the interested
 *  output statistics.
 * \param s0 The initial estimates of the standard deviation of the interested
 *  output statistics.
 * \param level The level of the confidence interval (must be in \f$[0,1]\f$).
 * \param max_trials The maximum number of trials to perform before giving-up.
 *
 * \return The maximum estimate of the minimum number of replications
 *  needed to reach the desired precision \a eps for all of the interested
 *  output statistics.
 *
 * For computing the estimate \f$R\f$ of the number of replications, first an
 * initial estimate \f$R_0\f$ is computed (using Normal quantiles) , and then,
 * from \f$R_0\f$ the final estimate \f$R\f$ is computed (using Student's
 * quantiles).
 */
template <typename RealT, typename UIntT>
UIntT num_replications(RealT eps, ::dcs::iterator::iterator_range< ::dcs::iterator::any_forward_iterator<RealT> > const& s0_range, RealT level=0.95, UIntT max_trials=::std::numeric_limits<UIntT>::max())
{
	UIntT r_max(0);

	for (
		::dcs::iterator::any_forward_iterator<RealT> it = s0_range.begin();
		it != s0_range.end();
		++it
	) {
		UIntT r0 = num_replications_initial<RealT,UIntT>(eps, *it, level);

		r_max = ::std::max(num_replications(eps, r0, *it, level, max_trials), r_max);
	}

	return r_max;
}


}} // Namespace dcs::des


#endif // DCS_DES_UTILITY_HPP 
