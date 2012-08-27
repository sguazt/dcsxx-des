/**
 * \file dcs/des/model/queue/lifo_queue_policy.hpp
 *
 * \brief Last-In First-Out (LIFO) queue policy
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

#ifndef DCS_DES_MODEL_LIFO_QUEUE_POLICY_HPP
#define DCS_DES_MODEL_LIFO_QUEUE_POLICY_HPP


#include <stack>


namespace dcs { namespace des { namespace model {

/**
 * \brief Last-In First-Out (LIFO) queue policy
 *
 * \tparam ValueT The type of queue elements.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename ValueT>
class lifo_queue_policy
{
	private: typedef ::std::stack<ValueT> container_type;
	public: typedef typename container_type::value_type value_type;
	public: typedef typename container_type::reference reference;
	public: typedef typename container_type::const_reference const_reference;
	public: typedef typename container_type::size_type size_type;


	// Compiler generated default-/copy-ctors and copy assignement are fine.


	public: void push(value_type const& value)
	{
		c_.push(value);
	}


	public: void pop()
	{
		c_.pop();
	}


	public: reference next()
	{
		return c_.top();
	}


	public: const_reference next() const
	{
		return c_.top();
	}


	public: bool empty() const
	{
		return c_.empty();
	}


	public: size_type size() const
	{
		return c_.size();
	}


	template <typename V>
	friend bool operator==(lifo_queue_policy<V> const& x, lifo_queue_policy<V> const& y)
	{
		return x.c_ == y.c_;
	}


	template <typename V>
	friend bool operator!=(lifo_queue_policy<V> const& x, lifo_queue_policy<V> const& y)
	{
		// Based on operator==
		return !(x == y);
	}


	template <typename V>
	friend bool operator<(lifo_queue_policy<V> const& x, lifo_queue_policy<V> const& y)
	{
		return x.c_ < y.c_;
	}


	template <typename V>
	friend bool operator>(lifo_queue_policy<V> const& x, lifo_queue_policy<V> const& y)
	{
		// Based on operator<
		return y < x;
	}


	template <typename V>
	friend bool operator<=(lifo_queue_policy<V> const& x, lifo_queue_policy<V> const& y)
	{
		// Based on operator<
		return !(y < x);
	}


	template <typename V>
	friend bool operator>=(lifo_queue_policy<V> const& x, lifo_queue_policy<V> const& y)
	{
		// Based on operator<
		return !(x < y);
	}


	/// The internal container.
	private: container_type c_;
};

}}} // Namespace dcs::des::model


#endif // DCS_DES_MODEL_LIFO_QUEUE_POLICY_HPP
