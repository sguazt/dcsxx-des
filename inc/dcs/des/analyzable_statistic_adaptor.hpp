/**
 * \file dcs/des/analyzable_statistic_adaptor.hpp
 *
 * \brief Adapt a given class to the base_statistic interface.
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

#ifndef DCS_DES_ANALYZABLE_STATISTIC_ADAPTOR_HPP
#define DCS_DES_ANALYZABLE_STATISTIC_ADAPTOR_HPP


#include <cstddef>
#include <dcs/des/base_analyzable_statistic.hpp>
#include <dcs/type_traits/add_const.hpp>
#include <dcs/type_traits/add_reference.hpp>
#include <dcs/type_traits/remove_reference.hpp>
#include <iosfwd>


namespace dcs { namespace des {

template <
	typename StatisticT,
	typename StatisticTraitsT=typename ::dcs::type_traits::remove_reference<StatisticT>::type
>
class analyzable_statistic_adaptor: public base_analyzable_statistic<typename StatisticTraitsT::value_type>
{
	public: typedef StatisticT adaptee_type;
	public: typedef typename StatisticTraitsT::value_type value_type;
	public: typedef typename ::dcs::type_traits::add_reference<StatisticT>::type adaptee_reference;
	public: typedef typename ::dcs::type_traits::add_reference<
									typename ::dcs::type_traits::add_const<StatisticT>::type
								>::type adaptee_const_reference;


	public: analyzable_statistic_adaptor(adaptee_const_reference stat)
		: adaptee_(stat)
	{
		// Empty
	}


    private: virtual void do_collect(value_type obs, value_type weight)
	{
		adaptee_(obs, weight);
	}


    private: virtual void do_reset()
	{
		adaptee_.reset();
	}


    public: virtual std::size_t do_num_observations() const
	{
		return adaptee_.num_observations();
	}


    private: virtual value_type do_estimate() const
	{
		return adaptee_.estimate();
	}


    private: virtual value_type do_variance() const
	{
		return adaptee_.variance();
	}


    private: virtual value_type do_standard_deviation() const
	{
		return adaptee_.standard_deviation();
	}


    private: virtual value_type do_half_width() const
	{
		return adaptee_.half_width();
	}


    private: virtual value_type do_relative_precision() const
	{
		return adaptee_.relative_precision();
	}


    private: virtual value_type do_confidence_level() const
	{
		return adaptee_.confidence_level();
	}


	private: virtual value_type do_target_relative_precision() const
	{
		return adaptee_.target_relative_precision();
	}


	private: virtual bool do_target_precision_reached() const
	{
		return adaptee_.target_precision_reached();
	}


	private: virtual bool do_disabled() const
	{
		return adaptee_.disabled();
	}

	private: virtual std::size_t do_max_num_observations() const
	{
		return adaptee_.max_num_observations();
	}


	private: virtual bool do_steady_state_entered() const
	{
		return adaptee_.steady_state_entered();
	}


	private: virtual std::size_t do_transient_phase_length() const
	{
		return adaptee_.transient_phase_length();
	}


	private: virtual value_type do_steady_state_enter_time() const
	{
		return adaptee_.steady_state_enter_time();
	}


	private: virtual void do_steady_state_enter_time(value_type value)
	{
		adaptee_.steady_state_enter_time(value);
	}


	private: adaptee_type adaptee_;
};

}} // Namespace dcs::des


#endif // DCS_DES_ANALYZABLE_STATISTIC_ADAPTOR_HPP
