#ifndef DCS_EXAMPLES_DES_BANK_HPP
#define DCS_EXAMPLES_DES_BANK_HPP


#include <cstddef>
#include <dcs/des/mean_estimator.hpp>
#include <dcs/des/model/queue/queue_M_M_k.hpp>
#include <dcs/des/model/queue/queue_statistics.hpp>
#include <dcs/des/null_transient_detector.hpp>
#include <dcs/des/quantile_estimator.hpp>
#include <dcs/des/replications/engine.hpp>
#include <dcs/des/replications/fixed_num_obs_replication_size_detector.hpp>
#include <dcs/des/replications/banks2005_num_replications_detector.hpp>
#include <dcs/des/replications/constant_num_replications_detector.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/math/constants.hpp>
#include <dcs/math/random.hpp>
#include <dcs/memory.hpp>


namespace dcs { namespace examples { namespace des { namespace bank {

template <typename RealT=double>
class bank
{
	public: typedef RealT real_type;
	private: typedef ::dcs::math::random::mt19937 random_generator_type;
	private: typedef ::dcs::math::random::minstd_rand1 deterministic_random_device_type;
	private: typedef ::dcs::des::replications::engine<real_type> des_engine_type;
	private: typedef typename des_engine_type::event_source_type des_event_source_type;
	private: typedef typename des_engine_type::event_type des_event_type;
	private: typedef ::dcs::shared_ptr<des_engine_type> des_engine_pointer;
	private: typedef typename des_engine_type::engine_context_type des_engine_context_type;
	//private: typedef ::dcs::des::base_statistic<real_type,std::size_t> output_statistic_type;
	private: typedef ::dcs::des::base_analyzable_statistic<real_type> output_statistic_type;
	private: typedef ::dcs::shared_ptr<output_statistic_type> output_statistic_pointer;
	private: typedef ::dcs::des::model::queue_M_M_k<
						real_type,
						random_generator_type,
						des_engine_type,
						output_statistic_type
				> queue_type;
	private: typedef bank<real_type> self_type;


	private: static const std::size_t default_replication_size = std::size_t(1000);
	private: static const std::size_t default_num_replications = std::size_t(5);
	private: static const std::size_t default_seed0 = std::size_t(5489);
	private: static const real_type default_ci_level;/* = 0.95 */
	private: static const real_type default_relative_precision;/* = 0.04 */


	public: bank(std::size_t num_tellers, real_type arr_rate, real_type svc_rate, bool verbose=false)
		: num_tellers_(num_tellers),
		  arr_rate_(arr_rate),
		  svc_rate_(svc_rate),
		  verbose_(verbose),
		  repl_size_(default_replication_size),
		  num_repl_(default_num_replications),
		  rnd_dev_(default_seed0),
		  rng_(),
		  ptr_des_engine_(new des_engine_type()),
		  queue_(arr_rate, svc_rate, num_tellers_, rng_, ptr_des_engine_),
		  ptr_mean_response_time_(),
		  ptr_q99_response_time_(),
		  response_time_rel_prec_(default_relative_precision),
		  response_time_ci_level_(default_ci_level)
	{
		// Empty
	}


	public: void simulate()
	{
		ptr_des_engine_->begin_of_replication_event_source().connect(
//		ptr_des_engine_->system_initialization_event_source().connect(
			::dcs::functional::bind(
				&self_type::process_begin_of_replication,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);

		ptr_des_engine_->end_of_replication_event_source().connect(
//		ptr_des_engine_->system_finalization_event_source().connect(
			::dcs::functional::bind(
				&self_type::process_end_of_replication,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);

		ptr_mean_response_time_ = ::dcs::des::make_analyzable_statistic(
					::dcs::des::mean_estimator<real_type>(response_time_ci_level_),
					::dcs::des::null_transient_detector<real_type>(),
					::dcs::des::replications::fixed_num_obs_replication_size_detector<real_type>(repl_size_),
					//::dcs::des::replications::constant_num_replications_detector<real_type>(num_repl_),
					::dcs::des::replications::banks2005_num_replications_detector<real_type>(response_time_ci_level_, response_time_rel_prec_),
					*ptr_des_engine_,
					response_time_rel_prec_,
					::dcs::math::constants::infinity<std::size_t>::value
		);
		queue_.statistic(
			::dcs::des::model::response_time_queue_statistic,
			ptr_mean_response_time_
		);
		ptr_q99_response_time_ = ::dcs::des::make_analyzable_statistic(
					::dcs::des::quantile_estimator<real_type>(0.99, response_time_ci_level_),
					::dcs::des::null_transient_detector<real_type>(),
					::dcs::des::replications::fixed_num_obs_replication_size_detector<real_type>(repl_size_),
					//::dcs::des::replications::constant_num_replications_detector<real_type>(num_repl_),
					::dcs::des::replications::banks2005_num_replications_detector<real_type>(response_time_ci_level_, response_time_rel_prec_),
					*ptr_des_engine_,
					response_time_rel_prec_,
					::dcs::math::constants::infinity<std::size_t>::value
		);
		queue_.statistic(
			::dcs::des::model::response_time_queue_statistic,
			ptr_q99_response_time_
		);

		ptr_des_engine_->run();

		::std::cout	<< "Final Response Time: " << ::std::endl
					<< "  Mean: " << *ptr_mean_response_time_ << ::std::endl
					<< "  99th Quantile: " << *ptr_q99_response_time_ << ::std::endl;
	}


	// Handle the beginning of a new replication
	private: void process_begin_of_replication(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(evt);
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(ctx);

		std::size_t seed = rnd_dev_();
		rng_.seed(seed);

		queue_.reset();
	}


	// Handle the end of a new replication
	private: void process_end_of_replication(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(ctx);

		std::size_t repl_num = evt.template unfolded_state<std::size_t>();

		if (verbose_)
		{
			::std::cout	<< "Replication #" << repl_num << ::std::endl
						<< "  # Arrivals: " << queue_.num_arrivals() << ::std::endl
						<< "  # Departures: " << queue_.num_departures() << ::std::endl
						<< "  # Discards: " << queue_.num_discards() << ::std::endl
						<< "  Response Time: " << ::std::endl
						<< "    Mean: " << *ptr_mean_response_time_ << ::std::endl
						<< "    99th Quantile: " << *ptr_q99_response_time_ << ::std::endl;
		}
	}


	private: std::size_t num_tellers_;
	private: real_type arr_rate_;
	private: real_type svc_rate_;
	private: bool verbose_;
	private: std::size_t repl_size_;
	private: std::size_t num_repl_;
	/// The deterministic random device (actually an LCG) used to generate
	/// different seeds for each replication.
	private: deterministic_random_device_type rnd_dev_;
	private: random_generator_type rng_;
	private: des_engine_pointer ptr_des_engine_;
	//private: real_type ci_level_;
	//private: real_type rel_prec_;
	private: queue_type queue_;
	private: output_statistic_pointer ptr_mean_response_time_;
	private: output_statistic_pointer ptr_q99_response_time_;
	private: real_type response_time_rel_prec_;
	private: real_type response_time_ci_level_;
};

template <typename RealT>
const RealT bank<RealT>::default_ci_level = 0.95;

template <typename RealT>
const RealT bank<RealT>::default_relative_precision = 0.04;

}}}} // Namespace dcs::examples::des::bank


#endif // DCS_EXAMPLES_DES_BANK_HPP
