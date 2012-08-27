/**
 * \file dcs/des/statistic_categories.hpp
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

#ifndef DCS_DES_STATISTIC_CATEGORY_HPP
#define DCS_DES_STATISTIC_CATEGORY_HPP


namespace dcs { namespace des {

/// Category for max estimators.
struct max_statistic_category { };


/// Category for min estimators.
struct min_statistic_category { };


/// Category for mean estimators.
struct mean_statistic_category { };


/// Category for quantile estimators.
struct quantile_statistic_category { };


/// Category for variance estimators.
struct variance_statistic_category { };


/// Statistic categories.
enum statistic_category
{
	max_statistic,
	mean_statistic,
	min_statistic,
	quantile_statistic,
	variance_statistic
};

}} // Namespace dcs::des


#endif // DCS_DES_STATISTIC_CATEGORY_HPP
