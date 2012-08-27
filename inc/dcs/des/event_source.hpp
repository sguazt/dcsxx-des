/**
 * \file dcs/des/event_source.hpp
 *
 * \brief Source of simulation events.
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

#ifndef DCS_DES_EVENT_SOURCE_HPP
#define DCS_DES_EVENT_SOURCE_HPP


#include <boost/signals2.hpp>
#include <boost/smart_ptr.hpp>
#include <cstddef>
#include <dcs/debug.hpp>
#include <dcs/des/fwd.hpp>
#include <dcs/functional/hash.hpp>
#include <iostream>
#include <sstream>
#include <string>


namespace dcs { namespace des {

namespace detail { namespace /*<unnamed>*/ {

/// Build an event source name from the event source ID.
template <typename UIntT>
inline ::std::string make_name(UIntT id)
{
	::std::ostringstream oss;
	oss << "Event Source " << id;

	return oss.str();
}


}} // Namespace detail::<unnamed>


/**
 * \brief Source of simulation events.
 *
 * \tparam RealT The type used for real numbers.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename RealT=double>
class event_source
{
	public: typedef  RealT real_type;
	public: typedef  unsigned long uint_type;
	private: typedef event<real_type> event_type;
	private: typedef engine_context<real_type> engine_context_type;
	private: typedef ::boost::signals2::signal<void (event_type const&, engine_context_type&)> signal_type;
	public: typedef typename signal_type::slot_type event_sink_type;
	public: typedef typename ::boost::signals2::connection connection_type;


	private: static uint_type counter_;


	/// Default constructor.
	public: event_source()
	: id_(++counter_),
	  name_(detail::make_name(id_)),
	  ptr_sig_(new signal_type()),
	  enabled_(true)
	{
		// empty
	}


	public: event_source(::std::string const& name)
		: id_(++counter_),
		  name_(name),
		  ptr_sig_(new signal_type()),
		  enabled_(true)
	{
		// empty
	}


	/// Copy constructor
	public: event_source(event_source const& that)
	: id_(++counter_), // non-copyable
	  name_(that.name_),
	  ptr_sig_(new signal_type()), // non-copyable
	  enabled_(that.enabled_)
	{
		// empty
	}


	/// Copy assignment
	public: event_source& operator=(event_source const& rhs)
	{
		if (&rhs != this)
		{
			id_ = ++counter_; // non-copyable
			name_ = rhs.name_;
			ptr_sig_ = ::boost::make_shared<signal_type>(); // non-copyable
			enabled_ = rhs.enabled_;
		}

		return *this;
	}


	public: uint_type id() const
	{
		return id_;
	}


	public: ::std::string const& name() const
	{
		return name_;
	}


	public: connection_type connect(event_sink_type const& sink)
	{
		return ptr_sig_->connect(sink);
	}


	public: template <typename S>
		void disconnect(S const& sink)
	{
		ptr_sig_->disconnect(sink);
	}


	public: void disconnect_all()
	{
		ptr_sig_->disconnect_all_slots();
	}


	public: void emit(event_type const& evt, engine_context_type& ctx)
	{
		if (enabled_)
		{
			(*ptr_sig_)(evt, ctx);
		}
	}


	public: bool empty() const
	{
		return ptr_sig_->empty();
	}


	public: ::std::size_t num_sinks() const
	{
		return ptr_sig_->num_slots();
	}


	public: bool enabled() const
	{
		return enabled_;
	}


	public: void enable(bool value)
	{
		enabled_ = value;
	}


	private: uint_type id_;
	private: ::std::string name_;
	private: ::boost::shared_ptr<signal_type> ptr_sig_;
	private: bool enabled_;
};


template <typename RealT>
typename event_source<RealT>::uint_type event_source<RealT>::counter_ = 0;


template <typename RealT>
inline bool operator==(event_source<RealT> const& x, event_source<RealT> const& y)
{
	return x.id() == y.id();
}


template <typename RealT>
inline bool operator!=(event_source<RealT> const& x, event_source<RealT> const& y)
{
	return !(x.id() == y.id());
}


template <typename RealT>
inline ::std::size_t hash_value(event_source<RealT> const& et)
{
	::dcs::functional::hash<typename event_source<RealT>::uint_type> hasher;
	return hasher(et.id());
}


template <
	typename CharT,
	typename CharTraits,
	typename RealT
>
inline ::std::basic_ostream<CharT,CharTraits>& operator<<(::std::basic_ostream<CharT,CharTraits>& os, event_source<RealT> const& source)
{
	return os << "<"
			  << source.id()
			  << " (" << source.name() << ")"
			  << ">";
}

}} // Namespace dcs::des


#endif // DCS_DES_EVENT_SOURCE_HPP
