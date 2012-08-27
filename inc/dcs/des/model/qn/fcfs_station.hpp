/**
 * \file src/dcs/des/model/qn/fcfs_station.hpp
 *
 * \brief First-Come First-Served (FCFS) service station.
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

#ifndef DCS_DES_MODEL_QN_FCFS_STATION_HPP
#define DCS_DES_MODEL_QN_FCFS_STATION_HPP


#include <dcs/des/model/qn/queueing_network_traits.hpp>
#include <dcs/des/model/qn/service_station.hpp>
#include <string>


//DEPRECATED
namespace dcs { namespace des { namespace model { namespace qn {

template <typename QueueNetT>
class fcfs_station: public service_station<QueueNetT>
{
	private: typedef service_station<QueueNetT> base_type;
	public: typedef QueueNetT queueing_network_type;
	public: typedef typename base_type::identifier_type identifier_type;
	public: typedef typename base_type::queueing_network_pointer queueing_network_pointer;


	public: fcfs_station(identifier_type id,
						 ::std::string const& name,
						 queueing_network_pointer const& ptr_net,
						 queueing_policy const& policy)
	: base_type(id, name, ptr_net, queueing_input_strategy(policy))
	{
	}
};


}}}} // Namespace dcs::des::model::qn


#endif // DCS_DES_MODEL_QN_FCFS_STATION_HPP
