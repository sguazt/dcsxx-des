/**
 * \file dcs/des/model/qn/queueing_network_traits.hpp
 *
 * \brief Traits class for queueing networks.
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

#ifndef DCS_DES_MODEL_QN_QUEUEING_NETWORK_TRAITS_HPP
#define DCS_DES_MODEL_QN_QUEUEING_NETWORK_TRAITS_HPP


namespace dcs { namespace des { namespace model { namespace qn {

template <typename QueueNetT>
struct queueing_network_traits
{
	typedef QueueNetT network_type;
	typedef typename QueueNetT::class_identifier_type class_identifier_type;
//	typedef typename QueueNetT::class_pointer class_pointer;
	typedef typename QueueNetT::class_size_type class_size_type;
	typedef typename QueueNetT::class_type class_type;
	typedef typename QueueNetT::customer_type customer_type;
	typedef typename QueueNetT::engine_pointer engine_pointer;
	typedef typename QueueNetT::engine_type engine_type;
	typedef typename QueueNetT::node_identifier_type node_identifier_type;
//	typedef typename QueueNetT::node_pointer node_pointer;
	typedef typename QueueNetT::node_size_type node_size_type;
	typedef typename QueueNetT::node_type node_type;
	typedef typename QueueNetT::priority_type priority_type;
	typedef typename QueueNetT::random_generator_pointer random_generator_pointer;
	typedef typename QueueNetT::random_generator_type random_generator_type;
	typedef typename QueueNetT::real_type real_type;
	typedef typename QueueNetT::uint_type uint_type;
//	typedef typename engine_type::event_source_type event_source_type;
//	typedef typename engine_type::event_type event_type;
//	typedef typename engine_type::engine_context_type engine_context_type;


	static node_identifier_type invalid_node_id()
	{
		return QueueNetT::invalid_node_id;
	}


	static class_identifier_type invalid_class_id()
	{
		return QueueNetT::invalid_class_id;
	}
};

}}}} // Namespace dcs::des::model::qn


#endif // DCS_DES_MODEL_QN_QUEUEING_NETWORK_TRAITS_HPP
