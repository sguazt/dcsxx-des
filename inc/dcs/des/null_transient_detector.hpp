/**
 * \file dcs/des/null_transient_detector.hpp
 *
 * \brief The do-nothing transient detector.
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

#ifndef DCS_DES_NULL_TRANSIENT_DETECTOR_HPP
#define DCS_DES_NULL_TRANSIENT_DETECTOR_HPP


#include <cstddef>
#include <utility>
#include <vector>


namespace dcs { namespace des {

template <typename RealT, typename UIntT = std::size_t>
class null_transient_detector
{
	public: typedef RealT real_type;
	public: typedef UIntT uint_type;
	public: typedef ::std::pair<real_type,real_type> sample_type;
	public: typedef ::std::vector<sample_type> sample_container;


	private: static const uint_type transient_size_ = 0;


	public: null_transient_detector()
	{
		// Empty
	}


	public: bool detect(real_type obs, real_type weight)
	{
		obs_.push_back(::std::make_pair(obs, weight));

		return true;
	}


	public: bool aborted() const
	{
		return false;
	}


	public: bool detected() const
	{
		return true;
	}


	public: uint_type estimated_size() const
	{
		return transient_size_;
	}


	public: void reset()
	{
		obs_.clear();
	}


	public: sample_container steady_state_observations() const
	{
		return obs_;
	}


	private: sample_container obs_;
};

}} // Namespace dcs::des


#endif // DCS_DES_NULL_TRANSIENT_DETECTOR_HPP
