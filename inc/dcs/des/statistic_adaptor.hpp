/**
 * \file dcs/des/statistic_adaptor.hpp
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

#ifndef DCS_DES_STATISTIC_ADAPTOR_HPP
#define DCS_DES_STATISTIC_ADAPTOR_HPP


#include <dcs/des/base_statistic.hpp>
#include <dcs/des/statistic_categories.hpp>
#include <dcs/type_traits/add_const.hpp>
#include <dcs/type_traits/add_reference.hpp>
#include <dcs/type_traits/remove_reference.hpp>
#include <string>


namespace dcs { namespace des {

template <
	typename StatisticT,
	typename StatisticTraitsT=typename ::dcs::type_traits::remove_reference<StatisticT>::type
>
class statistic_adaptor: public base_statistic<
							typename StatisticTraitsT::value_type,
							typename StatisticTraitsT::uint_type
						>
{
	public: typedef StatisticT adaptee_type;
	public: typedef typename StatisticTraitsT::value_type value_type;
	public: typedef typename StatisticTraitsT::uint_type uint_type;
	public: typedef typename ::dcs::type_traits::add_reference<StatisticT>::type adaptee_reference;
	public: typedef typename ::dcs::type_traits::add_reference<
									typename ::dcs::type_traits::add_const<StatisticT>::type
								>::type adaptee_const_reference;


	public: statistic_adaptor(adaptee_const_reference stat)
		: adaptee_(stat)
	{
		// Empty
	}


	private: ::std::string do_name() const
	{
		return adaptee_.name();
	}


	private: statistic_category do_category() const
	{
		return adaptee_.category();
	}


    private: void do_collect(value_type obs, value_type weight)
	{
		adaptee_(obs, weight);
	}


    private: void do_reset()
	{
		adaptee_.reset();
	}


    private: uint_type do_num_observations() const
	{
		return adaptee_.num_observations();
	}


    private: value_type do_estimate() const
	{
		return adaptee_.estimate();
	}


    private: value_type do_variance() const
	{
		return adaptee_.variance();
	}


    private: value_type do_standard_deviation() const
	{
		return adaptee_.standard_deviation();
	}


    private: value_type do_half_width() const
	{
		return adaptee_.half_width();
	}


    private: value_type do_relative_precision() const
	{
		return adaptee_.relative_precision();
	}


    private: value_type do_confidence_level() const
	{
		return adaptee_.confidence_level();
	}


	private: ::std::ostream& do_print(::std::ostream& os) const
	{
		return os << adaptee_;
	}


	private: adaptee_type adaptee_;
};

}} // Namespace dcs::des


#endif // DCS_DES_STATISTIC_ADAPTOR_HPP
