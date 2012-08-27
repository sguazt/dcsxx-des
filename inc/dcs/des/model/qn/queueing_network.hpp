/**
 * \file dcs/des/model/qn/queueing_network.hpp
 *
 * \brief The whole queueing network.
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

#ifndef DCS_DES_MODEL_QN_QUEUEING_NETWORK_HPP
#define DCS_DES_MODEL_QN_QUEUEING_NETWORK_HPP


#include <algorithm>
#include <boost/smart_ptr.hpp>
#include <cstddef>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/base_statistic.hpp>
#include <dcs/des/entity.hpp>
#include <dcs/des/model/qn/customer.hpp>
#include <dcs/des/model/qn/customer_class.hpp>
#include <dcs/des/model/qn/network_node.hpp>
#include <dcs/des/model/qn/output_statistic_category.hpp>
#include <dcs/des/model/qn/queueing_network_traits.hpp>
#include <dcs/exception.hpp>
#include <dcs/functional/bind.hpp>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>


namespace dcs { namespace des { namespace model { namespace qn {

/**
* \brief Queueing Network model.
*
* A queueing network model \c QN is described by the following tuple:
* \f[
*   QN = \langle R, S, \mathbf{P} \rangle
* \f]
* where:
* - \f$R\f$ is the set of customer classes
* - \f$S\f$ is the set of service nodes
* - \f$\mathbf{P}\f$ is the routing matrix
* .
*
* \author Marco Guazzone (marco.guazzone@gmail.com)
*/
template <
typename UIntT,
typename RealT,
typename UniformRandomGeneratorT,
typename DesEngineT/*,
typename ClassT = customer_class,
typename NodeT = network_node,
typename PrioriryT = int*/
>
class queueing_network: public ::dcs::des::entity//, public dcs::enable_shared_from_this< queueing_network<UIntT,RealT,UniformRandomGeneratorT,DesEngineT> >
{
	//@{ Typedefs


	private: typedef ::dcs::des::entity base_type;
	private: typedef queueing_network<
						UIntT,
						RealT,
						UniformRandomGeneratorT,
						DesEngineT/*,
						ClassT,
						NodeT,
						PriorityT*/> self_type;
	private: typedef queueing_network_traits<self_type> traits_type;
	public: typedef UIntT uint_type;
	public: typedef RealT real_type;
	public: typedef UniformRandomGeneratorT random_generator_type;
	public: typedef DesEngineT engine_type;
	//	public: typedef ClassT class_type;
	//	public: typedef NodeT node_type;
	//	public: typedef PriorityT priority_type;
	public: typedef customer_class<traits_type> class_type;
	public: typedef network_node<traits_type> node_type;
	public: typedef customer<traits_type> customer_type;
//	public: typedef typename customer_type::identifier_type customer_identifier_type;//XXX: cannot do this since the customer type might not be available
	public: typedef ::std::size_t customer_identifier_type;
	public: typedef int priority_type;
	public: typedef ::boost::shared_ptr<class_type> class_pointer;
	public: typedef ::boost::shared_ptr<node_type> node_pointer;
	private: typedef ::std::vector<class_pointer> class_container;
	private: typedef ::std::vector<node_pointer> node_container;
	private: typedef ::boost::shared_ptr<customer_type> customer_pointer;
	public: typedef typename class_container::size_type class_size_type;
	public: typedef typename node_container::size_type node_size_type;
	public: typedef class_size_type class_identifier_type;
	public: typedef node_size_type node_identifier_type;
	public: typedef ::boost::shared_ptr<random_generator_type> random_generator_pointer;
	public: typedef ::boost::shared_ptr<engine_type> engine_pointer;
	public: typedef base_statistic<real_type,uint_type> output_statistic_type;
	public: typedef ::boost::shared_ptr<output_statistic_type> output_statistic_pointer;
	private: typedef ::std::vector<node_identifier_type> node_id_container;
	private: typedef ::std::vector<node_id_container> class_node_container;
	private: typedef typename engine_type::event_source_type event_source_type;
	private: typedef typename engine_type::event_type event_type;
	private: typedef typename engine_type::engine_context_type engine_context_type;
	private: typedef ::boost::shared_ptr<event_source_type> event_source_pointer;
	// DEVEL-NOTE: we model the output statistic container as map of category to
	//  vectors of stats so that for each stats category (e.g., response time,
	//  throughput, ...) we can collect different kind of stats (e.g., mean,
	//  quantiles, ...).
	private: typedef ::std::vector<output_statistic_pointer> output_statistic_container;
	private: typedef ::std::map<network_output_statistic_category,output_statistic_container> output_statistic_category_container;


	//@} Typedefs


	//@{ Constants


	public: static const node_identifier_type invalid_node_id;
	public: static const class_identifier_type invalid_class_id;
	private: static const ::std::string arrival_event_source_name;
	private: static const ::std::string departure_event_source_name;
	private: static const ::std::string discard_event_source_name;


	//@} Constants


	//@{ Member functions


	//	public: queueing_network()
	//	{
	//	}


	/// A constructor.
	public: queueing_network(random_generator_pointer const& ptr_rng,
							 engine_pointer const& ptr_eng,
							 bool enabled = true)
	: base_type(enabled),
	  classes_(),
	  nodes_(),
	  ptr_rng_(ptr_rng),
	  ptr_eng_(ptr_eng),
	  next_customer_id_(0),
	  ptr_arr_evt_src_(new event_source_type(arrival_event_source_name)),
	  ptr_dep_evt_src_(new event_source_type(departure_event_source_name)),
	  ptr_dis_evt_src_(new event_source_type(discard_event_source_name)),
	  narr_(0),
	  ndep_(0),
	  ndis_(0)
	{
		DCS_DEBUG_TRACE_L(5, "(" << this << ") BEGIN Constructor");//XXX

		// pre: random number generator pointer must be a valid pointer
		DCS_ASSERT(
			ptr_rng_,
			throw ::std::invalid_argument("[dcs::des::model::qn::queueing_network::ctor] Invalid random number generator.")
		);
		// pre: DES engine pointer must be a valid pointer
		DCS_ASSERT(
			ptr_eng_,
			throw ::std::invalid_argument("[dcs::des::model::qn::queueing_network::ctor] Invalid DES engine.")
		);

		init();

		DCS_DEBUG_TRACE_L(5, "(" << this << ") END Constructor");//XXX
	}


