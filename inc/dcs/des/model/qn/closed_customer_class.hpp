/**
 * \file dcs/des/model/qn/closed_customer_class.hpp
 *
 * \brief Closed customer class.
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

#ifndef DCS_DES_MODEL_QN_CLOSED_CUSTOMER_CLASS_HPP
#define DCS_DES_MODEL_QN_CLOSED_CUSTOMER_CLASS_HPP


#include <cstddef>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/model/qn/customer_class.hpp>


namespace dcs { namespace des { namespace model { namespace qn {

template <typename TraitsT>
class closed_customer_class: public customer_class<TraitsT>
{
	public: typedef TraitsT traits_type;
	private: typedef customer_class<TraitsT> base_type;
	public: typedef typename base_type::identifier_type identifier_type;
	private: typedef typename base_type::customer_type customer_type;
	private: typedef typename base_type::customer_pointer customer_pointer;


	public: closed_customer_class(::std::string const& name, ::std::size_t size)
	: base_type(name),
	  size_(size)
	{
	}


	public: closed_customer_class(identifier_type id, ::std::string const& name, ::std::size_t size)
	: base_type(id, name),
	  size_(size)
	{
	}


	// Compiler-generator copy-constructor, copy-assignment and destructor
	// are fine.


	public: ::std::size_t size() const
	{
		return size_;
	}


	public: void size(::std::size_t s)
	{
		size_ = s;
	}


	private: customer_class_category do_category() const
	{
		return closed_customer_class_category;
	}


	private: customer_pointer do_make_customer() const
	{
		//TODO
		throw ::std::runtime_error("[dcs::des::model::qn::closed_customer_class::do_make_customer] Not yet implemented.");

		// precondition: class has already been associated to a network
		DCS_ASSERT(
			this->network_ptr(),
			throw ::std::logic_error("[dcs::des::model::qn::closed_customer_class::do_make_customer] Missing queueing network association.")
		);
		// precondition: reference node is a valid node
		DCS_ASSERT(
			this->reference_node() != traits_type::invalid_node_id(),
			throw ::std::logic_error("[dcs::des::model::qn::closed_customer_class::do_make_customer] Missing reference node.")
		);
 
		customer_pointer ptr_customer(
				new customer_type(
					this->network_ptr()->generate_customer_id(),
					this->id(),
					this->reference_node()
				)
			);

		// Setup arrival time
		ptr_customer->arrival_time(this->network().engine().simulated_time());

		return ptr_customer;
	}


	private: ::std::size_t size_;
};


}}}} // Namespace dcs::des::model::qn


#endif // DCS_DES_MODEL_QN_CLOSED_CUSTOMER_CLASS_HPP
