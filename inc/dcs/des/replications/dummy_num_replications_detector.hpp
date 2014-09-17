/**
 * \file dcs/des/replications/dummy_num_replications_detector.hpp
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

#ifndef DCS_DES_REPLICATIONS_DUMMY_NUM_REPLICATIONS_DETECTOR_HPP
#define DCS_DES_REPLICATIONS_DUMMY_NUM_REPLICATIONS_DETECTOR_HPP


#include <cstddef>
#include <dcs/macro.hpp>


namespace dcs { namespace des { namespace replications {

template <typename RealT>
class dummy_num_replications_detector
{
	public: typedef RealT real_type;


	// At least 2 replications are needed to compute variance and similar stats.
	private: static const std::size_t num_replications_ = 1;


	public: dummy_num_replications_detector()
	{
		// Empty
	}


	public: bool detect(std::size_t r_cur, real_type estimate, real_type stddev)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( r_cur );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( estimate );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( stddev );

		return true;
	}


	public: bool detected() const
	{
		return true;
	}


	public: bool aborted() const
	{
		return false;
	}


	public: std::size_t estimated_number() const
	{
		return num_replications_;
	}


	public: void reset()
	{
		// empty
	}
};

}}} // Namespace dcs::des::replications


#endif // DCS_DES_REPLICATIONS_DUMMY_NUM_REPLICATIONS_DETECTOR_HPP