	/// A constructor.
	public: queueing_network(class_size_type nc,
							 node_size_type ns,
							 random_generator_pointer const& ptr_rng,
							 engine_pointer const& ptr_eng,
							 bool enabled = true)
	: base_type(enabled),
	  classes_(nc),
	  nodes_(ns),
	  ptr_rng_(ptr_rng),
	  ptr_eng_(ptr_eng),
	  next_customer_id_(0),
	  ptr_arr_evt_src_(new event_source_type(arrival_event_source_name)),
	  ptr_dep_evt_src_(new event_source_type(departure_event_source_name)),
	  ptr_dis_evt_src_(new event_source_type(discard_event_source_name)),
	  narr_(0),
	  ndep_(0),
	  ndis_(0)
	{
		DCS_DEBUG_TRACE_L(5, "(" << this << ") BEGIN Constructor");//XXX

		// pre: random number generator pointer must be a valid pointer
		DCS_ASSERT(
			ptr_rng_,
			throw ::std::invalid_argument("[dcs::des::model::qn::queueing_network::ctor] Invalid random number generator.")
		);
		// pre: DES engine pointer must be a valid pointer
		DCS_ASSERT(
			ptr_eng_,
			throw ::std::invalid_argument("[dcs::des::model::qn::queueing_network::ctor] Invalid DES engine.")
		);

		init();

		DCS_DEBUG_TRACE_L(5, "(" << this << ") END Constructor");//XXX
	}


	/// The copy constructor.
	private: queueing_network(queueing_network const& that)
	: base_type(that)
	{
		DCS_EXCEPTION_THROW(::std::runtime_error, "Broken implementation");

		//FIXME: there are some open issues with the copy of pointers to
		//       object (see below).

		DCS_DEBUG_TRACE_L(5, "(" << this << ") BEGIN Copy Constructor");//XXX

		// Classes
		typedef typename class_container::const_iterator class_iterator;
		class_iterator class_end_it = that.classes_.end();
		for (class_iterator it = that.classes_.begin(); it != class_end_it; ++it)
		{
//NO: cannot be done since class_type is an abstract class!!
//			class_pointer ptr_class(new class_type(*(*it)));
//			this->add_node(ptr_class);

			//FIXME: this copy may be harmfull since we are changing the
			//       state (i.e., the network pointer in this case) of a
			//       pointer to an object that may be used in another class.
			this->add_class(*it);
		}
		// Nodes
		typedef typename node_container::const_iterator node_iterator;
		node_iterator node_end_it = that.nodes_.end();
		for (node_iterator it = that.nodes_.begin(); it != node_end_it; ++it)
		{
//NO: cannot be done since node_type is an abstract class!!
//			node_pointer ptr_node(new node_type(*(*it)));
//			this->add_node(ptr_node);

			//FIXME: this copy may be harmfull since we are changing the
			//       state (i.e., the network pointer in this case) of a
			//       pointer to an object that may be used in another class.
			this->add_node(*it);
		}
		// Random number generator (is a shared object)
		ptr_rng_ = that.ptr_rng_;
		// DES engine (is a shared object)
		ptr_eng_ = that.ptr_eng_;
		// Customer id generator
		next_customer_id_ = that.next_customer_id_;
		// Arrival event source
		ptr_arr_evt_src_ = event_source_pointer(new event_source_type(*(that.ptr_arr_evt_src_)));
		// Departure event source
		ptr_dep_evt_src_ = event_source_pointer(new event_source_type(*(that.ptr_dep_evt_src_)));
		// Discard event source
		ptr_dis_evt_src_ = event_source_pointer(new event_source_type(*(that.ptr_dis_evt_src_)));
		// # Arrivals
		narr_ = that.narr_;
		// # Departures
		ndep_ = that.ndep_;
		// # Discards
		ndis_ = that.ndis_;
		// Statistics
		typedef typename output_statistic_category_container::const_iterator output_statistic_category_iterator;
		typedef typename output_statistic_container::const_iterator output_statistic_iterator;
		output_statistic_category_iterator cat_stat_end_it = that.stats_.end();
		for (output_statistic_category_iterator it = that.stats_.begin(); it != cat_stat_end_it; ++it)
		{
			network_output_statistic_category category = it->first;
			output_statistic_iterator stat_end_it = it->second.end();

			for (output_statistic_iterator stat_it = it->second.begin(); stat_it != stat_end_it; ++stat_it)
			{
				this->statistic(category, *stat_it);
			}
		}

		init();

		DCS_DEBUG_TRACE_L(5, "(" << this << ") END Copy Constructor");//XXX
	}


	/// The destructor
	public: virtual ~queueing_network()
	{
		DCS_DEBUG_TRACE_L(5, "(" << this << ") BEGIN Destructor");//XXX

		// pre: pointer to NETWORK-ARRIVAL event source must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_arr_evt_src_ );
		// pre: pointer to NETWORK-DEPARTURE event source must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_dep_evt_src_ );
		// pre: pointer to NETWORK-DISCARD event source must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_dis_evt_src_ );
		// pre: pointer to DES engine must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_eng_ );

		finit();

		DCS_DEBUG_TRACE_L(5, "(" << this << ") END Destructor");//XXX
	}


