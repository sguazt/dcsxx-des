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


#include <dcs/des/base_statistic.hpp>
#include <dcs/math/constants.hpp>
#include <iostream>


namespace dcs { namespace des {

/**
 * \brief Base class for analyzable output statistics.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename ValueT, typename UIntT>
class base_analyzable_statistic: public base_statistic<ValueT,UIntT>
{
	public: typedef ValueT value_type;
	public: typedef UIntT uint_type;
	private: typedef base_statistic<value_type,uint_type> base_type;


	/// Infinite number of observations.
	public: static const uint_type num_observations_infinity;// = ::dcs::math::constants::infinity<uint_type>::value;


//	/// Default constructor
//	public: base_analyzable_statistic()
//	: enabled_(true)
//	{
//	}


	// Compiler-generated copy-constructor/assignment are fine.


	/**
	 * \brief Returns the wanted relative precision.
	 * \return The wanted relative precision.
	 */
	public: value_type target_relative_precision() const
	{
		return do_target_relative_precision();
	}


	/**
	 * \brief Tells if the target precision has been reached.
	 * \return \c true if the target precision has been reached; \c false
	 *  otherwise.
	 */
	public: bool target_precision_reached() const
	{
		return do_target_precision_reached();
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
	public: uint_type max_num_observations() const
	{
		return do_max_num_observations();
	}


	/// Returns \c true if the statistic has entered its steady state.
	public: bool steady_state_entered() const
	{
		return do_steady_state_entered();
	}


	/// Returns the length (in number of observations) of transient phase.
	public: uint_type transient_phase_length() const
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


	/// The destructor.
	public: virtual ~base_analyzable_statistic() { }


	protected: virtual void do_initialize_for_experiment()
	{
	}

	protected: virtual void do_finalize_for_experiment()
	{
	}

	/**
	 * \brief Returns the wanted relative precision.
	 * \return The wanted relative precision.
	 */
	private: virtual value_type do_target_relative_precision() const = 0;


	/**
	 * \brief Tells if the target precision has been reached.
	 * \return \c true if the target precision has been reached; \c false
	 *  otherwise.
	 */
	private: virtual bool do_target_precision_reached() const = 0;


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
	private: virtual uint_type do_max_num_observations() const = 0;


	/// Returns \c true if the statistic has entered its steady state.
	private: virtual bool do_steady_state_entered() const = 0;


	/// Returns the length (in number of observations) of transient phase.
	private: virtual uint_type do_transient_phase_length() const = 0;


	private: virtual value_type do_steady_state_enter_time() const = 0;


	private: virtual void do_steady_state_enter_time(value_type value) = 0;


	private: virtual bool do_observation_complete() const = 0;


	private: virtual ::std::ostream& do_print(::std::ostream& os) const
	{
		return os << this->estimate()
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


	public: template <
					typename CharT,
					typename CharTraitsT,
					typename ValueT2,
					typename UIntT2
		>
		friend ::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, base_analyzable_statistic<ValueT2,UIntT2> const& stat)
	{
		return stat.do_print(os);
	}


//	private: bool enabled_;
}; // base_analyzable_statistic


template <typename ValueT, typename UIntT>
const UIntT base_analyzable_statistic<ValueT,UIntT>::num_observations_infinity = ::dcs::math::constants::infinity<UIntT>::value;

}} // Namespace dcs::des


#endif // DCS_DES_BASE_ANALYZABLE_STATISTIC_HPP
