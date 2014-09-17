/**
 * \file dcs/des/max_estimator.hpp
 *
 * \brief Maximum estimator for independent and identically distributed samples.
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

#ifndef DCS_DES_MAX_ESTIMATOR_HPP
#define DCS_DES_MAX_ESTIMATOR_HPP


#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <dcs/debug.hpp>
#include <dcs/des/base_statistic.hpp>
#include <dcs/des/statistic_categories.hpp>
#include <dcs/math/constants.hpp>
#include <dcs/math/stats/distribution/students_t.hpp>
#include <dcs/math/stats/function/quantile.hpp>
#include <string>


namespace dcs { namespace des {

/**
 * \brief Maximum estimator for independent and identically distributed samples.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename ValueT>
class max_estimator: public base_statistic<ValueT>
{
	private: typedef base_statistic<ValueT> base_type;
	public: typedef ValueT value_type;
	public: typedef max_statistic_category category_type;


	public: explicit max_estimator(value_type ci_level = base_type::default_confidence_level, std::string const& name = "Max")
	: base_type(ci_level, name),
	  count_(0),
	  m_(-::dcs::math::constants::infinity<value_type>::value)
	{
		// Empty
	}


	private: statistic_category do_category() const
	{
		return max_statistic;
	}


	private: void do_collect(value_type obs, value_type /*ignored_weight*/)
	{
		++count_;

		if (m_ < obs)
		{
			m_ = obs;
		}
	}


	private: value_type do_estimate() const
	{
		return m_;
	}


	private: value_type do_half_width() const
	{
		return 0;
	}


	private: std::size_t do_num_observations() const
	{
		return count_;
	}


	private: value_type do_relative_precision() const
	{
		return 0;
	}


	private: void do_reset()
	{
		m_ = -::dcs::math::constants::infinity<value_type>::value;
		count_ = std::size_t(0);
	}


	private: value_type do_variance() const
	{
		return 0;
	}


	private: std::size_t count_; ///< Number of observations seen to date.
	private: value_type m_; ///< Accumulator for the minimum value.
};


}} // Namespace dcs::des


#endif // DCS_DES_MAX_ESTIMATOR_HPP
