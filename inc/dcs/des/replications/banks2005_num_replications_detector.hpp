/**
 * \file dcs/des/replications/banks2005_num_replications_detector.hpp
 *
 * \brief Number of replications detector based on the confidence interval
 *  method proposed by (Banks, 2005).
 *
 * References:
 * - J. Banks, J.S. Carson II, B.L. Nelson, and D.M. Nicol.
 *   "Discrete-Event System Simulation, 4th edition"
 *   Prentice Hall, 2005
 * .
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

#ifndef DCS_DES_REPLICATIONS_BANKS2005_NUM_REPLICATIONS_DETECTOR_HPP
#define DCS_DES_REPLICATIONS_BANKS2005_NUM_REPLICATIONS_DETECTOR_HPP


#include <cmath>
#include <dcs/macro.hpp>
#include <dcs/math/constants.hpp>
#include <dcs/math/stats/distribution/normal.hpp>
#include <dcs/math/stats/distribution/students_t.hpp>
#include <dcs/math/stats/function/quantile.hpp>
#include <dcs/math/function/sqr.hpp>
#include <iostream>
//#include <vector>
#include <stdexcept>


namespace dcs { namespace des { namespace replications {

template <
	typename RealT,
	typename UIntT
>
class banks2005_num_replications_detector
{
	public: typedef RealT real_type;
	public: typedef UIntT uint_type;
//	public: typedef ::std::vector<real_type> vector_type;


	public: static const real_type default_ci_level; // = 0.95
	public: static const real_type default_relative_precision; // = 0.04
	public: static const uint_type default_min_num_replications = 2;
	public: static const uint_type default_max_num_replications; // = ::dcs::math::constants::infinity<uint_type>::value;


	public: explicit banks2005_num_replications_detector(real_type ci_level=default_ci_level, real_type rel_prec=default_relative_precision, uint_type min_num_repl=default_min_num_replications, uint_type max_num_repl=default_max_num_replications)
		: ci_level_(ci_level),
		  rel_prec_(rel_prec),
		  r_min_(min_num_repl),
		  r_max_(max_num_repl),
		  r_(0),
		  detected_(false),
		  aborted_(false),
		  first_call_(true)
	{
		// pre: min # replications >= 2
		DCS_ASSERT(
			r_min_ >= 2,
			throw ::std::invalid_argument("[dcs::des::replications::banks2005_num_replications_detector] Min number of replications must be >= 2.")
		);
		// pre: min # replications <= max # replications
		DCS_ASSERT(
			r_min_ <= r_max_,
			throw ::std::invalid_argument("[dcs::des::replications::banks2005_num_replications_detector] Min number of replications must be <=  max number of replications.")
		);
	}


	public: bool detect(uint_type r_cur, real_type estimate, real_type stddev)
	{
		if (r_cur < r_min_)
		{
			detected_ = false;
			return false;
		}
		if (r_cur >= r_max_)
		{
			aborted_ = true;
			return false;
		}

		if (stddev < 0 || ::std::isinf(stddev))
		{
			::std::clog << "[Warning] Standard deviation is negative or infinite." << ::std::endl;
			detected_ = false;
			return false;
		}

		real_type half_alpha((1-ci_level_)/real_type(2));

		//real_type c = rel_prec_/(1+rel_prec_);

		// Compute an initial estimate of R
		if (first_call_)
		{
			first_call_ = false;

			::dcs::math::stats::normal_distribution<real_type> norm;
			real_type z = ::dcs::math::stats::quantile(norm, half_alpha);
			r_ =  static_cast<uint_type>(::dcs::math::sqr(z*stddev/(rel_prec_*estimate)));
			//r_ =  static_cast<uint_type>(::dcs::math::sqr(z*stddev/(c*estimate)));

			if (r_ < r_min_)
			{
				r_ = r_min_;
			}
		}

		real_type r_want(0);

		// Compute the real estimate of R
		do
		{
			::dcs::math::stats::students_t_distribution<real_type> student_t(r_-1);
			real_type t = ::dcs::math::stats::quantile(student_t, half_alpha);
			r_want = ::dcs::math::sqr(t*stddev/(rel_prec_*estimate));
			//r_want = ::dcs::math::sqr(t*stddev/(c*estimate));

			if (r_ < r_want)
			{
				++r_;
			}
		}
		while (r_ < r_want && r_ < r_max_);

		detected_ = (r_ < r_max_);
//		aborted_ = (r_ >= r_max_);

//		means_.push_back(estimator);
::std::cerr << "(" << this << ") Detecting # Replications --> " << ::std::boolalpha << detected_ << " (r_: " << r_ << " - r_want: " << r_want << " - r_max_: " << r_max_ << ")" << ::std::endl;//XXX

		return detected_;
	}


	public: bool detected() const
	{
		return detected_;
	}


	public: bool aborted() const
	{
		return aborted_;
	}


	public: uint_type estimated_number() const
	{
		return r_;
	}


//	public: vector_type computed_estimators() const
//	{
//		return estimators_;
//	}


	public: void reset()
	{
//		estimators_.clear();
		aborted_ = detected_
				 = false;
		first_call_ = true;
		r_ = 0;
	}


	/// The confidence level
	private: real_type ci_level_;
	/// The target relative precision.
	private: real_type rel_prec_;
	/// The minimum number of replications
	private: uint_type r_min_;
	/// The maximum number of replications
	private: uint_type r_max_;
	/// The actual number of replications
	private: uint_type r_;
	private: bool detected_;
	private: bool aborted_;
	private: bool first_call_;
//	private: vector_type estimators_;
};

template <typename RealT, typename UIntT>
const UIntT banks2005_num_replications_detector<RealT,UIntT>::default_max_num_replications = ::dcs::math::constants::infinity<UIntT>::value;

template <typename RealT, typename UIntT>
const RealT banks2005_num_replications_detector<RealT,UIntT>::default_ci_level = 0.95;

template <typename RealT, typename UIntT>
const RealT banks2005_num_replications_detector<RealT,UIntT>::default_relative_precision = 0.04;

}}} // Namespace dcs::des::replications


#endif // DCS_DES_REPLICATIONS_BANKS2005_NUM_REPLICATIONS_DETECTOR_HPP
