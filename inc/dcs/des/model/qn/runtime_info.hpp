/**
 * \file dcs/des/model/qn/runtime_info.hpp
 *
 * \brief Runtime information associated to each served job.
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

#ifndef DCS_DES_MODEL_QN_RUNTIME_INFO_HPP
#define DCS_DES_MODEL_QN_RUNTIME_INFO_HPP


#include <boost/smart_ptr.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/model/qn/server_utilization_profile.hpp>
#include <dcs/des/model/qn/customer.hpp>
#include <dcs/math/traits/float.hpp>
#include <limits>


namespace dcs { namespace des { namespace model { namespace qn {

template <typename TraitsT>
class runtime_info
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef server_utilization_profile<real_type> utilization_profile_type;
	public: typedef customer<traits_type> customer_type;
	public: typedef ::boost::shared_ptr<customer_type> customer_pointer;


	public: runtime_info()
	: ptr_customer_(),
	  st_(0),
	  sd_(0),
	  wt_(0),
	  lwut_(st_),
	  share_(1),
	  mult_(1),
	  sid_(0)
	{
	}

	public: runtime_info(customer_pointer const& ptr_customer, real_type demand)
	: ptr_customer_(ptr_customer),
	  st_(0),
	  sd_(demand),
	  wt_(0),
	  lwut_(st_),
	  share_(1),
	  mult_(1),
	  sid_(0)
	{
	}

	public: runtime_info(customer_pointer const& ptr_customer, real_type st, real_type demand)
	: ptr_customer_(ptr_customer),
	  st_(st),
	  sd_(demand),
	  wt_(0),
	  lwut_(st_),
	  share_(1),
	  mult_(1),
	  sid_(0)
	{
	}


//	public: void runtime(real_type t)
//	{
//		DCS_DEBUG_ASSERT( t >= 0 );
//
//		rt_ = t;
//	}


	public: real_type service_demand() const
	{
		return sd_;
	}


	public: real_type runtime() const
	{
		return sd_/mult_;
	}


//	public: void start_time(real_type t)
//	{
//		DCS_DEBUG_ASSERT( t >= 0 );
//
//		st_ = t;
//	}


	public: real_type start_time() const
	{
		return st_;
	}


	/// Accumulate work by elapsed time from last work-update.
	public: void accumulate_work(real_type t)
	{
		DCS_DEBUG_ASSERT( t >= 0 );
		DCS_DEBUG_ASSERT( ::dcs::math::float_traits<real_type>::definitely_less_equal(wt_+(t-lwut_)/mult_, sd_) );

		wt_ += (t-lwut_)/mult_;
		lwut_ = t;
	}


	/// Accumulate the work done in an interval of time (the work will be scaled by current share)
	public: void accumulate_work_time(real_type w)
	{
		// NOTE: the use of epsilon() is to take care of cancellation errors due
		//       to floating-point subtraction.

DCS_DEBUG_TRACE("(" << this << ") Accumulating work time for Customer: " << ::std::setprecision(12) << get_customer() << " -> Work Time: " << w << " - Work: " << w/mult_ << " - Old Total work: " << wt_ << " - New Total work: " << (wt_+w/mult_) << " - Service Demand: " << sd_ << " - Condition: " << ::std::boolalpha << (wt_+w/mult_<=sd_) << " - Condition#2: " << ::std::boolalpha << ((wt_+w/mult_)<=sd_) << " - Condition#3: " << ::std::boolalpha << ((wt_+w/mult_)>sd_) << " - Difference: " << ((wt_+w/mult_)-sd_) << " - epsilon: " << ::std::numeric_limits<real_type>::epsilon());//XXX
		DCS_DEBUG_ASSERT( w >= 0 );
		//DCS_DEBUG_ASSERT( wt_+w/mult_ <= sd_ );
		DCS_DEBUG_ASSERT( ::dcs::math::float_traits<real_type>::definitely_less_equal(wt_+w/mult_, sd_) );

		wt_ += w/mult_;
		lwut_ = -1; // not available
	}


	/// Accumulate the effective amount of work done (the work will not be scaled)
	public: void accumulate_work2(real_type w)
	{
		// NOTE: the use of epsilon() is to take care of cancellation errors due
		//       to floating-point subtraction.

DCS_DEBUG_TRACE("(" << this << ") Accumulating work for Customer: " << ::std::setprecision(12) << get_customer() << " -> Work: " << w << " - Old Total work: " << wt_ << " - New Total work: " << (wt_+w) << " - Service Demand: " << sd_ << " - Condition: " << ::std::boolalpha << (wt_+w<=sd_) << " - Condition#2: " << ::std::boolalpha << ((wt_+w)<=sd_) << " - Condition#3: " << ::std::boolalpha << ((wt_+w)>sd_) << " - Difference: " << ((wt_+w)-sd_) << " - epsilon: " << ::std::numeric_limits<real_type>::epsilon());//XXX
		DCS_DEBUG_ASSERT( w >= 0 );
		//DCS_DEBUG_ASSERT( wt_+w <= sd_ );
		DCS_DEBUG_ASSERT( ::dcs::math::float_traits<real_type>::definitely_less_equal(wt_+w, sd_) );

		wt_ += w;
		lwut_ = -1; // not available
	}


	public: real_type completed_work() const
	{
		return wt_;
	}


	public: real_type residual_work() const
	{
		// NOTE: the use of epsilon() is to take care of cancellation errors due
		//       to floating-point subtraction.

		//DCS_DEBUG_ASSERT( sd_ >= wt_ );
		DCS_DEBUG_ASSERT( ::dcs::math::float_traits<real_type>::definitely_less_equal(wt_, sd_) );

		//return sd_-wt_;

		real_type rt(sd_-wt_);

		return ::dcs::math::float_traits<real_type>::definitely_greater(rt, static_cast<real_type>(0)) ? rt : 0;
	}


	public: void server_id(uint_type id)
	{
		sid_ = id;
	}


	public: uint_type server_id() const
	{
		return sid_;
	}


	public: customer_type const& get_customer() const
	{
		return *ptr_customer_;
	}


	public: customer_type& get_customer()
	{
		return *ptr_customer_;
	}


	public: void share(real_type share)
	{
		share_ = share;
	}


	public: real_type share() const
	{
		return share_;
	}


	public: void capacity_multiplier(real_type val)
	{
		mult_ = val;
	}


	public: real_type capacity_multiplier() const
	{
		return mult_;
	}


	public: void utilization_profile(utilization_profile_type const& profile)
	{
//		return u_prof_;
		ptr_customer_->node_utilization_profile(ptr_customer_->current_node(), profile);
	}


	public: utilization_profile_type const& utilization_profile() const
	{
//		return u_prof_;
		return ptr_customer_->node_utilization_profile();
	}


//	private: unsigned long cid_;
	private: customer_pointer ptr_customer_;
	/// Start time.
	private: real_type st_;
	/// Service demand
	private: real_type sd_;
	/// Working time.
	private: real_type wt_;
	/// Last work update time.
	private: real_type lwut_;
	/// The current resource share.
	private: real_type share_;
	/// The current resource capacity multiplier.
	private: real_type mult_;
	/// The current server ID.
	private: uint_type sid_;
//	private: utilization_profile_type u_prof_;
};

}}}} // Namespace dcs::des::model::qn


#endif // DCS_DES_MODEL_QN_RUNTIME_INFO_HPP
