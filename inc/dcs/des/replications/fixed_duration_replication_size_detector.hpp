/**
 * \file dcs/des/replications/constant_replication_size_detector.hpp
 *
 * \brief Replications with fixed length.
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

#ifndef DCS_DES_REPLICATIONS_FIXED_DURATION_REPLICATION_SIZE_DETECTOR_HPP
#define DCS_DES_REPLICATIONS_FIXED_DURATION_REPLICATION_SIZE_DETECTOR_HPP


#include <boost/smart_ptr.hpp>
#include <cstddef>
#include <dcs/debug.hpp>
#include <dcs/math/constants.hpp>
#include <utility>
#include <vector>


namespace dcs { namespace des { namespace replications {

/**
 * \brief Replication of fixed length.
 *
 * \tparam RealT The type used for real numbers.
 * \tparam UIntT The type used for unsigned integral numbers.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename RealT,
		  typename UIntT = std::size_t,
		  typename DesEngineT = dcs::des::engine<RealT> >
class fixed_duration_replication_size_detector
{
	public: typedef RealT real_type;
	public: typedef UIntT uint_type;
	public: typedef DesEngineT des_engine_type;
	//public: typedef ::boost::shared_ptr<des_engine_type> des_engine_pointer;
	public: typedef des_engine_type* des_engine_pointer;
	public: typedef ::std::pair<real_type,real_type> sample_type;
	public: typedef ::std::vector<sample_type> vector_type;


	/**
	 * \brief A constructor.
	 *
	 * \param time The time at which the detection must stop
	 * \param ptr_engine A pointer to the simulator engine
	 */
	public: fixed_duration_replication_size_detector(real_type time,
													 des_engine_type* ptr_engine)
	: max_duration_(time),
	  ptr_eng_(ptr_engine)
	{
		// empty
	}

	/// Deprecated
	public: fixed_duration_replication_size_detector(real_type time,
													 boost::shared_ptr<des_engine_type> const& ptr_engine)
	: max_duration_(time),
	  ptr_eng_(ptr_engine.get())
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

//		detected_ = ptr_eng_->simulated_time() >= max_duration_;

//		return detected_;
		return detected();
	}


	/**
	 * Tells if the replication size has been detected.
	 *
	 * \return \c true if replication size has been detected; \c false
	 *  otherwise.
	 */
	public: bool detected() const
	{
//		return detected_;
		return ptr_eng_->simulated_time() >= max_duration_;
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
//						= false;
		obs_.clear();
	}


	public: uint_type estimated_size() const
	{
		return obs_.size();
	}


	public: vector_type consumed_observations() const
	{
		return obs_;
	}


	/// Number of observations
	private: real_type max_duration_;
	private: des_engine_pointer ptr_eng_;
//	/// Tells if replication size detection has been aborted without finding a
//	/// right replication size.
//	private: bool detect_aborted_;
//	/// Tells if replication size has been detected.
//	private: bool detected_;
	private: vector_type obs_;

};

}}} // Namespace dcs::des::replications


#endif // DCS_DES_REPLICATIONS_FIXED_DURATION_REPLICATION_SIZE_DETECTOR_HPP
