/**
 * \file dcs/des/model/qn/lcfs_queueing_strategy.hpp
 *
 * \brief Last-Come First-Served (LCFS) queueing strategy.
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

#ifndef DCS_DES_MODEL_QN_LCFS_QUEUEING_STRATEGY_HPP
#define DCS_DES_MODEL_QN_LCFS_QUEUEING_STRATEGY_HPP


#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/model/qn/queueing_strategy.hpp>
#include <stack>


namespace dcs { namespace des { namespace model { namespace qn {

template <typename TraitsT>
class lcfs_queueing_strategy: public queueing_strategy<TraitsT>
{
	public: typedef queueing_strategy<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename base_type::customer_pointer customer_pointer;
	private: typedef ::std::stack<customer_pointer> customer_container;
	public: typedef typename base_type::size_type size_type;


	public: lcfs_queueing_strategy()
	: base_type()
	{
	}


	public: explicit lcfs_queueing_strategy(size_type capacity)
	: base_type(capacity)
	{
	}


	// Compiler-generated copy-constructor, copy-assignment, and destructor
	// are fine.


	private: bool do_can_push(customer_pointer const& ptr_customer) const
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_customer );

		return this->infinite_capacity() || (this->size() < this->capacity());
	}


	private: void do_push(customer_pointer const& ptr_customer)
	{
		// pre: customer pointer must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_customer );
		// pre: queue is not full
		DCS_ASSERT(
			this->can_push(ptr_customer),
			throw ::std::logic_error("[dcs::des::model::qn::lcfs_queueing_strategy::do_push] Queue is full.")
		);

		stack_.push(ptr_customer);
	}


	private: void do_push_back(customer_pointer const& ptr_customer)
	{
		// pre: customer pointer must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_customer );
		// pre: queue is not full
		DCS_ASSERT(
			this->can_push(ptr_customer),
			throw ::std::logic_error("[dcs::des::model::qn::lcfs_queueing_strategy::do_push_back] Queue is full.")
		);

		stack_.push(ptr_customer);
	}


	private: void do_pop()
	{
		// pre: queue is not empty
		DCS_ASSERT(
			!this->empty(),
			throw ::std::logic_error("[dcs::des::model::qn::lcfs_queueing_strategy::do_pop] Queue is empty.")
		);

		stack_.pop();
	}


	private: bool do_empty() const
	{
		return stack_.empty();
	}


	private: size_type do_size() const
	{
		return stack_.size();
	}


	private: customer_pointer const& do_peek() const
	{
		// pre: queue is not empty
		DCS_ASSERT(
			!this->empty(),
			throw ::std::logic_error("[dcs::des::model::qn::lcfs_queueing_strategy::do_peek] Queue is empty.")
		);

		return stack_.top();
	}


	private: customer_pointer do_peek()
	{
		// pre: queue is not empty
		DCS_ASSERT(
			!this->empty(),
			throw ::std::logic_error("[dcs::des::model::qn::lcfs_queueing_strategy::do_peek] Queue is empty.")
		);

		return stack_.top();
	}


	private: void do_reset()
	{
		stack_ = customer_container();
	}


	private: customer_container stack_;
};

}}}} // Namespace dcs::des::model::qn


#endif // DCS_DES_MODEL_QN_LCFS_QUEUEING_STRATEGY_HPP
