/**
 * \file src/dcs/des/event.hpp
 *
 * \brief A simulation event.
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

#ifndef DCS_DES_EVENT_HPP
#define DCS_DES_EVENT_HPP


#include <dcs/des/event_source.hpp>
#include <dcs/des/fwd.hpp>
#include <boost/smart_ptr.hpp>
#include <dcs/type_traits/add_const.hpp>
#include <dcs/type_traits/add_reference.hpp>
#include <dcs/util/any.hpp>
#include <iostream>
#include <string>


namespace dcs { namespace des {

/**
 * \brief A simulation event.
 *
 * \tparam RealT The type for the fire time.
 *
 * Represent a simulation event which makes to advance the simulated clock.
 *
 * \author Cosimo Anglano, &lt;cosimo.anglano@mfn.unipmn.it&gt;
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename RealT=double>
class event
{
	//public: typedef unknown_base_event_tag tag;
	private: typedef event<RealT> self_type;
	public: typedef RealT real_type;
	public: typedef event_source<real_type> event_source_type;
	public: typedef engine_context<real_type> engine_context_type;
	public: typedef ::dcs::util::any state_type;


	//FIXME: let the creator of the event decide what ID to assigne
	private: static unsigned long next_id;


	/**
	 * \brief Create a new event with the given fire \a time.
	 * \param time The event fire time.
	 *
	 * \param time The event fire time.
	 */
	public: explicit event(::boost::shared_ptr<event_source_type> const& ptr_src, real_type sched_time=0, real_type fire_time=0, state_type const& state=state_type())
		: ptr_src_(ptr_src),
		  sched_time_(sched_time),
		  fire_time_(fire_time),
		  state_(state),
		  id_(next_id++)
	{
		// empty
	}


	// compiler generated copy-constructor and copy assigment are fine.

	public: event(event const& that)
	: ptr_src_(that.ptr_src_),
	  sched_time_(that.sched_time_),
	  fire_time_(that.fire_time_),
	  state_(that.state_),
	  id_(that.id_)
	  //id_(next_id++)
	{
		// FIXME: What to do with id_?
	}


	public: event& operator=(event const& rhs)
	{
		if (&rhs != this)
		{
			// FIXME: Will id_ change or not?

			ptr_src_ = rhs.ptr_src_;
			sched_time_ = rhs.sched_time_;
			fire_time_ = rhs.fire_time_;
			state_ = rhs.state_;
			id_ = rhs.id_;
		}

		return *this;
	}


	public: unsigned long id() const
	{
		return id_;
	}


	public: real_type schedule_time() const
	{
		return sched_time_;
	}


	public: void schedule_time(real_type time)
	{
		sched_time_ = time;
	}


	public: real_type fire_time() const
	{
		return fire_time_;
	}


	public: void fire_time(real_type time)
	{
		fire_time_ = time;
	}


	public: event_source_type const& source() const
	{
		return *ptr_src_;
	}


	public: void fire(engine_context_type& ctx)
	{
		ptr_src_->emit(*this, ctx);
	}


	public: state_type& state()
	{
		return state_;
	}


	public: state_type const& state() const
	{
		return state_;
	}


	public: template <typename T>
		typename ::dcs::type_traits::add_reference<T>::type unfolded_state()
	{
		typedef typename ::dcs::type_traits::add_reference<T>::type ref_type;

		return ::dcs::util::any_cast<ref_type>(state_);
	}


	public: template <typename T>
		typename ::dcs::type_traits::add_reference<typename ::dcs::type_traits::add_const<T>::type>::type unfolded_state() const
//	public: double unfolded_state() const
	{
		typedef typename ::dcs::type_traits::add_reference<
					typename ::dcs::type_traits::add_const<T>::type
				>::type const_ref_type;

		return ::dcs::util::any_cast<const_ref_type>(state_);
/*
		return ::dcs::util::any_cast<double>(state_);
*/
	}


	//@{ Member variables

	/// The event source.
	private: ::boost::shared_ptr<event_source_type> ptr_src_;
	/// The time this event has been scheduled.
	private: real_type sched_time_;
	/// The time this event is fired.
	private: real_type fire_time_;
	/// The event state.
	private: state_type state_;
	/// The event identifier
	private: unsigned long id_;

	//@} Member variables
};

template <typename RealT>
unsigned long event<RealT>::next_id = 0UL;


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, event<RealT> const& evt)
{
	os	<< "<"
		<< evt.id()
		<< ","
		<< evt.source()
		<< " @ "
		<< evt.schedule_time()
		<< " -> "
		<< evt.fire_time()
		<< ">";
	return os;
}


template <typename RealT>
bool operator<(event<RealT> const& x, event<RealT> const& y)
{
	return x.fire_time() < y.fire_time();
}


template <typename RealT>
bool operator==(event<RealT> const& x, event<RealT> const& y)
{
	return x.fire_time() == y.fire_time();
}


template <typename RealT>
bool operator<=(event<RealT> const& x, event<RealT> const& y)
{
	return x < y || x == y;
}


template <typename RealT>
bool operator>(event<RealT> const& x, event<RealT> const& y)
{
	return !(x <= y);
}


template <typename RealT>
bool operator>=(event<RealT> const& x, event<RealT> const& y)
{
	return !(x < y);
}


template <typename RealT>
bool operator!=(event<RealT> const& x, event<RealT> const& y)
{
	return !(x == y);
}

}} // dcs::des


#endif // DCS_DES_EVENT_HPP
