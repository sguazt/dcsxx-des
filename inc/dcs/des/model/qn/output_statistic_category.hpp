/**
 * \file dcs/des/model/qn/output_statistic_category.hpp
 *
 * \brief Categories for output statistics.
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

#ifndef DCS_DES_MODEL_QN_OUTPUT_STATISTIC_CATEGORY_HPP
#define DCS_DES_MODEL_QN_OUTPUT_STATISTIC_CATEGORY_HPP


namespace dcs { namespace des { namespace model { namespace qn {


enum node_output_statistic_category
{
	busy_time_statistic_category = 0, ///< System-level busy time.
	interarrival_time_statistic_category, ///< Request interarrival time.
	num_busy_statistic_category, ///< Number of busy servers.
	num_waiting_statistic_category, ///< Number of waiting requests.
	response_time_statistic_category, ///< Request response tim time.
	service_time_statistic_category, ///< System-level service time.
	throughput_statistic_category, ///< Request throughput.
	utilization_statistic_category, ///< System-level server utilization.
	waiting_time_statistic_category, ///< Request waiting time.
	num_arrivals_statistic_category, ///< Number of arrived customers to the node.
	num_departures_statistic_category ///< Number of departed customers to the node.
};

enum network_output_statistic_category
{
	net_response_time_statistic_category = 0, ///< System-wide response time.
	net_throughput_statistic_category, ///< System-wide throughput.
	net_num_arrivals_statistic_category, ///< Number of arrived customers to the network.
	net_num_departures_statistic_category ///< Number of departed customers to the network.
};

}}}} // Namespace dcs::des::model::qn


#endif // DCS_DES_MODEL_QN_OUTPUT_STATISTIC_CATEGORY_HPP
