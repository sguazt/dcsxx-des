/**
 * \file dcs/des/replications/constant_num_replications_detector.hpp
 *
 * \brief Number of replications detector with a prescribed number of
 *  replications.
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

#ifndef DCS_DES_REPLICATIONS_CONSTANT_NUM_REPLICATIONS_DETECTOR_HPP
#define DCS_DES_REPLICATIONS_CONSTANT_NUM_REPLICATIONS_DETECTOR_HPP


#include <dcs/macro.hpp>
#include <dcs/math/constants.hpp>
//#include <vector>


namespace dcs { namespace des { namespace replications {

template <
	typename RealT,
	typename UIntT
>
class constant_num_replications_detector
{
	public: typedef RealT real_type;
	public: typedef UIntT uint_type;
//	public: typedef ::std::vector<real_type> vector_type;


	public: static const uint_type default_num_replications; // = ::dcs::math::constants::infinity<uint_type>::value;


	public: explicit constant_num_replications_detector(uint_type num_repl=default_num_replications)
		: r_(num_repl)
	{
		// Empty
	}


	public: bool detect(uint_type r_cur, real_type estimate, real_type stddev)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( r_cur );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( estimate );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( stddev );

//		estimators_.push_back(estimator);

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


	public: uint_type estimated_number() const
	{
		return r_;
	}


//	public: vector_type computed_estimators() const
//	{
//		return estimators_;
//	}


	public: void reset()
	{
//		estimators_.clear();
	}


	/// The actual number of replications
	private: uint_type r_;
//	private: vector_type estimators_;
};

template <typename RealT, typename UIntT>
const UIntT constant_num_replications_detector<RealT,UIntT>::default_num_replications = ::dcs::math::constants::infinity<UIntT>::value;

}}} // Namespace dcs::des::replications


#endif // DCS_DES_REPLICATIONS_CONSTANT_NUM_REPLICATIONS_DETECTOR_HPP
