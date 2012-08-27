/**
 * \file dcs/des/model/qn/queueing_strategy.hpp
 *
 * \brief A generic queueing strategy.
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

#ifndef DCS_DES_MODEL_QN_QUEUEING_STRATEGY_HPP
#define DCS_DES_MODEL_QN_QUEUEING_STRATEGY_HPP


#include <boost/smart_ptr.hpp>
#include <cstddef>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <stdexcept>


namespace dcs { namespace des { namespace model { namespace qn {

template <typename TraitsT>
class queueing_strategy
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::customer_type customer_type;
	public: typedef typename ::boost::shared_ptr<customer_type> customer_pointer;
	//public: typedef ::std::size_t size_type;
	public: typedef ::std::ptrdiff_t size_type;


	public: static const size_type inf_capacity = -1;


	public: queueing_strategy()
	: capacity_(inf_capacity),
	  is_inf_(true)
	{
	}


	public: explicit queueing_strategy(size_type capacity)
	: capacity_(capacity < 0 ? inf_capacity : capacity),
	  is_inf_(capacity >= 0 ? false : true)
	{
	}


	public: virtual ~queueing_strategy()
	{
	}


	// Compiler-generated copy-constructor and copy-assignment are fine.


	public: void infinite_capacity(bool flag)
	{
		is_inf_ = flag;
		if (is_inf_)
		{
			capacity_ = inf_capacity;
		}
	}


	public: bool infinite_capacity() const
	{
		return is_inf_;
	}


	public: void capacity(size_type capacity)
	{
		capacity_ = capacity;
		is_inf_ = false;
	}


	public: size_type capacity() const
	{
		if (is_inf_)
		{
			return inf_capacity;
		}

		return capacity_;
	}


	public: bool can_push(customer_pointer const& ptr_customer) const
	{
		// pre: customer pointer must be a valid pointer.
		DCS_ASSERT(
			ptr_customer,
			throw ::std::invalid_argument("[dcs::des::model::qn::queueing_strategy::can_push] Invalid customer.")
		);

		return do_can_push(ptr_customer);
	}


	public: void push(customer_pointer const& ptr_customer)
	{
		// pre: customer pointer must be a valid pointer.
		DCS_ASSERT(
			ptr_customer,
			throw ::std::invalid_argument("[dcs::des::model::qn::queueing_strategy::can_push] Invalid customer.")
		);

		do_push(ptr_customer);
	}


	public: void pop()
	{
		do_pop();
	}


	public: void push_back(customer_pointer const& ptr_customer)
	{
		// pre: customer pointer must be a valid pointer.
		DCS_ASSERT(
			ptr_customer,
			throw ::std::invalid_argument("[dcs::des::model::qn::queueing_strategy::can_push] Invalid customer.")
		);

		do_push_back(ptr_customer);
	}


	public: customer_pointer const& peek() const
	{
		return do_peek();
	}


	public: customer_pointer peek()
	{
		return do_peek();
	}


	public: bool empty() const
	{
		return do_empty();
	}


	public: size_type size() const
	{
		return do_size();
	}


	public: void reset()
	{
		do_reset();
	}


	private: virtual bool do_can_push(customer_pointer const& ptr_customer) const = 0;


	private: virtual void do_push(customer_pointer const& ptr_customer) = 0;


	private: virtual void do_pop() = 0;


	private: virtual void do_push_back(customer_pointer const& ptr_customer) = 0;


	private: virtual customer_pointer const& do_peek() const = 0;


	private: virtual customer_pointer do_peek() = 0;


	private: virtual bool do_empty() const = 0;


	private: virtual size_type do_size() const = 0;


	private: virtual void do_reset() = 0;


	private: size_type capacity_;
	private: bool is_inf_;
};


}}}} // Namespace dcs::des::model::qn


#endif // DCS_DES_MODEL_QN_QUEUEING_STRATEGY_HPP