	private: queueing_network& operator=(queueing_network const& rhs)
	{
		DCS_EXCEPTION_THROW(::std::runtime_error, "Broken implementation");

		//FIXME: there are some open issues with the copy of pointers to
		//       object (see below).

		DCS_DEBUG_TRACE_L(5, "(" << this << ") BEGIN Assignment operator");//XXX

		if (this != &rhs)
		{
			finit();

			base_type::operator=(rhs);

			// Classes
			classes_.clear(); // elements will automatically deleted
			typedef typename class_container::const_iterator class_iterator;
			class_iterator class_end_it = rhs.classes_.end();
			for (class_iterator it = rhs.classes_.begin(); it != class_end_it; ++it)
			{
//NO: cannot be done since class_type is an abstract class!!
//				class_pointer ptr_class(new class_type(*(*it)));
//				this->add_node(ptr_class);

				//FIXME: this copy may be harmfull since we are changing the
				//       state (i.e., the network pointer in this case) of a
				//       pointer to an object that may be used in another class.
				this->add_class(*it);
			}
			// Nodes
			nodes_.clear(); // elements will automatically deleted
			typedef typename node_container::const_iterator node_iterator;
			node_iterator node_end_it = rhs.nodes_.end();
			for (node_iterator it = rhs.nodes_.begin(); it != node_end_it; ++it)
			{
//NO: cannot be done since node_type is an abstract class!!
//				node_pointer ptr_node(new node_type(*(*it)));
//				this->add_node(ptr_node);

				//FIXME: this copy may be harmfull since we are changing the
				//       state (i.e., the network pointer in this case) of a
				//       pointer to an object that may be used in another class.
				this->add_node(*it);
			}
			// Random number generator (is a shared object)
			ptr_rng_ = rhs.ptr_rng_;
			// DES engine (is a shared object)
			ptr_eng_ = rhs.ptr_eng_;
			// Customer id generator
			next_customer_id_ = rhs.next_customer_id_;
			// Arrival event source
			ptr_arr_evt_src_ = event_source_pointer(new event_source_type(*(rhs.ptr_arr_evt_src_)));
			// Departure event source
			ptr_dep_evt_src_ = event_source_pointer(new event_source_type(*(rhs.ptr_dep_evt_src_)));
			// Discard event source
			ptr_dis_evt_src_ = event_source_pointer(new event_source_type(*(rhs.ptr_dis_evt_src_)));
			// # Arrivals
			narr_ = rhs.narr_;
			// # Departures
			ndep_ = rhs.ndep_;
			// # Discards
			ndis_ = rhs.ndis_;
			// Statistics
			stats_.clear(); // elements will be automatically deleted
			typedef typename output_statistic_category_container::const_iterator output_statistic_category_iterator;
			typedef typename output_statistic_container::const_iterator output_statistic_iterator;
			output_statistic_category_iterator cat_stat_end_it = rhs.stats_.end();
			for (output_statistic_category_iterator it = rhs.stats_.begin(); it != cat_stat_end_it; ++it)
			{
				network_output_statistic_category category = it->first;
				output_statistic_iterator stat_end_it = it->second.end();

				for (output_statistic_iterator stat_it = it->second.begin(); stat_it != stat_end_it; ++stat_it)
				{
					this->statistic(category, *stat_it);
				}
			}

			init();
		}

		DCS_DEBUG_TRACE_L(5, "(" << this << ") END Assignment operator");//XXX

		return *this;
	}


	/**
	 * \brief Add a (possibly new) customer class to this network.
	 *
	 * If the customer class already exists, the old one is overwritten.
	 */
	public: void add_class(class_pointer const& ptr_class)
	{
		DCS_DEBUG_TRACE_L(5, "(" << this << ") Adding class: " << *ptr_class << " - (" << ptr_class << ")");//XXX

		// precondition: ptr_class is not null
		DCS_ASSERT(
			ptr_class,
			::std::invalid_argument("[dcs::des::model::qn::queueing_network::add_node] Pointer to customer class is null.")
		);

		// Test if a valid ID has already been assigned
		if (ptr_class->id() == invalid_class_id)
		{
			ptr_class->id(classes_.size());
		}

		// Set a reference back to this network
		ptr_class->network(this);

		// Finally, make sure there is enough space...
		if (ptr_class->id() >= classes_.size())
		{
			classes_.resize(ptr_class->id()+1);
		}

		// ... And add the customer class.
		classes_[ptr_class->id()] = ptr_class;

		DCS_DEBUG_TRACE_L(5, "(" << this << ") Added class: " << *ptr_class);//XXX
	}


	/**
	 * \brief Add a (possibly new) node to this network.
	 *
	 * If the node already exists, the old one is overwritten.
	 */
	public: void add_node(node_pointer const& ptr_node)
	{
		DCS_DEBUG_TRACE_L(5, "(" << this << ") Adding node: " << *ptr_node << " - (" << ptr_node << ")");//XXX

		// precondition: ptr_node is not null
		DCS_ASSERT(
			ptr_node,
			::std::invalid_argument("[dcs::des::model::qn::queueing_network::add_node] Pointer to network node is null.")
		);

//		//TODO: make sure that node_id_ does not overflow

		// Test if a valid ID has already been assigned
		if (ptr_node->id() == invalid_node_id)
		{
//			ptr_node->id(node_id_++);
			ptr_node->id(nodes_.size());
		}
//		else
//		{
//			// Advance the global id
//			if (node_id_ < ptr_node->id())
//			{
//				node_id_ = ptr_node->id() + 1;
//			}
//		}

		// Set a reference back to this network
		ptr_node->network(this);

		// Finally, make sure there is enough space...
		if (ptr_node->id() >= nodes_.size())
		{
			nodes_.resize(ptr_node->id()+1);
		}

		// ... And add the node.
		nodes_[ptr_node->id()] = ptr_node;

		DCS_DEBUG_TRACE_L(5, "(" << this << ") Added node: " << *ptr_node);//XXX
	}


