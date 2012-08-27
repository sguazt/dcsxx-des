/**
 * \file dcs/des/model/qn/server_utilization_profile.hpp
 *
 * \brief Utilization profile associated to a server.
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

#ifndef DCS_DES_MODEL_QN_SERVER_UTILIZATION_PROFILE_HPP
#define DCS_DES_MODEL_QN_SERVER_UTILIZATION_PROFILE_HPP


#include <boost/icl/interval.hpp>
#include <boost/icl/interval_map.hpp>
#include <boost/smart_ptr.hpp>
#include <iosfwd>
#include <iterator>
#include <utility>


namespace dcs { namespace des { namespace model { namespace qn {

namespace detail {

template <typename ItemT>
struct item_const_wrapper
{
	public: typedef ItemT item_type;
	private: typedef typename item_type::first_type interval_type;
	public: typedef typename interval_type::domain_type time_type;
	public: typedef typename item_type::second_type value_type;
	private: typedef item_type const& reference;
	private: typedef item_type const* pointer;

	public: item_const_wrapper(item_type const& item)
	: ptr_item_(&item)
	{
	}

	public: time_type begin_time() const { return ::boost::icl::lower(ptr_item_->first); }

	public: time_type end_time() const { return ::boost::icl::upper(ptr_item_->first); }

	public: value_type utilization() const { return ptr_item_->second; }

	private: pointer ptr_item_;
};


template <typename ItemT>
struct item_wrapper
{
	public: typedef ItemT item_type;
	private: typedef typename ::boost::remove_const<item_type>::first_type interval_type;
	public: typedef typename interval_type::domain_type time_type;
	public: typedef typename item_type::second_type value_type;
	private: typedef item_type& reference;
	private: typedef item_type* pointer;

	public: item_wrapper(item_type& item)
	: ptr_item_(&item)
	{
	}

	public: time_type begin_time() const { return ::boost::icl::lower(ptr_item_->first); }

	public: time_type end_time() const { return ::boost::icl::upper(ptr_item_->first); }

	public: value_type utilization() const { return ptr_item_->second; }
	public: void utilization(value_type v) { ptr_item_->second_ = v; }

	private: pointer ptr_item_;
};

} // Namespace detail


//template <typename RealT>
//class server_utilization_profile_item
//{
	//public: typedef RealT real_type;


	//public: server_utilization_profile_item()
	//: t1_(0),
	  //t2_(0),
	  //u_(0)
	//{
	//}


	//public: server_utilization_profile_item(real_type t1, real_type t2, real_type u)
	//: t1_(t1),
	  //t2_(t2),
	  //u_(u)
	//{
	//}


	//public: void begin_time(real_type t)
	//{
		//t1_ = t;
	//}


	//public: real_type begin_time() const
	//{
		//return t1_;
	//}


	//public: void end_time(real_type t)
	//{
		//t2_ = t;	
	//}


	//public: real_type end_time() const
	//{
		//return t2_;
	//}


	//public: void utilization(real_type u)
	//{
		//u_ = u;
	//}


	//public: real_type utilization() const
	//{
		//return u_;
	//}


	//private: real_type t1_;
	//private: real_type t2_;
	//private: real_type u_;
//};


template <typename RealT>
class server_utilization_profile
{
	public: typedef RealT real_type;
//	public: typedef server_utilization_profile_item<real_type> item_type;
	private: typedef ::boost::icl::interval<real_type> interval_type;
	private: typedef ::boost::icl::interval_map<real_type,real_type> interval_container;


	public: class const_iterator;
	public: class iterator;
	public: typedef ::std::reverse_iterator<const_iterator> const_reverse_iterator;
	public: typedef ::std::reverse_iterator<iterator> reverse_iterator;


	template <typename CharT, typename CharTraitsT, typename TT>
	friend ::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, server_utilization_profile<TT> const& profile);


	public: void operator()(real_type t1, real_type t2, real_type u)
	{
		items_ += ::std::make_pair(interval_type::right_open(t1, t2), u);
	}


	public: iterator begin()
	{
		return iterator(items_.begin());
	}


	public: iterator end()
	{
		return iterator(items_.end());
	}


	public: const_iterator begin() const
	{
		return const_iterator(items_.begin());
	}


	public: const_iterator end() const
	{
		return const_iterator(items_.end());
	}


	public: reverse_iterator rbegin()
	{
		return reverse_iterator(items_.rbegin());
	}


	public: reverse_iterator rend()
	{
		return reverse_iterator(items_.rend());
	}


	public: const_reverse_iterator rbegin() const
	{
		return const_reverse_iterator(items_.rbegin());
	}


	public: const_reverse_iterator rend() const
	{
		return const_reverse_iterator(items_.rend());
	}


	public: class const_iterator
	{
        private: typedef typename interval_container::const_iterator interval_iterator;
        public: typedef typename interval_iterator::iterator_category iterator_category;
        private: typedef typename interval_container::value_type item_type;
        public: typedef detail::item_const_wrapper<item_type> value_type;
        public: typedef typename interval_iterator::difference_type difference_type;
        public: typedef ::boost::shared_ptr<value_type> pointer;
        public: typedef value_type reference;


        public: const_iterator()
        {
        }


        public: const_iterator(interval_iterator it)
        : it_(it)
        {
        }


        public: reference operator*() const
        {
			return detail::item_const_wrapper<item_type>(*it_);
        }


        public: pointer operator->() const
        {
            return ::boost::make_shared<item_type>(operator*());
        }


        public: const_iterator& operator++()
        {
            ++it_;
            return *this;
        }


        public: const_iterator& operator++(int)
        {
            const_iterator tmp(*this);
            ++it_;
            return tmp;
        }


        friend bool operator!=(const_iterator const& x, const_iterator const& y)
        {
            return x.it_ != y.it_;
        }


        friend bool operator==(const_iterator const& x, const_iterator const& y)
        {
            return x.it_ == y.it_;
        }


        friend bool operator>(const_iterator const& x, const_iterator const& y)
        {
            return x.it_ > y.it_;
        }


        friend bool operator<(const_iterator const& x, const_iterator const& y)
        {
            return x.it_ < y.it_;
        }


        private: interval_iterator it_;
	}; // const_iterator


	public: class iterator
	{
        private: typedef typename interval_container::const_iterator interval_iterator;
        public: typedef typename interval_iterator::iterator_category iterator_category;
        private: typedef typename interval_container::value_type item_type;
        public: typedef detail::item_wrapper<item_type> value_type;
        public: typedef typename interval_iterator::difference_type difference_type;
        public: typedef ::boost::shared_ptr<value_type> pointer;
        public: typedef value_type reference;


        public: iterator()
        {
        }


        public: iterator(interval_iterator it)
        : it_(it)
        {
        }


        public: reference operator*() const
        {
			return detail::item_wrapper<item_type>(*it_);
        }


        public: pointer operator->() const
        {
            return ::boost::make_shared<item_type>(operator*());
        }


        public: iterator& operator++()
        {
            ++it_;
            return *this;
        }


        public: iterator& operator++(int)
        {
            iterator tmp(*this);
            ++it_;
            return tmp;
        }


        public: friend bool operator!=(iterator const& x, iterator const& y)
        {
            return x.it_ != y.it_;
        }


        public: friend bool operator==(iterator const& x, iterator const& y)
        {
            return x.it_ == y.it_;
        }


        public: friend bool operator>(iterator const& x, iterator const& y)
        {
            return x.it_ > y.it_;
        }


        public: friend bool operator<(iterator const& x, iterator const& y)
        {
            return x.it_ < y.it_;
        }


        private: interval_iterator it_;
	}; // iterator


	private: interval_container items_;
}; // server_utilization_profile


template <typename CharT, typename CharTraitsT, typename TraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os,
													server_utilization_profile<TraitsT> const& profile)
{
	os << profile.items_;

	return os;
}

}}}} // Namespace dcs::des::model::qn


#endif // DCS_DES_MODEL_QN_SERVER_UTILIZATION_PROFILE_HPP
