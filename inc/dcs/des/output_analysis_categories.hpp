/**
 * \file dcs/des/output_analysis_categories.hpp
 *
 * \brief Categories for simulation output analysis.
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

#ifndef DCS_DES_OUTPUT_ANALYSIS_CATEGORIES_HPP
#define DCS_DES_OUTPUT_ANALYSIS_CATEGORIES_HPP


namespace dcs { namespace des {

/// Category for Batch Means output analysis.
struct batch_means_analysis_category { };


/// Category for Independent Replication output analysis.
struct independent_replications_analysis_category { };


/// Category for Replicated Batch Means output analysis.
struct replicated_batch_means_analysis_category { };

}} // Namespace dcs::des


#endif // DCS_DES_OUTPUT_ANALYSIS_CATEGORIES_HPP