	/**
	 * \brief Retrieve the node associated to the given identifier.
	 * \param id The identifier of the wanted node.
	 * \return The node associated to the identifier \a id.
	 * \exception std::logic_error No node associated to the given identifier.
	 */
	public: node_type const& get_node(node_identifier_type id) const
	{
		// pre: user must pass a valid node identifier
		if (!check_node(id))
		{
			DCS_DEBUG_TRACE_L(3, "(" << this << ") Unable to get node for ID: " << id);
			throw ::std::logic_error("[dcs::des::model::qn::get_node] No node associated to the given identifier.");
		}

		return *nodes_[id];
	}


	/**
	 * \brief Retrieve the node associated to the given identifier.
	 * \param id The identifier of the wanted node.
	 * \return The node associated to the identifier \a id.
	 * \exception std::logic_error No node associated to the given identifier.
	 */
	public: node_type& get_node(node_identifier_type id)
	{
		// pre: user must pass a valid node identifier
		if (!check_node(id))
		{
			DCS_DEBUG_TRACE_L(3, "(" << this << ") Unable to get node for ID: " << id);
			throw ::std::logic_error("[dcs::des::model::qn::get_node] No node associated to the given identifier.");
		}

		return *nodes_[id];
	}


	/**
	 * \brief Retrieve the class associated to the given identifier.
	 * \param id The identifier of the wanted class.
	 * \return The class associated to the identifier \a id.
	 * \exception std::logic_error No class associated to the given identifier.
	 */
	public: class_type const& get_class(class_identifier_type id) const
	{
		// pre: user must pass a valid class identifier
		if (!check_class(id))
		{
			DCS_DEBUG_TRACE_L(3, "(" << this << ") Unable to get class for ID: " << id);

			throw ::std::logic_error("[dcs::des::model::qn::get_class] No class associated to the given identifier.");
		}

		return *classes_[id];
	}


	/**
	 * \brief Retrieve the class associated to the given identifier.
	 * \param id The identifier of the wanted class.
	 * \return The class associated to the identifier \a id.
	 * \exception std::logic_error No class associated to the given identifier.
	 */
	public: class_type& get_class(class_identifier_type id)
	{
		// pre: user must pass a valid class identifier
		if (!check_class(id))
		{
			DCS_DEBUG_TRACE_L(3, "(" << this << ") Unable to get class for ID: " << id);

			throw ::std::logic_error("[dcs::des::model::qn::get_class] No class associated to the given identifier.");
		}

		return *classes_[id];
	}


	/// Return the number of customer classes.
	public: class_size_type num_classes() const
	{
		return classes_.size();
	}


	/// Return the number of nodes.
	public: node_size_type num_nodes() const
	{
		return nodes_.size();
	}


	/// Return the DES engine.
	public: engine_type const& engine() const
	{
		// pre: DES engine pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_eng_ );

		return *ptr_eng_;
	}


	/// Return the DES engine.
	public: engine_type& engine()
	{
		// pre: DES engine pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_eng_ );

		return *ptr_eng_;
	}


	/// Return the random number generator.
	public: random_generator_type const& random_generator() const
	{
		// pre: random number generator pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_rng_ );

		return *ptr_rng_;
	}


	/// Return the random number generator.
	public: random_generator_type& random_generator()
	{
		// pre: random number generator pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_rng_ );

		return *ptr_rng_;
	}


	public: customer_identifier_type generate_customer_id()
	{
		return next_customer_id_++;
	}


	/// Return the event source for the NETWORK-ARRIVAL event.
	public: event_source_type const& arrival_event_source() const
	{
		// pre: event source pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_arr_evt_src_ );

		return *ptr_arr_evt_src_;
	}


	/// Return the event source for the NETWORK-ARRIVAL event.
	public: event_source_type& arrival_event_source()
	{
		// pre: event source pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_arr_evt_src_ );

		return *ptr_arr_evt_src_;
	}


	/// Return the event source for the NETWORK-DEPARTURE event.
	public: event_source_type const& departure_event_source() const
	{
		// pre: event source pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_dep_evt_src_ );

		return *ptr_dep_evt_src_;
	}


	/// Return the event source for the NETWORK-DEPARTURE event.
	public: event_source_type& departure_event_source()
	{
		// pre: event source pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_dep_evt_src_ );

		return *ptr_dep_evt_src_;
	}


	/// Return the event source for the NETWORK-DISCARD event.
	public: event_source_type const& discard_event_source() const
	{
		// pre: event source pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_dis_evt_src_ );

		return *ptr_dis_evt_src_;
	}


	/// Return the event source for the NETWORK-DISCARD event.
	public: event_source_type& discard_event_source()
	{
		// pre: event source pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_dis_evt_src_ );

		return *ptr_dis_evt_src_;
	}


	/**
	 * \brief Schedule the arrival to the network of the given customer at
	 *  the given time delay.
	 * \param ptr_customer Pointer to the arriving customer.
	 * \param delay Time offset with respect to the current simulated time.
	 */
	public: void schedule_arrival(customer_pointer const& ptr_customer, real_type delay)
	{
		// pre: customer pointer must be a valid pointer.
		DCS_ASSERT(
			ptr_customer,
			throw ::std::invalid_argument("[dcs::des::model::qn::queueing_network::schedule_arrival] Invalid customer.")
		);
		// pre: delay >= 0
		DCS_ASSERT(
			delay >= 0,
			throw ::std::invalid_argument("[dcs::des::model::qn::queueing_network::schedule_arrival] Delay must be a non-negative value.")
		);
		// pre: simulator engine pointer must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_eng_ );
		// pre: arrival event source must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_arr_evt_src_ );

		DCS_DEBUG_TRACE_L(3, "(" << this << ") Begin Scheduling NETWORK-ARRIVAL event for Customer: " << *ptr_customer << " at Delay: " << delay << " (Clock: " << ptr_eng_->simulated_time() << ")");//XXX

		ptr_eng_->schedule_event(
				ptr_arr_evt_src_,
				ptr_eng_->simulated_time()+delay,
				ptr_customer
		);

		DCS_DEBUG_TRACE_L(3, "(" << this << ") End scheduling NETWORK-ARRIVAL event for Customer: " << *ptr_customer << " at Delay: " << delay << " (Clock: " << ptr_eng_->simulated_time() << ")");//XXX
	}


