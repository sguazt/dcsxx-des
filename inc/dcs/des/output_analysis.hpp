/**
 * \file dcs/des/output_analysis.hpp
 *
 * \brief Utilities for simulation output analysis.
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

#ifndef DCS_DES_OUTPUT_ANALYSIS_HPP
#define DCS_DES_OUTPUT_ANALYSIS_HPP


#include <boost/smart_ptr.hpp>


namespace dcs { namespace des {

/**
 * \brief Creates an analyzable output staistic type from the given statistic
 *  and for the given discrete-event simulation engine.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <
	typename StatisticT,
	typename OutputAnalyzerT
>
struct make_analyzable_statistic_type;


/**
 * \brief Creates an analyzable output staistic from the given statistic
 *  and for the given discrete-event simulation engine.
 */
template <
	typename StatisticT,
	typename OutputAnalyzerT,
	typename DesEngineT
>
::boost::shared_ptr<
	typename make_analyzable_statistic_type<
		StatisticT,
		OutputAnalyzerT
	>::type
//> make_analyzable_statistic(StatisticT const& stat, DesEngineT const& des_engine, typename StatisticT::value_type relative_precision, typename StatisticT::value_type confidence_level, typename StatisticT::uint_type max_obs);
> make_analyzable_statistic(StatisticT const& stat, DesEngineT const& des_engine, OutputAnalyzerT const& output_analyzer, typename StatisticT::value_type relative_precision, /*typename StatisticT::value_type confidence_level,*/ typename StatisticT::uint_type max_obs);

}} // Namespace dcs::des


#endif // DCS_DES_OUTPUT_ANALYSIS_HPP
