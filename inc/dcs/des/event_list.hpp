/**
 * \file dcs/des/event_list.hpp
 *
 * \brief Event list.
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

#ifndef DCS_DES_EVENT_LIST_HPP
#define DCS_DES_EVENT_LIST_HPP


#include <algorithm>
#include <boost/smart_ptr.hpp>
#include <functional>
#include <list>
#include <queue>
#include <vector>


namespace dcs { namespace des {

namespace detail { namespace /*<unnamed>*/ {

/**
 * \brief Ordered linked list.
 *
 * \note If you want to keep insertion order against equal items you should
 *       still use the \c std::less comparator.
 *       The reason lies in the way the \c push method is implemented;
 *       specifically, it uses the \c std::list::merge operation which is
 *       performed in the following way:
 *       "The merging \c merge(x,cmp) is performed using two iterators: one to
 *        iterate through \c x and another one to keep the insertion point in
 *        the list object; During the iteration of \c x, if the current element
 *        in \c x compares less than the element at the current insertion point
 *        in the list object, the element is removed from \c x and inserted into
 *        that location, otherwise the insertion point is advanced.
 *        This operation is repeated until either end is reached, in which
 *        moment the remaining elements of \c x (if any) are moved to the end of
 *        the list object and the function returns."
 *       (source: http://www.cplusplus.com/reference/stl/list/merge/)
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <
	typename T,
	typename ComparatorT=::std::less<T>
>
class ordered_list
{
	private: typedef ::std::list<T> list_impl_type;
	public: typedef T value_type;
	public: typedef ComparatorT comparator_type;
	public: typedef typename list_impl_type::pointer pointer;
	public: typedef typename list_impl_type::const_pointer const_pointer;
	public: typedef typename list_impl_type::reference reference;
	public: typedef typename list_impl_type::const_reference const_reference;
	public: typedef typename list_impl_type::iterator iterator;
	public: typedef typename list_impl_type::const_iterator const_iterator;
	public: typedef typename list_impl_type::reverse_iterator reverse_iterator;
	public: typedef typename list_impl_type::const_reverse_iterator const_reverse_iterator;
	public: typedef typename list_impl_type::size_type size_type;
	public: typedef typename list_impl_type::difference_type difference_type;
	public: typedef typename list_impl_type::allocator_type allocator_type;


	public: ordered_list()
	{
		// empty
	}


	public: template <typename ForwardIterT>
		ordered_list(ForwardIterT first, ForwardIterT last)
//	: list_(first, last) // NO: no ordered insertion
	{
		while (first != last)
		{
			push(*first);
			++first;
		}
	}


	/// Insert the given element (w.r.t- the order defined by the comparator).
	public: void push(value_type const& x)
	{
		list_impl_type l(1, x);
		list_.merge(l, cmp_);
	}


	/// Remove the minimum element (w.r.t. the order defined by the comparator).
	public: void pop()
	{
		list_.pop_front();
	}


	/// Return the minimum element (w.r.t. the order defined by the comparator).
	public: value_type const& top() const
	{
		return list_.front();
	}


	// Removes from the list the element located at the given position.
	public: iterator erase(iterator pos)
	{
		return list_.erase(pos);
	}


	// Removes from the list the elements located at the given range.
	public: iterator erase(iterator first, iterator last)
	{
		return list_.erase(first, last);
	}


	/// Return \c true if the list is empty; \c false otherwise.
	public: bool empty() const
	{
		return list_.empty();
	}


	/// Return the current size of the list.
	public: size_type size() const
	{
		return list_.size();
	}


	public: iterator begin()
	{
		return list_.begin();
	}


	public: iterator end()
	{
		return list_.end();
	}


	public: const_iterator begin() const
	{
		return list_.begin();
	}


	public: const_iterator end() const
	{
		return list_.end();
	}


	/// The underlying implementation.
	private: list_impl_type list_;
	/// The element comparator.
	private: comparator_type cmp_;
};


template <typename PtrEventT>
struct less: public ::std::binary_function<PtrEventT, PtrEventT, bool>
{
	bool operator()(PtrEventT const& x, PtrEventT const& y) const
	{
		return (*x) < (*y);
	}
};


template <typename PtrEventT>
struct less_equal: public ::std::binary_function<PtrEventT, PtrEventT, bool>
{
	bool operator()(PtrEventT const& x, PtrEventT const& y) const
	{
		return (*x) <= (*y);
	}
};

}}// Namespace detail::<unnamed>

/**
 * \brief Base event-list.
 *
 * \tparam EventT The event type.
 * \tparam SequenceT The event container type (default to priority-queue).
 *
 * \author Cosimo Anglano, &lt;cosimo.anglano@mfn.unipmn.it&gt;
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <
	typename EventT,
	//typename SequenceT=::std::priority_queue< EventT, ::std::vector<EventT>, ::std::greater<EventT> > // std::priority_queue by default returns the greater element
	//typename SequenceT=detail::ordered_list<EventT, ::std::less<EventT> >
	typename SequenceT=detail::ordered_list< ::boost::shared_ptr<EventT>, detail::less< ::boost::shared_ptr<EventT> > >//[sguazt] EXP
>
class event_list
{
	/// The type of the value hold by this container.
	public: typedef typename SequenceT::value_type value_type;
	/// The type of the reference to the value hold by this container.
	public: typedef typename SequenceT::reference reference;
	/// The type of the constant reference to the value hold by this container.
	public: typedef typename SequenceT::const_reference const_reference;
	/// The type of the size of this container.
	public: typedef typename SequenceT::size_type size_type;
	/// The type of the internal container.
	public: typedef SequenceT container_type;


	// compiler generated default/copy ctor and copy assignement are fine


	//@{ Public member functions

	/**
	 * \brief Insert a new event in the list.
	 * \param ptr_evt The (pointer to an) event to be inserted.
	 */
	public: void push(value_type const& evt)
	{
		seq_.push(evt);
	}