	/**
	 * \brief Schedule the departure from the network of the given customer at
	 *  the given time delay.
	 * \param ptr_customer Pointer to the departing customer.
	 * \param delay Time offset with respect to the current simulated time.
	 */
	public: void schedule_departure(customer_pointer const& ptr_customer, real_type delay)
	{
		// pre: customer pointer must be a valid pointer.
		DCS_ASSERT(
			ptr_customer,
			throw ::std::invalid_argument("[dcs::des::model::qn::queueing_network::schedule_departure] Invalid customer.")
		);
		// pre: delay >= 0
		DCS_ASSERT(
			delay >= 0,
			throw ::std::invalid_argument("[dcs::des::model::qn::queueing_network::schedule_departure] Delay must be a non-negative value.")
		);
		// pre: simulator engine pointer must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_eng_ );
		// pre: departure event source must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_dep_evt_src_ );

		DCS_DEBUG_TRACE_L(3, "(" << this << ") Begin Scheduling NETWORK-DEPARTURE event for Customer: " << *ptr_customer << " at Delay: " << delay << " (Clock: " << ptr_eng_->simulated_time() << ")");//XXX

		ptr_eng_->schedule_event(
				ptr_dep_evt_src_,
				ptr_eng_->simulated_time()+delay,
				ptr_customer
		);

		DCS_DEBUG_TRACE_L(3, "(" << this << ") End Scheduling NETWORK-DEPARTURE event for Customer: " << *ptr_customer << " at Delay: " << delay << " (Clock: " << ptr_eng_->simulated_time() << ")");//XXX
	}


	/**
	 * \brief Schedule the discard from the network of the given customer at
	 *  the given time delay.
	 * \param ptr_customer Pointer to the discarded customer.
	 * \param delay Time offset with respect to the current simulated time.
	 */
	public: void schedule_discard(customer_pointer const& ptr_customer, real_type delay)
	{
		// pre: customer pointer must be a valid pointer.
		DCS_ASSERT(
			ptr_customer,
			throw ::std::invalid_argument("[dcs::des::model::qn::queueing_network::schedule_discard] Invalid customer.")
		);
		// pre: delay >= 0
		DCS_ASSERT(
			delay >= 0,
			throw ::std::invalid_argument("[dcs::des::model::qn::queueing_network::schedule_discard] Delay must be a non-negative value.")
		);
		// pre: simulator engine pointer must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_eng_ );
		// pre: discard event source must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_dis_evt_src_ );

		DCS_DEBUG_TRACE_L(3, "(" << this << ") Begin Scheduling NETWORK-DISCARD event for Customer: " << *ptr_customer << " at Delay: " << delay << " (Clock: " << ptr_eng_->simulated_time() << ")");//XXX

		ptr_eng_->schedule_event(
				ptr_dis_evt_src_,
				ptr_eng_->simulated_time()+delay,
				ptr_customer
		);

		DCS_DEBUG_TRACE_L(3, "(" << this << ") End Scheduling NETWORK-DISCARD event for Customer: " << *ptr_customer << " at Delay: " << delay << " (Clock: " << ptr_eng_->simulated_time() << ")");//XXX
	}


	/// Return the overall number of arrived customers.
	public: uint_type num_arrivals() const
	{
		return narr_;
	}


	/// Return the overall number of (successfully) departed customers.
	public: uint_type num_departures() const
	{
		return ndep_;
	}


	/// Return the overall number of discarded customers.
	public: uint_type num_discards() const
	{
		return ndis_;
	}


	public: void statistic(network_output_statistic_category category, output_statistic_pointer const& ptr_stat)
	{
		DCS_DEBUG_TRACE_L(5, "(" << this << ") BEGIN Adding statistic: " << *ptr_stat << " - (" << ptr_stat << ") for category: " << category);//XXX

		// pre: statistic pointer must be a valid pointer.
		DCS_ASSERT(
			ptr_stat,
			throw ::std::invalid_argument("[dcs::des::model::qn::queueing_network::statistic] Invalid statistic.")
		);

//		ensure_space_stat(category);

		stats_[category].push_back(ptr_stat);
	//	stats_[category].insert(ptr_stat->category(), ptr_stat);//TODO

		DCS_DEBUG_TRACE_L(5, "(" << this << ") END Adding statistic: " << *ptr_stat << ") for category: " << category);//XXX
	}


	public: ::std::vector<output_statistic_pointer> statistic(network_output_statistic_category category) const
	{
		// pre: existent statistic
		if (!check_stat(category))
		{
			throw ::std::logic_error("[dcs::des::model::qn::statistic] No statistic associated to the given category.");
		}

		return stats_.at(category);
	}


	//@{ dcs::des::entity implementation


	private: void do_enable(bool flag)
	{
		// Enable/Disable event sources
		{
			ptr_arr_evt_src_->enable(flag);
			ptr_dep_evt_src_->enable(flag);
			ptr_dis_evt_src_->enable(flag);
		}

		// Enable/Disable stats
		{
			typedef typename output_statistic_category_container::iterator cat_stat_iterator;
			typedef typename output_statistic_container::iterator stat_iterator;

			cat_stat_iterator cat_stat_end_it(stats_.end());
			for (cat_stat_iterator cat_stat_it = stats_.begin(); cat_stat_it != cat_stat_end_it; ++cat_stat_it)
			{
				stat_iterator stat_end_it(cat_stat_it->second.end());
				for (stat_iterator stat_it = cat_stat_it->second.begin(); stat_it != stat_end_it; ++stat_it)
				{
					(*stat_it)->enable(flag);
				}
			}
		}

		// Enable/Disable nodes
		{
			typedef typename node_container::iterator iterator;
			iterator end = nodes_.end();
			for (iterator it = nodes_.begin(); it != end; ++it)
			{
				node_pointer ptr_node(*it);

				// check: paranoid check
				DCS_DEBUG_ASSERT( ptr_node );

				ptr_node->enable(flag);
			}
		}

//		if (flag && !this->enabled())
//		{
////			initialize_simulation();
//			initialize_experiment();
//		}

//		if (flag)
//		{
//			if (!this->enabled())
//			{
//				connect_to_event_sources();
//				schedule_node_arrivals();
//			}
//		}
//		else
//		{
//			if (this->enabled())
//			{
//				disconnect_from_event_sources();
//			}
//		}

		if (flag && !this->enabled())
		{
			schedule_node_arrivals();
		}
	}


