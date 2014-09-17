/**
 * \file dcs/des/any_analyzable_statistic.hpp
 *
 * \brief Type-erasure version of analyzable statistics.
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

#ifndef DCS_DES_ANY_ANALYZABLE_STATISTIC_HPP
#define DCS_DES_ANY_ANALYZABLE_STATISTIC_HPP


#include <boost/smart_ptr.hpp>
#include <dcs/des/analyzable_statistic_adaptor.hpp>
#include <dcs/des/base_analyzable_statistic.hpp>
#include <dcs/type_traits/add_const.hpp>
#include <dcs/type_traits/add_reference.hpp>
#include <dcs/type_traits/remove_reference.hpp>
#include <dcs/util/holder.hpp>


namespace dcs { namespace des {

/**
 * \brief Type-erasure version of analyzable statistics.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename ValueT>
class any_analyzable_statistic
{
	public: typedef ValueT value_type;


	public: any_analyzable_statistic() { }


	public: template <typename StatisticT>
		any_analyzable_statistic(StatisticT const& stat)
		: ptr_stat_(new analyzable_statistic_adaptor<StatisticT>(stat))
	{
		// Empty
	}


	public: template <typename StatisticT>
		any_analyzable_statistic(::dcs::util::holder<StatisticT> const& wrap_stat)
		: ptr_stat_(new analyzable_statistic_adaptor<StatisticT>(wrap_stat.get()))
	{
		// Empty
	}


//	public: template <typename StatisticT>
//		any_analyzable_statistic(::boost::shared_ptr<StatisticT> const& ptr_stat)
//		: base_type(ptr_stat)
//	{
//		// Empty
//	}


	// Compiler-generated copy ctor and copy assignement are fine.
 

	public: template <typename StatisticT>
		void statistic(StatisticT stat)
	{
		ptr_stat_ = new analyzable_statistic_adaptor<StatisticT>(stat);
	}


	///@{ AnalyzableStatistic concept implementation


    public: void operator()(value_type obs, value_type weight)
    {
        ptr_stat_->operator()(obs, weight);
    }


    public: void reset()
    {
        ptr_stat_->reset();
    }


    public: std::size_t num_observations() const
    {
        return ptr_stat_->num_observations();
    }


    public: value_type estimate() const
    {
        return ptr_stat_->estimate();
    }


    public: value_type variance() const
    {
        return ptr_stat_->variance();
    }


    public: value_type standard_deviation() const
    {
        return ptr_stat_->standard_deviation();
    }


    public: value_type half_width() const
    {
        return ptr_stat_->half_width();
    }


    public: value_type relative_precision() const
    {
        return ptr_stat_->relative_precision();
    }


    public: value_type confidence_level() const
    {
        return ptr_stat_->confidence_level();
    }


    public: template <
                typename CharT,
                typename CharTraitsT
            >
        ::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os)
    {
        return ptr_stat_->print();
    }


	public: value_type target_relative_precision() const
	{
		return this->statistic().target_relative_precision();
	}


	public: bool target_precision_reached() const
	{
		return this->statistic().target_precision_reached();
	}


	public: bool disabled() const
	{
		return this->statistic().disabled();
	}


	public: std::size_t max_num_observations() const
	{
		return this->statistic().max_num_observations();
	}


	public: bool steady_state_entered() const
	{
		return this->statistic().steady_state_entered();
	}


	public: std::size_t transient_phase_length() const
	{
		return this->statistic().transient_phase_length();
	}


	public: value_type steady_state_enter_time() const
	{
		return this->statistic().steady_state_enter_time();
	}


	public: void steady_state_enter_time(value_type value)
	{
		this->statistic().steady_state_enter_time(value);
	}

	///@} AnalyzbleStatistic concept implementation


	private: ::boost::shared_ptr< base_analyzable_statistic<value_type,std::size_t> > ptr_stat_;
};


template <
	typename StatisticT,
	typename StatisticTraitsT=typename ::dcs::type_traits::remove_reference<StatisticT>::type
>
struct make_any_analyzable_statistic_type
{
	typedef any_analyzable_statistic<typename StatisticTraitsT::value_type> type;
};


namespace detail {

template <
	typename StatisticT,
	typename StatisticTraitsT=typename ::dcs::type_traits::remove_reference<StatisticT>::type
>
struct make_any_analyzable_statistic_impl;


template <typename StatisticT, typename StatisticTraitsT>
struct make_any_analyzable_statistic_impl
{
	typedef typename make_any_analyzable_statistic_type<StatisticT,StatisticTraitsT>::type any_analyzable_statistic_type;

	static any_analyzable_statistic_type apply(StatisticT& stat)
	{
		return any_analyzable_statistic_type(stat);
	}
};


template <typename StatisticT, typename StatisticTraitsT>
struct make_any_analyzable_statistic_impl<StatisticT&,StatisticTraitsT>
{
	typedef typename make_any_analyzable_statistic_type<StatisticT,StatisticTraitsT>::type any_analyzable_statistic_type;

	static any_analyzable_statistic_type apply(StatisticT& stat)
	{
		::dcs::util::holder<StatisticT&> wrap_stat(stat);
		return any_analyzable_statistic_type(wrap_stat);
	}
};

} // Namespace detail


template <typename StatisticT>
typename make_any_analyzable_statistic_type<StatisticT>::type make_any_analyzable_statistic(StatisticT stat)
{
	return detail::make_any_analyzable_statistic_impl<StatisticT>::apply(stat);
}

}} // Namespace dcs:des


#endif // DCS_DES_ANY_ANALYZABLE_STATISTIC_HPP
