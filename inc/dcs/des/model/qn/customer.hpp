/**
 * \file dcs/des/model/qn/customer.hpp
 *
 * \brief Customer
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

#ifndef DCS_DES_MODEL_QN_CUSTOMER_HPP
#define DCS_DES_MODEL_QN_CUSTOMER_HPP


#include <boost/smart_ptr.hpp>
#include <cstddef>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/model/qn/server_utilization_profile.hpp>
#include <iostream>
//#include <limits>
#include <map>
#include <stdexcept>
#include <vector>


namespace dcs { namespace des { namespace model { namespace qn {

template <typename TraitsT>
class customer
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::class_type class_type;
	public: typedef typename traits_type::class_identifier_type class_identifier_type;
	public: typedef typename traits_type::node_type node_type;
	public: typedef typename traits_type::node_identifier_type node_identifier_type;
	public: typedef typename traits_type::priority_type priority_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef ::std::size_t identifier_type;
	public: typedef typename traits_type::network_type network_type;
	public: typedef ::boost::shared_ptr<network_type> network_pointer;
	public: typedef server_utilization_profile<real_type> utilization_profile_type;//EXP
//	public: typedef typename traits_type::network_type* network_pointer;


//	/// The time value returned when the time information is not yet available.
//	private: static const real_type bad_time_;


	public: enum life_status
	{
		born_status, ///< Customer has entered in the network
		node_arrived_status, ///< Customer is arrived to a node.
		node_entered_status, ///< Customer has entered a node.
		node_served_status, ///< Customer has been served to a node.
		node_killed_status, ///< Customer has been killed by a node.
		node_exited_status, ///< Customer is exited from a node.
		node_left_status, ///< Customer has left a node.
		died_status ///< Customer has left the network
	};


//	private: static identifier_type global_id_;


	public: customer()
	: id_(),
	  class_id_(traits_type::invalid_class_id()),
	  old_class_id_(traits_type::invalid_class_id()),
	  node_id_(traits_type::invalid_node_id()),
	  old_node_id_(traits_type::invalid_node_id()),
	  priority_(),
	  status_(born_status),
	  arrtime_(0),
	  runtime_(0),
	  deptime_(0),
	  node_arrtimes_(),
//	  node_runtimes_(),
	  node_deptimes_(),
	  node_util_profiles_()
	{
		// Empty
	}


	public: customer(identifier_type cid, class_identifier_type c, node_identifier_type n)
	: id_(cid),
	  class_id_(c),
	  old_class_id_(traits_type::invalid_class_id()),
	  node_id_(n),
	  old_node_id_(traits_type::invalid_node_id()),
	  priority_(),
	  status_(born_status),
	  arrtime_(0),
	  runtime_(0),
	  deptime_(0),
	  node_arrtimes_(),
//	  node_runtimes_(),
	  node_deptimes_()
	{
		// precondition: the input class has a valid ID
		DCS_ASSERT(
			class_id_ != traits_type::invalid_class_id(),
			::std::invalid_argument("[dcs::des::model::qn::customer::ctor] Class has an invalid ID.")
		);
		// precondition: the input node has a valid ID
		DCS_ASSERT(
			node_id_ != traits_type::invalid_node_id(),
			::std::invalid_argument("[dcs::des::model::qn::customer::ctor] Node has an invalid ID.")
		);
	}


	// Compiler-generator copy-constructor, copy-assignment, and destructor
	// are fine.


	public: identifier_type id() const
	{
		return id_;
	}


	public: void change_class(class_identifier_type c)
	{
		// precondition: the input class has a valid ID
		DCS_ASSERT(
			c != traits_type::invalid_class_id(),
			::std::invalid_argument("[dcs::des::model::qn::customer::change_class] Class has an invalid ID.")
		);

		old_class_id_ = class_id_;
		class_id_ = c;
	}


	public: class_identifier_type current_class() const
	{
		return class_id_;
	}


	public: class_identifier_type previous_class() const
	{
		return old_class_id_;
	}


	public: void change_node(node_identifier_type n)
	{
		// precondition: the input node has a valid ID
		DCS_ASSERT(
			n != traits_type::invalid_node_id(),
			::std::invalid_argument("[dcs::des::model::qn::customer::change_node] Node has an invalid ID.")
		);

		old_node_id_ = node_id_;
		node_id_ = n;
	}


	public: node_identifier_type current_node() const
	{
		return node_id_;
	}


	public: node_identifier_type previous_node() const
	{
		return old_node_id_;
	}


	public: priority_type priority() const
	{
		return priority_;
	}


	public: void priority(priority_type p)
	{
		priority_ = p;
	}


	public: void status(life_status s)
	{
		status_ = s;
	}


	public: life_status status() const
	{
		return status_;
	}


	public: void arrival_time(real_type time)
	{
		arrtime_ = time;
	}


	public: real_type arrival_time() const
	{
		return arrtime_;
	}


	public: void departure_time(real_type time)
	{
		deptime_ = time;
	}


	public: real_type departure_time() const
	{
		return deptime_;
	}


	public: void runtime(real_type time)
	{
		runtime_ = time;
	}


	public: real_type runtime() const
	{
		return runtime_;
	}


	public: void node_arrival_time(node_identifier_type node_id, real_type time)
	{
//		if (node_id >= node_arrtimes_.size())
//		{
//			node_arrtimes_.resize(node_id+1);
//		}

		DCS_DEBUG_ASSERT(
					(node_arrtimes_.count(node_id) == 0 && node_deptimes_.count(node_id) == 0)
				||   node_arrtimes_[node_id].size() == node_deptimes_[node_id].size()
			);

		if (node_arrtimes_.count(node_id) == 0)
		{
			node_arrtimes_[node_id] = ::std::vector<real_type>();
			node_deptimes_[node_id] = ::std::vector<real_type>();
		}

		node_arrtimes_[node_id].push_back(time);
	}


	public: ::std::vector<real_type> node_arrival_times(node_identifier_type node_id) const
	{
//		if (node_id >= node_arrtimes_.size())
//		{
////			return bad_time_;
//			return ::std::vector<real_type>();
//		}
		if (node_arrtimes_.count(node_id) == 0)
		{
			return ::std::vector<real_type>();
		}

		return node_arrtimes_.at(node_id);
	}


	public: void node_departure_time(node_identifier_type node_id, real_type time)
	{
//		if (node_id >= node_deptimes_.size())
//		{
//			node_deptimes_.resize(node_id+1);
//		}

		DCS_DEBUG_ASSERT(
					(node_arrtimes_.count(node_id) == 1 && node_deptimes_.count(node_id) == 0)
				||   node_arrtimes_[node_id].size() == (node_deptimes_[node_id].size()+1)
			);

		if (node_deptimes_.count(node_id) == 0)
		{
			node_deptimes_[node_id] = ::std::vector<real_type>();
		}

		node_deptimes_[node_id].push_back(time);
	}


	public: ::std::vector<real_type> node_departure_times(node_identifier_type node_id) const
	{
//		if (node_id >= node_deptimes_.size())
//		{
////			return bad_time_;
//			return ::std::vector<real_type>();
//		}
		if (node_deptimes_.count(node_id) == 0)
		{
			return ::std::vector<real_type>();
		}
		return node_deptimes_.at(node_id);
	}


	public: void node_utilization_profile(node_identifier_type node_id, utilization_profile_type const& profile)
	{
		node_util_profiles_[node_id].push_back(profile);
	}


	public: ::std::vector<utilization_profile_type> node_utilization_profiles(node_identifier_type node_id) const
	{
		if (node_util_profiles_.count(node_id) == 0)
		{
			return ::std::vector<utilization_profile_type>();
		}
		return node_util_profiles_.at(node_id);
	}


	/// The customer identifier.
	private: identifier_type id_;
	/// The current class of this customer.
	private: class_identifier_type class_id_;
	/// The previous class of this customer.
	private: class_identifier_type old_class_id_;
	/// The identifier of the node where this customer currently resides.
	private: node_identifier_type node_id_;
	/// The identifier of the node where this customer previously resided.
	private: node_identifier_type old_node_id_;
	/// The current customer priority (not *yet* used).
	private: priority_type priority_;
	/// The current life status of this customer.
	private: life_status status_;
	/// The arrival time to the network.
	private: real_type arrtime_;
	/// The effective running time (without waiting time).
	private: real_type runtime_;
	/// The departure time from the network.
	private: real_type deptime_;
	/// The arrival time of every passage to each node.
	private: ::std::map< node_identifier_type ,::std::vector<real_type> > node_arrtimes_;
//	private: ::std::vector<real_type> runtimes_;
	/// The departure time of every passage from each node.
	private: ::std::map< node_identifier_type, ::std::vector<real_type> > node_deptimes_;
	/// The per-node collection of utilization profiles
	private: ::std::map< node_identifier_type, ::std::vector<utilization_profile_type> > node_util_profiles_;
};


//template <typename TraitsT>
//typename customer<TraitsT>::identifier_type customer<TraitsT>::global_id_ = identifier_type/*zero*/();


//template <typename TraitsT>
//const typename customer<TraitsT>::real_type customer<TraitsT>::bad_time_ = ::std::numeric_limits<typename customer<TraitsT>::real_type>::infinity();


template <
	typename CharT,
	typename CharTraitsT,
	typename QueueNetTraitsT
>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, customer<QueueNetTraitsT> const& c)
{
	return os << "<"
			  <<   "ID: " << c.id()
			  << ", Class: " << c.current_class()
			  << ", Node: " << c.current_node()
			  << ", Priority: " << c.priority()
			  << ", Status: " << c.status()
			  << ", Arrival Time: " << c.arrival_time()
			  << ", Departure Time: " << c.departure_time()
			  << ", Runtime: " << c.runtime()
			  << ">";
}

}}}} // Namespace dcs::des::model::qn


#endif // DCS_DES_MODEL_QN_CUSTOMER_HPP
