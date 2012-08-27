/**
 * \file dcs/des/model/qn/customer_class.hpp
 *
 * \brief Base class for customer classes.
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

#ifndef DCS_DES_MODEL_QN_CUSTOMER_CLASS_HPP
#define DCS_DES_MODEL_QN_CUSTOMER_CLASS_HPP


#include <boost/smart_ptr.hpp>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/model/qn/customer_class_category.hpp>
#include <iostream>
#include <stdexcept>
#include <string>


namespace dcs { namespace des { namespace model { namespace qn {

template <typename TraitsT>
class customer_class
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::class_identifier_type identifier_type;
	public: typedef typename traits_type::node_type node_type;
	public: typedef typename traits_type::network_type network_type;
	public: typedef typename traits_type::customer_type customer_type;
	//public: typedef ::boost::shared_ptr<network_type> network_pointer;
	public: typedef network_type* network_pointer;
	public: typedef node_type* node_pointer;
	//public: typedef ::boost::shared_ptr<node_type> node_pointer;
	public: typedef ::boost::shared_ptr<customer_type> customer_pointer;
	private: typedef typename traits_type::node_identifier_type node_identifier_type;


	public: explicit customer_class(::std::string const& name)
	: id_(traits_type::invalid_class_id()),
	  name_(name),
	  node_id_(traits_type::invalid_node_id()),
	  ptr_net_()
	{
	}


	public: customer_class(identifier_type id, ::std::string const& name)
	: id_(id),
	  name_(name),
	  node_id_(traits_type::invalid_node_id()),
	  ptr_net_()
	{
	}


	public: virtual ~customer_class() { }


	// Compiler-generator copy-constructor, copy-assignment are fine.


	public: identifier_type id() const
	{
		return id_;
	}


	public: void id(identifier_type x)
	{
		id_ = x;
	}


	public: void name(::std::string const& s)
	{
		name_ = s;
	}


	public: ::std::string const& name() const
	{
		return name_;
	}


//	public: void reference_node(node_type const& node)
//	{
//		// precondition: node has a valid ID
//		DCS_ASSERT(
//			node.id() != traits_type::invalid_node_id,
//			::std::invalid_argument("[dcs::des::model::qn::customer_class::reference_node] Node with an invalid ID cannot be a reference node.")
//		);
//		// precondition: network already attached
//		DCS_ASSERT(
//			ptr_net_ != 0,
//			::std::logic_error("[dcs::des::model::qn::customer_class::reference_node] Class has not been added to a network yet.")
//		);
//
//		node_id_ = node.id();
//	}


//	public: node_type const& reference_node() const
//	{
//		// precondition: network already attached
//		DCS_ASSERT(
//			ptr_net_ != 0,
//			::std::logic_error("[dcs::des::model::qn::customer_class::reference_node] Class has not been added to a network yet.")
//		);
//
//		return ptr_net_->get_node(node_id_);
//	}


//	public: node_type& reference_node()
//	{
//		// precondition: network already attached
//		DCS_ASSERT(
//			ptr_net_ != 0,
//			::std::logic_error("[dcs::des::model::qn::customer_class::reference_node] Class has not been added to a network yet.")
//		);
//
//		return ptr_net_->get_node(node_id_);
//	}


	public: void reference_node(node_identifier_type n)
	{
		// precondition: node has a valid ID
		DCS_ASSERT(
			n != traits_type::invalid_node_id(),
			::std::invalid_argument("[dcs::des::model::qn::customer_class::reference_node] Node with an invalid ID cannot be a reference node.")
		);

		node_id_ = n;
	}


	public: node_identifier_type reference_node() const
	{
		return node_id_;
	}


	public: customer_pointer make_customer() const
	{
		return do_make_customer();
	}


	public: void network(network_pointer const& ptr_net)
	{
		// pre: network pointer must be a valid pointer.
		DCS_ASSERT(
			ptr_net,
			throw ::std::invalid_argument("[dcs::des::model::qn::customer_class::network] Invalid network.")
		);

		ptr_net_ = ptr_net;
	}


	public: network_type const& network() const
	{
		// pre: pointer to network must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_net_ );

		return *ptr_net_;
	}


	public: network_type& network()
	{
		// pre: pointer to network must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_net_ );

		return *ptr_net_;
	}


	protected: network_pointer const& network_ptr() const
	{
		// pre: pointer to network must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_net_ );

		return ptr_net_;
	}


	protected: network_pointer network_ptr()
	{
		// pre: pointer to network must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_net_ );

		return ptr_net_;
	}


	public: customer_class_category category() const
	{
		return do_category();
	}


	private: virtual customer_class_category do_category() const = 0;


	private: virtual customer_pointer do_make_customer() const = 0;


	private: identifier_type id_;
	private: ::std::string name_;
	private: node_identifier_type node_id_;
	private: network_pointer ptr_net_;
};


template <
	typename CharT,
	typename CharTraitsT,
	typename QueueNetTraitsT
>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, customer_class<QueueNetTraitsT> const& c)
{
	return os << "<"
			  << "ID: " << c.id()
			  << ", Name: " << c.name()
			  << ", Category: " << c.category()
			  << ">";
}

}}}} // Namespace dcs::des::model::qn


#endif // DCS_DES_MODEL_QN_CUSTOMER_CLASS_HPP
