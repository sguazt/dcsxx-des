/**
 * \file dcs/des/model/queue/queue_M_M_1.hpp
 *
 * \brief Model for the M/M/1 queue.
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
#ifndef DCS_DES_MODEL_QUEUE_M_M_1_HPP
#define DCS_DES_MODEL_QUEUE_M_M_1_HPP


#include <boost/smart_ptr.hpp>
#include <cstddef>
#include <dcs/des/engine.hpp>
#include <dcs/des/model/queue/fifo_queue_policy.hpp>
#include <dcs/des/model/queue/queue_job.hpp>
#include <dcs/des/model/queue/open_queue.hpp>
#include <dcs/math/random/any_generator.hpp>
#include <dcs/math/stats/distribution/any_distribution.hpp>
#include <dcs/math/stats/distribution/exponential.hpp>


namespace dcs { namespace des { namespace model {

/**
 * \brief Model for the M/M/1 queue.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <
	typename RealT=double,
	typename UniformRandomGeneratorT=::dcs::math::random::any_generator<RealT>,
	typename DesEngineT=::dcs::des::engine<RealT>,
	typename OutputStatisticT=::dcs::des::any_statistic<RealT>
>
class queue_M_M_1: public open_queue<
							RealT,
							::dcs::math::stats::exponential_distribution<RealT>,
							::dcs::math::stats::exponential_distribution<RealT>,
							UniformRandomGeneratorT,
							fifo_queue_policy< queue_job<RealT> >,
							DesEngineT,
							OutputStatisticT
					>
{
	public: typedef RealT real_type;
	public: typedef ::dcs::math::stats::exponential_distribution<real_type> iatime_distribution_type;
	public: typedef ::dcs::math::stats::exponential_distribution<real_type> service_distribution_type;
	public: typedef UniformRandomGeneratorT random_generator_type;
	public: typedef fifo_queue_policy< queue_job<real_type> > queue_policy_type;
	public: typedef DesEngineT des_engine_type;
	public: typedef OutputStatisticT output_statistic_type;
	private: typedef open_queue<
						real_type,
						iatime_distribution_type,
						service_distribution_type,
						random_generator_type,
						queue_policy_type,
						des_engine_type,
						output_statistic_type> base_type;


	/// A constructor.
	public: queue_M_M_1(real_type arrival_rate, real_type service_rate, random_generator_type& rng, ::boost::shared_ptr<des_engine_type> const& ptr_eng)
		: base_type(
			iatime_distribution_type(arrival_rate),
			service_distribution_type(service_rate),
			1,
			false,
			0,
			rng,
			ptr_eng
		)
	{
		// empty
	}
};

}}} // Namespace dcs::des::model


#endif // DCS_DES_MODEL_QUEUE_M_M_1_HPP
