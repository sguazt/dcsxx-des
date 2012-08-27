/**
 * \file dcs/des/model/queue/queue_statistics.hpp
 *
 * \brief Output statistic tags for queueing models.
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

#ifndef DCS_DES_MODEL_QUEUE_QUEUE_STATISTICS_HPP
#define DCS_DES_MODEL_QUEUE_QUEUE_STATISTICS_HPP


namespace dcs { namespace des { namespace model {

/**
 * \brief Output statistic tags for queueing models.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
enum queue_statistics
{
	busy_time_queue_statistic = 0, ///< System-level busy time.
	interarrival_time_queue_statistic, ///< Request interarrival time.
	num_busy_queue_statistic, ///< Number of busy servers.
	num_waiting_queue_statistic, ///< Number of waiting requests.
	response_time_queue_statistic, ///< Request response time.
	service_time_queue_statistic, ///< System-level service time.
	throughput_queue_statistic, ///< Request throughput.
	utilization_queue_statistic, ///< System-level server utilization.
	waiting_time_queue_statistic ///< Request waiting time.
};

}}} // Namespace dcs::des::model


#endif // DCS_DES_MODEL_QUEUE_QUEUE_STATISTICS_HPP
