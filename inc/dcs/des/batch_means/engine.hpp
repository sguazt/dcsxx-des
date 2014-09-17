/**
 * \file dcs/des/batch_means/engine.hpp
 *
 * \brief Discrete-event simulator engine with output analysis based on
 *  Batch Means method.
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
 * \author Cosimo Anglano (cosimo.anglano@di.unipmn.it)
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */

#ifndef DCS_DES_BATCH_MEANS_ENGINE_HPP
#define DCS_DES_BATCH_MEANS_ENGINE_HPP


#include <boost/smart_ptr.hpp>
#include <cstddef>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/batch_means/analyzable_statistic.hpp>
#include <dcs/des/batch_means/dummy_batch_size_detector.hpp>
#include <dcs/des/engine.hpp>
#include <dcs/des/null_transient_detector.hpp>
#include <dcs/des/output_analysis.hpp>
#include <dcs/des/output_analysis_categories.hpp>
#include <dcs/math/constants.hpp>


namespace dcs { namespace des {

namespace batch_means {

/**
 * \brief Discrete-event simulator engine with output analysis based on the
 *  Batch Means method.
 *
 * \tparam RealT The type used for real numbers.
// * \tparam TransientPhaseDetectorT The default transient phase detector type for
// *  analyzable output statistics.
// * \tparam BatchSizeDetectorT The default transient phase detector type for
// *  analyzable output statistics.
 *
 * The rationale for passing the transient phase detector and batch size
 * detector type is that
 * - output statistic should have the same transient phase and batch size
 *   detectors;
 * - when we are going to create a new output statistic it depends on the type
 *   of output analysis; since output analysis and simulator engine are strictly
 *   correlated, the creation of output statistics should be related to the type
 *   of simulator engine actually used.
 * .
 *
 * \author Cosimo Anglano (cosimo.anglano@di.unipmn.it)
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename RealT=double>
class engine: public ::dcs::des::engine<RealT>
{
	private: typedef ::dcs::des::engine<RealT> base_type;
	public: typedef typename base_type::real_type real_type;
//	public: typedef TransientPhaseDetectorT transient_phase_detector_type;
//	public: typedef BatchSizeDetectorT batch_size_detector_type;
	public: typedef typename base_type::event_type event_type;
	public: typedef typename base_type::engine_context_type engine_context_type;
	public: typedef typename base_type::event_source_type event_source_type;
	public: typedef ::dcs::des::batch_means_analysis_category output_analysis_category;


	protected: void prepare_simulation(engine_context_type& ctx)
	{
		base_type::prepare_simulation(ctx);
		this->initialize_simulated_system(ctx);
	}


	protected: void finalize_simulation(engine_context_type& ctx)
	{
		this->finalize_simulated_system(ctx);
		base_type::finalize_simulation(ctx);
	}


	private: void do_run()
	{
		DCS_DEBUG_TRACE( "Begin SIMULATION" );

		engine_context_type ctx(this);

		this->prepare_simulation(ctx);

		while (!this->end_of_simulation() && !this->future_event_list().empty())
		{
			DCS_DEBUG_TRACE( "Simulation time: " << this->simulated_time());

			this->fire_next_event(ctx);

			this->monitor_statistics();
		}

		if (this->end_of_simulation())
		{
			this->future_event_list().clear();
		}

		this->finalize_simulation(ctx);

		DCS_DEBUG_TRACE( "End SIMULATION" );
	}


	private: boost::shared_ptr< base_analyzable_statistic<RealT> > do_make_analyzable_statistic(any_statistic<RealT> const& stat)
	{
		typedef ::dcs::des::null_transient_detector<real_type> transient_detector_type;
		typedef dummy_batch_size_detector<real_type> batch_size_detector_type;
		typedef analyzable_statistic<any_statistic<RealT>,
									 transient_detector_type,
									 batch_size_detector_type> analyzable_statistic_impl_type;

		return ::boost::shared_ptr<analyzable_statistic_impl_type>(
				new analyzable_statistic_impl_type(
					stat,
					transient_detector_type(),
					batch_size_detector_type(),
					::dcs::math::constants::infinity<real_type>::value,
					::dcs::math::constants::infinity<std::size_t>::value
				)
			);
	}
};


namespace detail {

template <
	typename TransientDetectorT,
	typename BatchSizeDetectorT
>
struct output_analyzer
{
	typedef TransientDetectorT transient_detector_type;
	typedef BatchSizeDetectorT batch_size_detector_type;
};

} // Namespace detail

} // Namespace batch_means


/// Instantiation of class \c make_analyzable_statistic_type for batch means
/// output analysis.
template <
	typename StatisticT,
//	typename RealT,
	typename TransientPhaseDetectorT,
	typename BatchSizeDetectorT
>
struct make_analyzable_statistic_type<
			StatisticT,
			batch_means::detail::output_analyzer<
				TransientPhaseDetectorT,
				BatchSizeDetectorT
			>//,
//			batch_means::engine<
//				RealT,
//				TransientPhaseDetectorT,
//				BatchSizeDetectorT
//			>
	>
{
	typedef batch_means::analyzable_statistic<
				StatisticT,
				TransientPhaseDetectorT,
				BatchSizeDetectorT
			> type;
};


/// Overload of function \c make_analyzable_statistic for batch means
/// output analysis.
template <
	typename StatisticT,
	typename RealT,
	typename TransientPhaseDetectorT,
	typename BatchSizeDetectorT
>
::boost::shared_ptr<
	typename make_analyzable_statistic_type<
		StatisticT,
		batch_means::detail::output_analyzer<
			TransientPhaseDetectorT,
			BatchSizeDetectorT
		>//,
//		batch_means::engine<
//			RealT,
//			TransientPhaseDetectorT,
//			BatchSizeDetectorT
//		>
	>::type
> make_analyzable_statistic(StatisticT const& stat, TransientPhaseDetectorT const& transient_detector, BatchSizeDetectorT const& batch_size_detector, batch_means::engine<RealT>& des_engine, RealT relative_precision, std::size_t max_obs)
{
	typedef typename make_analyzable_statistic_type<
						StatisticT,
						batch_means::detail::output_analyzer<
							TransientPhaseDetectorT,
							BatchSizeDetectorT
						>//,
//						batch_means::engine<
//							RealT,
//							TransientPhaseDetectorT,
//							BatchSizeDetectorT
//						>
					>::type statistic_type;

	::boost::shared_ptr<statistic_type> ptr_stat(
			new statistic_type(
				stat,
				transient_detector,
				batch_size_detector,
				relative_precision,
				max_obs
			)
	);

	des_engine.analyze_statistic(ptr_stat);

	return ptr_stat;
}

}} // Namespace dcs::des


#endif // DCS_DES_BATCH_MEANS_ENGINE_HPP
