/**
 * \file dcs/des/batch_means/dummy_batch_size_detector.hpp
 *
 * \brief Dummy batch size detector.
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

#ifndef DCS_DES_BATCH_MEANS_DUMMY_BATCH_SIZE_DETECTOR_HPP
#define DCS_DES_BATCH_MEANS_DUMMY_BATCH_SIZE_DETECTOR_HPP


#include <boost/numeric/ublas/expression_types.hpp>
#include <boost/numeric/ublas/traits.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublasx/operation/size.hpp>
#include <cmath>
#include <cstddef>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/macro.hpp>
#include <dcs/math/constants.hpp>
#include <dcs/math/function/sqr.hpp>
#include <dcs/math/stats/distribution/normal.hpp>
#include <stdexcept>
#include <vector>


namespace dcs { namespace des { namespace batch_means {

/**
 * \brief Dummy batch size detector.
 *
 * \tparam RealT The type used for real numbers.
 * \tparam UIntT The type used for unsigned integral numbers.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename RealT=double, typename UIntT=::std::size_t>
class dummy_batch_size_detector
{
	public: typedef RealT real_type;
	public: typedef UIntT uint_type;
	public: typedef ::std::vector<real_type> vector_type;


	private: static const uint_type batch_size_ = 0;


	/// A constructor.
	public: dummy_batch_size_detector()
	{
		// empty
	}


	/**
	 * \brief Accumulate observation into the current batch mean and try to
	 *  detect a reasonable batch size.
	 * \param obs The new observation.
	 * \return \c true if batch size is successfully detected; \c false
	 *  otherwise.
	 */
	public: bool detect(real_type obs, real_type weight)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(obs);
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(weight);

		return true;
	}


	/**
	 * Tells if the batch size has been detected.
	 *
	 * \return \c true if batch size has been detected; \c false otherwise.
	 */
	public: bool detected() const
	{
		return true;
	}


	/**
	 * Tells if the batch size detection has been aborted without finding
	 * a suitable batch size..
	 *
	 * \return \c true if batch size detection has been aborted; \c false
	 *  otherwise.
	 */
	public: bool aborted() const
	{
		return false;
	}


	/// Reset the state of the detector.
	public: void reset()
	{
		// empty
	}


	public: uint_type estimated_size() const
	{
		return batch_size_;
	}


	public: vector_type computed_estimators() const
	{
		return vector_type();
	}


};

}}} // Namespace dcs::des::batch_means


#endif // DCS_DES_BATCH_MEANS_DUMMY_BATCH_SIZE_DETECTOR_HPP
