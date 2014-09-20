/**
 * \file dcs/des/base_analyzable_statistic.hpp
 *
 * \brief Base class for analyzable output statistics.
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

#ifndef DCS_DES_BASE_ANALYZABLE_STATISTIC_HPP
#define DCS_DES_BASE_ANALYZABLE_STATISTIC_HPP


#include <cstddef>
#include <dcs/assert.hpp>
#include <dcs/des/base_statistic.hpp>
#include <dcs/exception.hpp>
#include <dcs/math/constants.hpp>
#include <dcs/math/traits/float.hpp>
#include <iostream>
#include <stdexcept>


namespace dcs { namespace des {

/**
 * \brief Base class for analyzable output statistics.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename ValueT>
class base_analyzable_statistic: public base_statistic<ValueT>
{
	template <typename CT, typename CTT, typename VT>
	friend ::std::basic_ostream<CT,CTT>& operator<<(::std::basic_ostream<CT,CTT>& os, base_analyzable_statistic<VT> const& stat);

	public: typedef ValueT value_type;
	private: typedef base_statistic<value_type> base_type;


	public: static const std::size_t num_observations_infinity; ///< Constant to represent an infinite number of observations
	public: static const value_type default_target_relative_precision; ///< Default target relative precision


//	/// Default constructor
//	public: base_analyzable_statistic()
//	: enabled_(true)
//	{
//	}

	protected: explicit base_analyzable_statistic(value_type relative_precision = default_target_relative_precision)
	: target_rel_prec_(relative_precision)
	{
	}

	// Compiler-generated copy-constructor/assignment are fine.

	/// The destructor.
	public: virtual ~base_analyzable_statistic() { }

	/// Returns the wanted relative precision.
	public: value_type target_relative_precision() const
	{
		return target_rel_prec_;
	}

	/// Returns the wanted relative precision.
	public: void target_relative_precision(value_type v)
	{
		// pre: relative precision > 0
		DCS_ASSERT(v > 0,
				   DCS_EXCEPTION_THROW(std::invalid_argument, "Relative precision must be a positive number"));

		target_rel_prec_ = v;
	}

	/// Tells if the target precision has been reached.
	public: bool target_precision_reached() const
	{
		if (std::isinf(this->target_relative_precision())
			||
			dcs::math::float_traits<value_type>::definitely_less_equal(this->relative_precision(), this->target_relative_precision()))
		{
			return true;
		}

		return false;
	}

//	/**
//	 * \brief Tells if output analysis on this statistic has been enaabled.
//	 * \return \c true if output analysis has been enabled; \c false otherwise.
//	 *
//	 * Output analysis on a statistic may be disabled for several reasons:
//	 * - The maximum number of observations has been reached.
//	 * - An error is occurred.
//	 * .
//	 */
//	public: bool enabled() const
//	{
//		return enabled_;
//	}

	/**
	 * \brief Returns the maximum number of observations that can be analyzed.
	 * \return The maximum number of analyzable observations.
	 */
	public: std::size_t max_num_observations() const
	{
		return do_max_num_observations();
	}

	/// Returns \c true if the statistic has entered its steady state.
	public: bool steady_state_entered() const
	{
		return do_steady_state_entered();
	}

	/// Returns the length (in number of observations) of transient phase.
	public: std::size_t transient_phase_length() const
	{
		return do_transient_phase_length();
	}

	public: value_type steady_state_enter_time() const
	{
		return steady_state_enter_time();
	}

	public: void steady_state_enter_time(value_type value)
	{
		do_steady_state_enter_time(value);
	}

	public: bool observation_complete() const
	{
		return do_observation_complete();
	}

	public: void initialize_for_experiment()
	{
		do_initialize_for_experiment();
	}

	public: void finalize_for_experiment()
	{
		do_finalize_for_experiment();
	}

	public: void refresh()
	{
		do_refresh();
	}

	protected: virtual void do_initialize_for_experiment()
	{
	}

	protected: virtual void do_finalize_for_experiment()
	{
	}

//	/**
//	 * \brief Tells if output analysis on this statistic has been enabled.
//	 * \return \c true if output analysis has been enabled; \c false otherwise.
//	 *
//	 * Output analysis on a statistic may be disabled for several reasons:
//	 * - The maximum number of observations has been reached.
//	 * - An error is occurred.
//	 * .
//	 */
//	private: virtual void do_enable(bool value) = 0;

	/**
	 * \brief Returns the maximum number of observations that can be analyzed.
	 * \return The maximum number of analyzable observations.
	 */
	private: virtual std::size_t do_max_num_observations() const = 0;


	/// Returns \c true if the statistic has entered its steady state.
	private: virtual bool do_steady_state_entered() const = 0;


	/// Returns the length (in number of observations) of transient phase.
	private: virtual std::size_t do_transient_phase_length() const = 0;


	private: virtual value_type do_steady_state_enter_time() const = 0;


	private: virtual void do_steady_state_enter_time(value_type value) = 0;


	private: virtual bool do_observation_complete() const = 0;


	private: virtual ::std::ostream& do_print(::std::ostream& os) const
	{
		return os << this->name()
				  << ": " << this->estimate()
				  << " +/- " << this->standard_deviation()
				  << " -- C.I. ("
				  << (this->estimate()-this->half_width())
				  << ", " << (this->estimate()+this->half_width())
				  << ") at " << (this->confidence_level()*100) << "%"
				  << " (r.e.: " << (this->relative_precision()*100) << "% - sample size: " << this->num_observations() << ")"
				  << (this->observation_complete() ? "" : " ([[INCOMPLETE]])")
				  << (this->enabled() ? "" : " ([[DISABLED]])");
	}


	protected: virtual void do_refresh() {}


	protected: void do_enable(bool value)
	{
		base_type::do_enable(value);

		refresh();
	}

//	private: bool enabled_;
	private: value_type target_rel_prec_; ///< The relative precision to be reached
}; // base_analyzable_statistic


template <typename ValueT>
const std::size_t base_analyzable_statistic<ValueT>::num_observations_infinity = ::dcs::math::constants::infinity<std::size_t>::value;

template <typename ValueT>
const ValueT base_analyzable_statistic<ValueT>::default_target_relative_precision = ::dcs::math::constants::infinity<ValueT>::value;

template <typename CharT, typename CharTraitsT, typename ValueT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, base_analyzable_statistic<ValueT> const& stat)
{
	return stat.do_print(os);
}


}} // Namespace dcs::des


#endif // DCS_DES_BASE_ANALYZABLE_STATISTIC_HPP