	/**
	 * \brief Extract the next event from the list.
	 */
	public: void pop()
	{
		seq_.pop();
	}


	/**
	 * \brief Tell if the list is empty (i.e., with no event).
	 * \return \c true if there no event inside the list; \c false otherwise.
	 */
	public: bool empty() const
	{
		return seq_.empty();
	}


	/**
	 * \brief Return the number of events inside the list.
	 * \return The number of events inside the list.
	 */
	public: size_type size() const
	{
		return seq_.size();
	}


	/**
	 * \brief Return the next event from the list (without extracting it).
	 * \return The next event.
	 */
	public: const_reference top() const
	{
		return seq_.top();
	}


	/**
	 * \brief Clear the list.
	 */
	public: void clear()
	{
		while (!seq_.empty())
		{
			seq_.pop();
		}
	}


	public: void erase(value_type const& evt)
	{
		//seq_.erase(::std::find(seq_.begin(), seq_.end(), evt));

		typedef typename container_type::iterator iterator;

		iterator end_it(seq_.end());
		// Find the position of the first "equivalent" event.
		// Note: Two events are equivalent if they are concurrent.
		iterator it(::std::find(seq_.begin(), end_it, evt));
		// Make sure to erase the exact event by checking for its ID
		while (it != end_it && (*it)->id() != evt->id())
		{
			++it;
		}
		if (it != end_it)
		{
			seq_.erase(it);
		}
		else
		{
			::std::clog << "[Warning] Event " << *evt << " not removed because it has not been found." << ::std::endl;
		}
	}


//	public: void erase(iterator pos)
//	{
//		seq_.erase(pos);
//	}


//	public: void erase(iterator first, iterator last)
//	{
//		seq_.erase(first, last);
//	}


//	public: void touch(value_type const& evt)
//	{
//		typedef typename container_type::iterator iterator;
//
////		iterator it(seq_.begin());
////		iterator end_it(seq_.end());
////		while (it != end_it && *it != evt)
////		{
////			++it;
////		}
//
//		iterator end_it(seq_.end());
//		iterator it(::std::find(seq_.begin(), end_it, evt));
//
//		if (it != end_it)
//		{
//			seq_.erase(it);
//			push(evt);
//		}
//	}


	//@} Public member functions

	//@{ Member variables

	/// The internal container
	private: SequenceT seq_;

	//@} Member variables
};

}} // dcs::des


#endif // DCS_DES_EVENT_LIST_HPP
