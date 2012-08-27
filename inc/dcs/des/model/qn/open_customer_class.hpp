/**
 * \file dcs/des/model/qn/open_customer_class.hpp
 *
 * \brief Open customer class.
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

#ifndef DCS_DES_MODEL_QN_OPEN_CUSTOMER_CLASS_HPP
#define DCS_DES_MODEL_QN_OPEN_CUSTOMER_CLASS_HPP


#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/model/qn/customer_class.hpp>
#include <dcs/math/stats/distribution/any_distribution.hpp>
#include <dcs/math/stats/function/rand.hpp>
#include <stdexcept>
#include <string>


namespace dcs { namespace des { namespace model { namespace qn {

template <typename TraitsT>
class open_customer_class: public customer_class<TraitsT>
{
	public: typedef TraitsT traits_type;
	private: typedef customer_class<TraitsT> base_type;
	public: typedef typename base_type::identifier_type identifier_type;
	private: typedef typename traits_type::real_type real_type;
	public: typedef ::dcs::math::stats::any_distribution<real_type> distribution_type;
	private: typedef typename base_type::customer_type customer_type;
	private: typedef typename base_type::customer_pointer customer_pointer;


	public: template <typename DistributionT>
		open_customer_class(::std::string const& name, DistributionT distr)
	: base_type(name),
	  distr_(::dcs::math::stats::make_any_distribution<DistributionT>(distr))
	{
	}


	public: template <typename DistributionT>
		open_customer_class(identifier_type id, ::std::string const& name, DistributionT distr)
	: base_type(id, name),
	  distr_(::dcs::math::stats::make_any_distribution<DistributionT>(distr))
	{
	}


	// Compiler-generated copy-constructor, copy-assignment, and destructor
	// are fine.


//	public: void arrival_distribution(distribution_type const& distr)
//	{
//		arr_distr_ = distr;
//	}


//	public: template <typename DistributionT>
//		void interarrival_distribution(DistributionT distr)
//	{
//		distr_ = ::dcs::math::stats::make_any_distribution<DistributionT>(distr);
//	}


	public: distribution_type const& interarrival_distribution() const
	{
		return distr_;
	}


	private: customer_class_category do_category() const
	{
		return open_customer_class_category;
	}


	private: customer_pointer do_make_customer() const
	{
		DCS_DEBUG_TRACE_L(3, "(" << this << ") BEGIN Do Making Customer for Class: " << *this << ".");//XXX

		// precondition: class has already been associated to a network
		DCS_ASSERT(
			this->network_ptr(),
			throw ::std::logic_error("[dcs::des::model::qn::open_customer_class::do_make_customer] Missing queueing network association.")
		);
		// precondition: reference node is a valid node
		DCS_ASSERT(
			this->reference_node() != traits_type::invalid_node_id(),
			throw ::std::logic_error("[dcs::des::model::qn::open_customer_class::do_make_customer] Missing reference node.")
		);

		customer_pointer ptr_customer(
				new customer_type(
					this->network_ptr()->generate_customer_id(),
					this->id(),
					this->reference_node()
				)
			);

		// Generate interarrival time and set it up as the arrival time
		real_type iatime(0);
//		typename traits_type::random_generator_type& ref_rng = const_cast<typename traits_type::random_generator_type&>(this->network().random_generator());
		typename traits_type::random_generator_type& ref_rng = const_cast<typename traits_type::network_type&>(this->network()).random_generator();
//		typename traits_type::network_type& ref_net = const_cast<typename traits_type::network_type&>(this->network());
//		typename traits_type::random_generator_type& ref_rng = ref_net.random_generator();
		while ((iatime = ::dcs::math::stats::rand(distr_, ref_rng)) < 0) ;
		ptr_customer->arrival_time(iatime);

		DCS_DEBUG_TRACE_L(3, "Generated new interarrival time: " << iatime); //XXX

		DCS_DEBUG_TRACE_L(3, "(" << this << ") END Do Making Customer for Class: " << *this << ".");//XXX

		return ptr_customer;
	}


	private: distribution_type distr_;
};

}}}} // Namespace dcs::des::model::qn


#endif // DCS_DES_MODEL_QN_OPEN_CUSTOMER_CLASS_HPP