	//@} dcs::des::entity implementation


	//TODO
	//	public: output_statistic_pointer statistic(network_output_statistic_category category, statistic_category subcategory) const
	//	{
	//		if (!check_stat(category))
	//		{
	//			throw ::std::logic_error("[dcs::des::model::qn::statistic] No statistic associated to the given category.");
	//		}
	//
	//		return stats_.at(category).at(subcategory);
	//	}


	/// Initialization during the object creation.
	private: void init()
	{
//		if (this->enabled())
//		{
			connect_to_event_sources();
//		}
	}


	private: void finit()
	{
//		if (this->enabled())
//		{
			disconnect_from_event_sources();
//		}
	}


	private: void connect_to_event_sources()
	{
		// pre: arrival event source pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_arr_evt_src_ );
		// pre: departure event source pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_dep_evt_src_ );
		// pre: discard event source pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_dis_evt_src_ );
		// pre: DES engine pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_eng_ );

		// Connect to the local event sources

		ptr_arr_evt_src_->connect(
			::dcs::functional::bind(
				&self_type::process_arrival,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
		ptr_dep_evt_src_->connect(
			::dcs::functional::bind(
				&self_type::process_departure,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
		ptr_dis_evt_src_->connect(
			::dcs::functional::bind(
				&self_type::process_discard,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);

		// Connect to foreign event sources.

		ptr_eng_->begin_of_sim_event_source().connect(
			::dcs::functional::bind(
				&self_type::process_begin_of_sim,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
		ptr_eng_->system_initialization_event_source().connect(
			::dcs::functional::bind(
				&self_type::process_sys_init,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
		ptr_eng_->system_finalization_event_source().connect(
			::dcs::functional::bind(
				&self_type::process_sys_finit,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
	}


	private: void disconnect_from_event_sources()
	{
		// pre: arrival event source pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_arr_evt_src_ );
		// pre: departure event source pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_dep_evt_src_ );
		// pre: discard event source pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_dis_evt_src_ );
		// pre: DES engine pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_eng_ );

		// Disconnect from local event sources

		ptr_arr_evt_src_->disconnect(
			::dcs::functional::bind(
				&self_type::process_arrival,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
		ptr_dep_evt_src_->disconnect(
			::dcs::functional::bind(
				&self_type::process_departure,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
		ptr_dis_evt_src_->disconnect(
			::dcs::functional::bind(
				&self_type::process_discard,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);

		// Disconnect from foreign event sources

		ptr_eng_->system_finalization_event_source().disconnect(
			::dcs::functional::bind(
				&self_type::process_sys_finit,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
		ptr_eng_->system_initialization_event_source().disconnect(
			::dcs::functional::bind(
				&self_type::process_sys_init,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
		ptr_eng_->begin_of_sim_event_source().disconnect(
			::dcs::functional::bind(
				&self_type::process_begin_of_sim,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
	}


	/// Check if the given identifier is a valid node identifier.
	private: bool check_node(node_identifier_type id) const
	{
		return id != invalid_node_id // Node identifier is a valid identifier
			   && id < nodes_.size() // Node identifier <= max selected # nodes
			   && nodes_[id]; // The stored info is a valid node pointer
	}


	/// Check if the given identifier is a valid class identifier.
	private: bool check_class(class_identifier_type id) const
	{
		return id != invalid_class_id
			   && id < classes_.size()
			   && classes_[id];
	}


	/// Check if the given statistic category is valid.
	private: bool check_stat(network_output_statistic_category category) const
	{
		typedef typename output_statistic_category_container::size_type size_type;

//		return stats_.size() > static_cast<size_type>(category)
//				   && stats_[category].size() > 0;
		return stats_.count(category) > 0
			   && !stats_.at(category).empty();
	}


//	/// Check if the statistics container has enough space for the given
//	/// statistic category.
//	private: void ensure_space_stat(network_output_statistic_category category)
//	{
//		typedef typename output_statistic_category_container::size_type size_type;
//
//		if (stats_.size() <= static_cast<size_type>(category))
//		{
//			stats_.resize(static_cast<size_type>(category)+1);
//		}
//	}


	/// Accumulate the given value for all the statistics associated to the
	/// given category.
	private: void accumulate_stat(network_output_statistic_category category, real_type value)
	{
		DCS_DEBUG_TRACE_L(5, "(" << this << ") BEGIN Accumulating statistic value: " << value << " for category: " << category);//XXX

		// Make sure that some statistic for this category has been registered.
		if (!check_stat(category))
		{
			DCS_DEBUG_TRACE_L(5, "Statistic not requested.");//XXX
			return;
		}

//		::std::for_each(
//			stats_[category].begin(),
//			stats_[category].end(),
//			::dcs::functional::bind(
//				&output_statistic_type::operator(),
//				::dcs::functional::placeholders::_1,
//				value
//			)
//		);
		typedef typename output_statistic_container::iterator stats_iterator;
		stats_iterator stat_end_it(stats_[category].end());
		for (stats_iterator stat_it = stats_[category].begin(); stat_it != stat_end_it; ++stat_it)
		{
			output_statistic_pointer ptr_stat(*stat_it);

			(*ptr_stat)(value);
		}

		DCS_DEBUG_TRACE_L(5, "(" << this << ") END Accumulating statistic value: " << value << " for category: " << category);//XXX
	}


	/// Reset all the statistics associated to the given category.
	private: void reset_stat(network_output_statistic_category category)
	{
		DCS_DEBUG_TRACE_L(5, "(" << this << ") BEGIN Resetting statistic for category: " << category);//XXX

		// Make sure that some statistic for this category has been registered.
		if (!check_stat(category))
		{
			return;
		}

//		ensure_space_stat(category);

		::std::for_each(
			stats_[category].begin(),
			stats_[category].end(),
			::dcs::functional::bind(
				&output_statistic_type::reset,
				::dcs::functional::placeholders::_1
			)
		);

		DCS_DEBUG_TRACE_L(5, "(" << this << ") END Resetting statistic for category: " << category);//XXX
	}


	private: void initialize_simulation()
	{
		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Initializing simulation.");//XXX

		// Reset simulation-level stats
		typename output_statistic_category_container::const_iterator stat_end_it = stats_.end();
		for (typename output_statistic_category_container::const_iterator it = stats_.begin();
			 it != stat_end_it;
			 ++it)
		{
			reset_stat(it->first);
		}

		// Reset nodes
		{
			typedef typename node_container::iterator iterator;
			iterator end = nodes_.end();
			for (iterator it = nodes_.begin(); it != end; ++it)
			{
				node_pointer ptr_node = *it;

				// Pointer to node must be a valid pointer.
				DCS_DEBUG_ASSERT( ptr_node );

				ptr_node->initialize_simulation();
			}
		}

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Initializing simulation.");//XXX
	}


	private: void initialize_experiment()
	{
		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Initializing experiment.");//XXX

		// Reset experiment-level stats
		narr_ = ndep_
			  = ndis_
			  = uint_type/*zero*/();

		// Reset nodes
		{
			typedef typename node_container::iterator iterator;
			iterator end = nodes_.end();
			for (iterator it = nodes_.begin(); it != end; ++it)
			{
				node_pointer ptr_node = *it;

				// Pointer to node must be a valid pointer.
				DCS_DEBUG_ASSERT( ptr_node );

				ptr_node->initialize_experiment();
			}
		}

		// Generate arrivals for each customer class:
		// - reset the customer identifier generator
		// - for each open class, generate a single arrival
		// - for each closed class, generate n arrivals, with n being the size
		//   of the population for this class.

		next_customer_id_ = customer_identifier_type/*zero*/();

//		// For each source/population node, schedule an arrival event
//		{
//			typedef typename class_container::const_iterator iterator;
//			iterator end = classes_.end();
//			for (iterator it = classes_.begin(); it != end; ++it)
//			{
//				class_pointer ptr_class = *it;
//
//				// Pointer to customer class must be a valid pointer.
//				DCS_DEBUG_ASSERT( ptr_class );
//
//				customer_pointer ptr_customer = ptr_class->make_customer();
//
//				// Pointer to customer must be a valid pointer.
//				DCS_DEBUG_ASSERT( ptr_customer );
//
//				// Reference node must be a valid node.
//				DCS_DEBUG_ASSERT( this->check_node(ptr_class->reference_node()) );
//
//				DCS_DEBUG_TRACE_L(5, "(" << this << ") Sending Customer: " << *ptr_customer << " to Node: " << *(nodes_[ptr_class->reference_node()]));//XXX
//
//				nodes_[ptr_class->reference_node()]->receive(ptr_customer, real_type(0));
//			}
//		}
		if (this->enabled())
		{
			schedule_node_arrivals();
		}

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Initializing experiment.");//XXX
	}


	private: void finalize_experiment()
	{
		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Finalizing experiment.");//XXX

		real_type sim_time(ptr_eng_->simulated_time());

		accumulate_stat(net_throughput_statistic_category, static_cast<real_type>(ndep_)/sim_time);
		accumulate_stat(net_num_arrivals_statistic_category, narr_);
		accumulate_stat(net_num_departures_statistic_category, ndep_);

		// Finalize nodes
		{
			typedef typename node_container::iterator iterator;
			iterator end = nodes_.end();
			for (iterator it = nodes_.begin(); it != end; ++it)
			{
				node_pointer ptr_node = *it;

				// Pointer to node must be a valid pointer.
				DCS_DEBUG_ASSERT( ptr_node );

				ptr_node->finalize_experiment();
			}
		}

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Finalizing experiment.");//XXX
	}


	private: void schedule_node_arrivals()
	{
		// For each source/population node, schedule an arrival event
		typedef typename class_container::const_iterator iterator;
		iterator end = classes_.end();
		for (iterator it = classes_.begin(); it != end; ++it)
		{
			class_pointer ptr_class = *it;

			// Pointer to customer class must be a valid pointer.
			DCS_DEBUG_ASSERT( ptr_class );

			customer_pointer ptr_customer = ptr_class->make_customer();

			// Pointer to customer must be a valid pointer.
			DCS_DEBUG_ASSERT( ptr_customer );

			// Reference node must be a valid node.
			DCS_DEBUG_ASSERT( this->check_node(ptr_class->reference_node()) );

			DCS_DEBUG_TRACE_L(5, "(" << this << ") Sending Customer: " << *ptr_customer << " to Node: " << *(nodes_[ptr_class->reference_node()]));//XXX

			nodes_[ptr_class->reference_node()]->receive(ptr_customer, real_type(0));
		}
	}


	//@{ Event Handlers


	/// Handler for the BEGIN-OF-SIMULATION event.
	private: void process_begin_of_sim(event_type const& evt, engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Processing BEGIN-OF-SIMULATION (Clock: " << ctx.simulated_time() << ").");//XXX

//		if (this->enabled())
//		{
//			initialize_simulation();
//		}
		initialize_simulation();

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Processing of BEGIN-OF-SIMULATION (Clock: " << ctx.simulated_time() << ").");//XXX
	}


	/// Handler for the SYSTEM-INITIALIZATION event.
	private: void process_sys_init(event_type const& evt, engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Processing SYSTEM-INITIALIZATION (Clock: " << ctx.simulated_time() << ").");//XXX

//		if (this->enabled())
//		{
////			initialize_simulation();
			initialize_experiment();
//		}

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Processing of SYSTEM-INITIALIZATION (Clock: " << ctx.simulated_time() << ").");//XXX
	}


	/// Handler for the SYSTEM-FINALIZATION event.
	private: void process_sys_finit(event_type const& evt, engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Processing SYSTEM-FINALIZATION (Clock: " << ctx.simulated_time() << ").");//XXX

//		if (this->enabled())
//		{
			finalize_experiment();
//		}

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Processing of SYSTEM-FINALIZATION (Clock: " << ctx.simulated_time() << ").");//XXX
	}


	/// Handler for the NETWORK-ARRIVAL event.
	private: void process_arrival(event_type const& evt, engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Processing NETWORK-ARRIVAL - Customer: " << *(evt.template unfolded_state<customer_pointer>()) << " (Clock: " << ctx.simulated_time() << ").");//XXX

		++narr_;

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Processing NETWORK-ARRIVAL - Customer: " << *(evt.template unfolded_state<customer_pointer>()) << " (Clock: " << ctx.simulated_time() << ").");//XXX
	}


	/// Handler for the NETWORK-DEPARTURE event.
	private: void process_departure(event_type const& evt, engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Processing NETWORK-DEPARTURE - Customer: " << *(evt.template unfolded_state<customer_pointer>()) << " (Clock: " << ctx.simulated_time() << ").");//XXX

		customer_pointer ptr_customer = evt.template unfolded_state<customer_pointer>();

		// check: customer pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_customer );

		// Update customer info
		ptr_customer->departure_time(ctx.simulated_time());
		ptr_customer->status(customer_type::died_status);

		/// Update statistics

		++ndep_;
//		accumulate_stat(net_throughput_statistic_category,
//						ndep_/ctx.simulated_time());
		accumulate_stat(net_response_time_statistic_category,
						ctx.simulated_time() - ptr_customer->arrival_time());

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Processing NETWORK-DEPARTURE - Customer: " << *(evt.template unfolded_state<customer_pointer>()) << " (Clock: " << ctx.simulated_time() << ").");//XXX
	}


	/// Handler for the NETWORK-DISCARD event.
	private: void process_discard(event_type const& evt, engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Processing NETWORK-DISCARD - Customer: " << *(evt.template unfolded_state<customer_pointer>()) << " (Clock: " << ctx.simulated_time() << ").");//XXX

		++ndis_;

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Processing NETWORK-DISCARD - Customer: " << *(evt.template unfolded_state<customer_pointer>()) << " (Clock: " << ctx.simulated_time() << ").");//XXX
	}


	//@} Event Handlers


	//@} Member functions


	//@{ Data members


	/// Container for customer classes.
	private: class_container classes_;
	/// Container for network nodes.
	private: node_container nodes_;
	/// Pointer to the random number generator.
	private: random_generator_pointer ptr_rng_;
	/// Pointer to the DES engine.
	private: engine_pointer ptr_eng_;
	/// The next available customer identifier.
	private: customer_identifier_type next_customer_id_;
	/// NETWORK-ARRIVAL event source: arrival of a customer at the network
	private: event_source_pointer ptr_arr_evt_src_;
	/// NETWORK-DEPARTURE event source: departure of a customer from the network
	private: event_source_pointer ptr_dep_evt_src_;
	/// NETWORK-DISCARD event source: discard of a customer from the network
	private: event_source_pointer ptr_dis_evt_src_;
	/// The overall number of arrived customers.
	private: uint_type narr_;
	/// The overall number of (successully) departed customers.
	private: uint_type ndep_;
	/// The overall number of discarded customers.
	private: uint_type ndis_;
	/// Output statistics grouped by their category.
	private: output_statistic_category_container stats_;


	//@} Data members
}; // queueing_network


template <
	typename UIntT,
	typename RealT,
	typename UniformRandomGeneratorT,
	typename DesEngineT
>
const typename queueing_network<UIntT,RealT,UniformRandomGeneratorT,DesEngineT>::class_identifier_type queueing_network<UIntT,RealT,UniformRandomGeneratorT,DesEngineT>::invalid_class_id = ::std::numeric_limits<typename queueing_network<UIntT,RealT,UniformRandomGeneratorT,DesEngineT>::class_identifier_type>::max();


template <
	typename UIntT,
	typename RealT,
	typename UniformRandomGeneratorT,
	typename DesEngineT
>
const typename queueing_network<UIntT,RealT,UniformRandomGeneratorT,DesEngineT>::node_identifier_type queueing_network<UIntT,RealT,UniformRandomGeneratorT,DesEngineT>::invalid_node_id = ::std::numeric_limits<typename queueing_network<UIntT,RealT,UniformRandomGeneratorT,DesEngineT>::node_identifier_type>::max();


template <
	typename UIntT,
	typename RealT,
	typename UniformRandomGeneratorT,
	typename DesEngineT
>
const ::std::string queueing_network<UIntT,RealT,UniformRandomGeneratorT,DesEngineT>::arrival_event_source_name("Arrival to Network");


template <
	typename UIntT,
	typename RealT,
	typename UniformRandomGeneratorT,
	typename DesEngineT
>
const ::std::string queueing_network<UIntT,RealT,UniformRandomGeneratorT,DesEngineT>::departure_event_source_name("Departure from Network");


template <
	typename UIntT,
	typename RealT,
	typename UniformRandomGeneratorT,
	typename DesEngineT
>
const ::std::string queueing_network<UIntT,RealT,UniformRandomGeneratorT,DesEngineT>::discard_event_source_name("Discard from Network");

}}}} // Namespace dcs::des::model::qn


#endif // DCS_DES_MODEL_QN_QUEUEING_NETWORK_HPP
