/**
 * \file dcs/des/replications/fixed_num_obs_replication_size_detector.hpp
 *
 * \brief Replication size detector based on a fixed number of observations.
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

#ifndef DCS_DES_REPLICATIONS_FIXED_NUM_OBS_REPLICATION_SIZE_DETECTOR_HPP
#define DCS_DES_REPLICATIONS_FIXED_NUM_OBS_REPLICATION_SIZE_DETECTOR_HPP


#include <cstddef>
#include <dcs/debug.hpp>
#include <dcs/math/constants.hpp>
#include <utility>
#include <vector>


namespace dcs { namespace des { namespace replications {

/**
 * \brief Replication size detector based on a fixed number of observations.
 *
 * \tparam RealT The type used for real numbers.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename RealT>
class fixed_num_obs_replication_size_detector
{
	public: typedef RealT real_type;
	public: typedef ::std::pair<real_type,real_type> sample_type;
	public: typedef ::std::vector<sample_type> vector_type;


	/// Constant for setting the duration of replication size determination
	/// to infinity.
	public: static const std::size_t num_obs_infinity; // = ::dcs::math::constants::infinity<std::size_t>::value;


	public: static const std::size_t default_num_obs = 1000;


	/**
	 * \brief A constructor.
	 *
	 * \param n_obs The number of observations to collect.
	 */
	public: explicit fixed_num_obs_replication_size_detector(std::size_t num_obs)
		: num_obs_(num_obs)//,
//		  detect_aborted_(false)
//		  detected_(true)
	{
		// empty
	}


	/**
	 * \brief Accumulate a new observation.
	 * \param obs The new observation.
	 * \return \c true if replication size is successfully detected; \c false
	 *  otherwise.
	 */
	public: bool detect(real_type obs, real_type weight)
	{
		obs_.push_back(::std::make_pair(obs, weight));

//NOTE: we already know how many observations to collect.
//      So it is useless to store all the observations until the wanted number
//      is reached!!
//
//		detected_ = obs_.size() >= num_obs_;
//
//		return detected_;
		return true;
	}


	/**
	 * Tells if the replication size has been detected.
	 *
	 * \return \c true if replication size has been detected; \c false
	 *  otherwise.
	 */
	public: bool detected() const
	{
//NOTE: we already know how many observations to collect.
//		return detected_;
		return true;
	}


	/**
	 * Tells if the replication size detection has been aborted without finding
	 * a suitable replication size..
	 *
	 * \return \c true if replication size detection has been aborted; \c false
	 *  otherwise.
	 */
	public: bool aborted() const
	{
//		return detect_aborted_;
		return false;
	}


	/// Reset the state of the detector.
	public: void reset()
	{
//		detect_aborted_ = detected_
//		detect_aborted_ = false;
		obs_.clear();
	}


	public: std::size_t estimated_size() const
	{
//		return obs_.size();
		return num_obs_;
	}


	public: vector_type consumed_observations() const
	{
		return obs_;
	}


	/// Number of observations
	private: std::size_t num_obs_;
//	/// Tells if replication size detection has been aborted without finding a
//	/// right replication size.
//	private: bool detect_aborted_;
//	/// Tells if replication size has been detected.
//	private: bool detected_;
	private: vector_type obs_;
};

template <typename RealT>
const std::size_t fixed_num_obs_replication_size_detector<RealT>::num_obs_infinity = ::dcs::math::constants::infinity<std::size_t>::value;

}}} // Namespace dcs::des::replications


#endif // DCS_DES_REPLICATIONS_FIXED_NUM_OBS_REPLICATION_SIZE_DETECTOR_HPP
