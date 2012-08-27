/**
 * \file dcs/des/engine_context.hpp
 *
 * \brief Context passed to simulation event target objects in order to
 *  communicate with the simulation engine.
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

#ifndef DCS_DES_ENGINE_CONTEXT_HPP
#define DCS_DES_ENGINE_CONTEXT_HPP


#include <boost/smart_ptr.hpp>
#include <dcs/des/fwd.hpp>
#include <iostream>


namespace dcs { namespace des {

/**
 * \brief Context passed to simulation event target objects in order to
 *  communicate with the simulation engine.
 *
 * \tparam RealT The type for real numbers.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename RealT>
class engine_context
{
	public: typedef RealT real_type;
	public: typedef engine<RealT> engine_type;
	public: typedef event<RealT> event_type;
	public: typedef event_source<RealT> event_source_type;


	public: template <typename RT> friend ::std::ostream& operator<<(::std::ostream& os, engine_context<RT> const& ctx);


	public: engine_context(engine_type* ptr_eng)
		: ptr_eng_(ptr_eng)
	{
	}


	public: real_type total_time() const
	{
		return ptr_eng_->total_time();
	}


	// \deprecated Use simulated_time
	public: real_type sim_time() const
	{
		return ptr_eng_->simulated_time();
	}


	public: real_type simulated_time() const
	{
		return ptr_eng_->simulated_time();
	}


	public: real_type last_event_time() const
	{
		return ptr_eng_->last_event_time();
	}


	public: void schedule_event(::boost::shared_ptr<event_source_type> const& ptr_evt_src, real_type fire_time)
	{
		ptr_eng_->schedule_event(ptr_evt_src, fire_time);
	}


	public: template <typename T>
		void schedule_event(::boost::shared_ptr<event_source_type> const& ptr_evt_src, real_type fire_time, T const& state)
	{
		ptr_eng_->schedule_event(ptr_evt_src, fire_time, state);
	}

/*
	public: engine_type& engine()
	{
		return *ptr_eng_;
	}


	public: engine_type const& engine() const
	{
		return *ptr_eng_;
	}
*/


	private: engine_type* ptr_eng_;
}; // engine_context


template <typename RealT>
inline
::std::ostream& operator<<(::std::ostream& os, engine_context<RealT> const& ctx)
{
	os << "<(engine context) Simulator engine: " << *(ctx.ptr_eng_) << ">";

	return os;
}


}} // Namespace dcs::des


#endif // DCS_DES_ENGINE_CONTEXT_HPP
