/* vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4: */

/**
 * \file dcs/des/quantile_estimator.hpp
 *
 * \brief Quantile estimator for independent and identically distributed
 *  samples.
 *
 * Copyright (C) 2009       Distributed Computing System (DCS) Group,
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

#ifndef DCS_DES_QUANTILE_ESTIMATOR_HPP
#define DCS_DES_QUANTILE_ESTIMATOR_HPP


#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/p_square_quantile.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <cstddef>
#include <dcs/des/base_statistic.hpp>
#include <dcs/des/statistic_categories.hpp>
#include <dcs/math/constants.hpp>
#include <dcs/math/stats/distribution/students_t.hpp>
#include <sstream>
#include <string>


namespace dcs { namespace des {

namespace quantest_detail {

template <typename RealT>
std::string make_name(RealT p)
{
	::std::ostringstream oss;
	oss << p << "th Quantile";
	return oss.str();
}

} // Namespace quantest_detail

/**
 * \brief Quantile estimator for independent and identically distributed
 *  samples.
 *
 * Based on:
 *  R. Jain and I. Chlamtac,
 *  "The P^2 algorithm for dynamic calculation of quantiles and histograms
 *   without storing observations",
 *  Communications of the ACM, Volume 28:(10):1076-1085, 1985.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename ValueT>
class quantile_estimator: public base_statistic<ValueT>
{
	private: typedef base_statistic<ValueT> base_type;
	public: typedef ValueT value_type;
	public: typedef quantile_statistic_category category_type;
	private: typedef ::boost::accumulators::accumulator_set<
				value_type,
				::boost::accumulators::stats<
					::boost::accumulators::tag::p_square_quantile
				>
			> accumulator_type;


	public: explicit quantile_estimator(value_type p=0.5, value_type ci_level = base_type::default_confidence_level, std::string const& name = "Quantile")
	//: base_type(ci_level, quantest_detail::make_name(p)),
	: base_type(ci_level, name),
	  acc_(::boost::accumulators::quantile_probability = p),
	  p_(p)
	{
		// Empty
	}


	public: value_type probability() const
	{
		return p_;
	}


	private: statistic_category do_category() const
	{
		return quantile_statistic;
	}


	private: void do_collect(value_type obs, value_type /*ignored_weight*/)
	{
		acc_(obs);
	}


	private: void do_reset()
	{
//		acc_.drop< ::boost::accumulators::p_square_quantile >(); // DON'T WORK
		acc_ = accumulator_type(::boost::accumulators::quantile_probability = p_);
	}


	private: value_type do_estimate() const
	{
		return ::boost::accumulators::p_square_quantile(acc_);
	}


	private: std::size_t do_num_observations() const
	{
		return ::boost::accumulators::count(acc_);
	}


	private: value_type do_variance() const
	{
		return 0;
	}


	private: value_type do_half_width() const
	{
        // Compute the half-width of a two-tailed confidence interval
        // Given a 100(1-\alpha)% confidence interval, the half-width is
        // computed as:
        //   h.w. = t_{1-\alpha/2,n-1}*S/\sqrt{n}
        // where t_{1-\alpha/2,n-1} is the (1-\alpha/2) quantile of a Student's
        // t distribution with (n-1) degrees of freedom

		if (this->num_observations() > 1)
		{
            // NOTE: ci_level_ == 1-\alpha
            //    -> t_{1-\alpha/2,n-1} == t_{1-(1-ci_level_)/2,n-1}
            //                          == t_{(1+ci_level_)/2,n-1}

			std::size_t n(this->num_observations());
			::dcs::math::stats::students_t_distribution<value_type> t_dist(n-1);
			value_type t(::dcs::math::stats::quantile(t_dist, (1+this->confidence_level())/value_type(2)));
			value_type q(this->estimate());

			return t*::std::sqrt(q*(1-q)/(this->num_observations()-1));
		}
		return ::dcs::math::constants::infinity<value_type>::value;
	}


	private: value_type do_relative_precision() const
	{
		return 0;//TODO
	}



	private: accumulator_type acc_;
	private: value_type p_;
};

}} // Namespace dcs::des


#endif // DCS_DES_QUANTILE_ESTIMATOR_HPP
