/**
 * \file dcs/des/base_statistic.hpp
 *
 * \brief Base class for output statistics.
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

#ifndef DCS_DES_BASE_STATISTIC_HPP
#define DCS_DES_BASE_STATISTIC_HPP


#include <cmath>
#include <dcs/des/statistic_categories.hpp>
#include <dcs/macro.hpp>
#include <iosfwd>
#include <string>


namespace dcs { namespace des {

/**
 * \brief Base class for output statistics.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename ValueT, typename UIntT>
class base_statistic
{
	public: typedef ValueT value_type;
	public: typedef UIntT uint_type;


	public: static const value_type default_confidence_level; //= 0.95;


	/// Default constructor
	public: base_statistic()
	: enabled_(true)
	{
	}


	// Compiler-generated copy-constructor/assignment are fine.


	/// Destructor
	public: virtual ~base_statistic() { }


	public: void operator()(value_type obs, value_type weight = value_type(1))
	{
		do_collect(obs, weight);
	}


	public: statistic_category category() const
	{
		return do_category();
	}


	public: void reset()
	{
		do_reset();
	}


	public: uint_type num_observations() const
	{
		return do_num_observations();
	}


	public: value_type estimate() const
	{
		return do_estimate();
	}


	public: value_type variance() const
	{
		return do_variance();
	}


	public: value_type standard_deviation() const
	{
		return do_standard_deviation();
	}


	public: value_type half_width() const
	{
		return do_half_width();
	}


	public: value_type relative_precision() const
	{
		return do_relative_precision();
	}


	public: value_type confidence_level() const
	{
		return do_confidence_level();
	}


	public: ::std::string name() const
	{
		return do_name();
	}


	public: void enable(bool value)
	{
		do_enable(value);

		enabled_ = value;
	}


	/**
	 * \brief Tells if output analysis on this statistic has been enaabled.
	 * \return \c true if output analysis has been enabled; \c false otherwise.
	 *
	 * Output analysis on a statistic may be disabled for several reasons:
	 * - The maximum number of observations has been reached.
	 * - An error is occurred.
	 * .
	 */
	public: bool enabled() const
	{
		return enabled_;
	}


	///@{ Interface methods

	private: virtual statistic_category do_category() const = 0;


	private: virtual void do_collect(value_type obs, value_type weight) = 0;


	private: virtual void do_reset() = 0;


	private: virtual uint_type do_num_observations() const = 0;


	private: virtual value_type do_estimate() const = 0;


	private: virtual value_type do_variance() const = 0;


	private: virtual value_type do_standard_deviation() const
	{
		return ::std::sqrt(variance());
	}


	private: virtual value_type do_half_width() const = 0;


	private: virtual value_type do_relative_precision() const = 0;


	private: virtual value_type do_confidence_level() const = 0;


	private: virtual ::std::string do_name() const = 0;


	/**
	 * \brief Tells if output analysis on this statistic has been enabled.
	 * \return \c true if output analysis has been enabled; \c false otherwise.
	 *
	 * Output analysis on a statistic may be disabled for several reasons:
	 * - The maximum number of observations has been reached.
	 * - An error is occurred.
	 * .
	 */
	protected: virtual void do_enable(bool value)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(value);

		// empty
	}


	protected: virtual ::std::ostream& do_print(::std::ostream& os) const
	{
		return os << estimate()
				  << " +/- " << standard_deviation()
				  << " -- C.I. ("
				  << (estimate()-half_width())
				  << ", " << (estimate()+half_width())
				  << ") at " << (confidence_level()*100) << "%"
				  << " (r.e. " << (relative_precision()*100) << "% - sample size: " << this->num_observations() << ")";
	}

	///@} Interface methods


	public: template <
				typename CharT,
				typename CharTraitsT,
				typename ValueT2,
				typename UIntT2
		>
		friend ::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, base_statistic<ValueT2,UIntT2> const& stat)
	{
		return stat.do_print(os);
	}


	private: bool enabled_;
}; // base_statistic


template <typename ValueT, typename UIntT>
const ValueT base_statistic<ValueT,UIntT>::default_confidence_level = ValueT(0.95);


}} // Namespace dcs::des


#endif // DCS_DES_BASE_STATISTIC_HPP
