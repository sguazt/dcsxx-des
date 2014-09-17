/**
 * \file dcs/des/any_statistic.hpp
 *
 * \brief Generic (type-erased) output statistic.
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

#ifndef DCS_DES_ANY_STATISTIC_HPP
#define DCS_DES_ANY_STATISTIC_HPP


#include <boost/smart_ptr.hpp>
#include <cstddef>
#include <dcs/des/base_statistic.hpp>
#include <dcs/des/statistic_adaptor.hpp>
#include <dcs/des/statistic_categories.hpp>
#include <dcs/type_traits/add_const.hpp>
#include <dcs/type_traits/add_reference.hpp>
#include <dcs/type_traits/remove_reference.hpp>
#include <dcs/util/holder.hpp>
#include <iosfwd>
#include <string>


namespace dcs { namespace des {

/**
 * \brief Generic (type-erased) output statistic.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename ValueT>
class any_statistic
{
	public: typedef ValueT value_type;


	public: template <typename CharT, typename CharTraits, typename VT>
		friend ::std::basic_ostream<CharT,CharTraits>& operator<<(::std::basic_ostream<CharT,CharTraits>& os, any_statistic<VT> const& stat);


	public: any_statistic() { }


	public: template <typename StatisticT>
		any_statistic(StatisticT const& stat)
		: ptr_stat_(new statistic_adaptor<StatisticT>(stat))
	{
		// Empty
	}


	public: template <typename StatisticT>
		any_statistic(::dcs::util::holder<StatisticT> const& wrap_stat)
		: ptr_stat_(new statistic_adaptor<StatisticT>(wrap_stat.get()))
	{
		// Empty
	}


	// Compiler-generated copy ctor and copy assignement are fine.


	public: template <typename StatisticT>
		void statistic(StatisticT stat)
	{
		ptr_stat_ = new statistic_adaptor<StatisticT>(stat);
	}


	///@{ Statistic concept implementation

	public: void operator()(value_type obs, value_type weight = value_type(1))
	{
		ptr_stat_->operator()(obs, weight);
	}


	public: statistic_category category() const
	{
		return ptr_stat_->category();
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


	public: ::std::string name() const
	{
		return ptr_stat_->name();
	}


//	public: template <
//				typename CharT,
//				typename CharTraitsT
//			>
//		::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os)
//	{
//		return ptr_stat_->print();
//	}

	///@} Statistic concept implementation


	protected: base_statistic<value_type>& statistic()
	{
		return *ptr_stat_;
	}


	protected: base_statistic<value_type> const& statistic() const
	{
		return *ptr_stat_;
	}


	private: ::boost::shared_ptr< base_statistic<value_type> > ptr_stat_;
}; // any_statistic


template <
	typename CharT,
	typename CharTraits,
	typename ValueT
>
::std::basic_ostream<CharT,CharTraits>& operator<<(::std::basic_ostream<CharT,CharTraits>& os, any_statistic<ValueT> const& stat)
{
//	return stat.print(os);
	os << *(stat.ptr_stat_);

	return os;
}


template <
	typename StatisticT,
	typename StatisticTraitsT=typename ::dcs::type_traits::remove_reference<StatisticT>::type
>
struct make_any_statistic_type
{
	typedef any_statistic<typename StatisticTraitsT::value_type> type;
};


namespace detail {

template <
	typename StatisticT,
	typename StatisticTraitsT=typename ::dcs::type_traits::remove_reference<StatisticT>::type
>
struct make_any_statistic_impl;


template <typename StatisticT, typename StatisticTraitsT>
struct make_any_statistic_impl
{
	typedef typename make_any_statistic_type<StatisticT,StatisticTraitsT>::type any_statistic_type;

	static any_statistic_type apply(StatisticT& stat)
	{
		return any_statistic_type(stat);
	}
};


template <typename StatisticT, typename StatisticTraitsT>
struct make_any_statistic_impl<StatisticT&,StatisticTraitsT>
{
	typedef typename make_any_statistic_type<StatisticT,StatisticTraitsT>::type any_statistic_type;

	static any_statistic_type apply(StatisticT& stat)
	{
		::dcs::util::holder<StatisticT&> wrap_stat(stat);
		return any_statistic_type(wrap_stat);
	}
};

} // Namespace detail


template <typename StatisticT>
typename make_any_statistic_type<StatisticT>::type make_any_statistic(StatisticT stat)
{
	return detail::make_any_statistic_impl<StatisticT>::apply(stat);
}

}} // Namespace dcs:des


#endif // DCS_DES_ANY_STATISTIC_HPP
