/**
 * \file dcs/des/replications/dummy_replication_size_detector.hpp
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

#ifndef DCS_DES_REPLICATIONS_DUMMY_REPLICATION_SIZE_DETECTOR_HPP
#define DCS_DES_REPLICATIONS_DUMMY_REPLICATION_SIZE_DETECTOR_HPP


#include <dcs/macro.hpp>
#include <utility>
#include <vector>


namespace dcs { namespace des { namespace replications {

/**
 * \brief Replication size detector based on a fixed number of observations.
 *
 * \tparam RealT The type used for real numbers.
 * \tparam UIntT The type used for unsigned integral numbers.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename RealT, typename UIntT>
class dummy_replication_size_detector
{
	public: typedef RealT real_type;
	public: typedef UIntT uint_type;
	public: typedef ::std::pair<real_type,real_type> sample_type;
	public: typedef ::std::vector<sample_type> vector_type;


	private: static const uint_type replication_size_ = 0;


	/**
	 * \brief A constructor.
	 *
	 * \param n_obs The number of observations to collect.
	 */
	public: dummy_replication_size_detector()
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
		return false;
	}


	/// Reset the state of the detector.
	public: void reset()
	{
		obs_.clear();
	}


	public: uint_type estimated_size() const
	{
		return replication_size_;
	}


	public: vector_type consumed_observations() const
	{
		return obs_;
	}


	private: vector_type obs_;
};

}}} // Namespace dcs::des::replications


#endif // DCS_DES_REPLICATIONS_DUMMY_REPLICATION_SIZE_DETECTOR_HPP
