/**
 * \file examples/src/dcs/des/simple_simulator/system.hpp
 *
 * \brief System model and simulator.
 *
 * Copyright (C) 2009-2010  Distributed Computing System (DCS) Group, Computer
 * Science Department - University of Piemonte Orientale, Alessandria (Italy).
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
 * \author Massimo Canonico, &lt;massimo.canonico@mfn.unipmn.it&gt;
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */

#ifndef DCS_EXAMPLES_DES_SIMPLE_SIMULATOR_SYSTEM_HPP
#define DCS_EXAMPLES_DES_SIMPLE_SIMULATOR_SYSTEM_HPP


#include <cstddef>
#include <dcs/debug.hpp>
#include <dcs/des/base_analyzable_statistic.hpp>
#include <dcs/des/batch_means/engine.hpp>
#include <dcs/des/mean_estimator.hpp>
#include <dcs/des/output_analysis.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
#include <dcs/math/stats/distributions.hpp>
#include <dcs/math/stats/function/rand.hpp>
#include <dcs/math/random.hpp>
#include <dcs/memory.hpp>
#include "machine.hpp"
#include <queue>
#include "task.hpp"
#include <vector>


namespace dcs { namespace examples { namespace des { namespace simple_simulator {

/**
 * \brief System model and simulator.
 *
 * \author Massimo Canonico, &lt;massimo.canonico@mfn.unipmn.it&gt;
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <
	typename RealT=double,
	typename UIntT=unsigned long
>
class system
{
	// Convenience type declarations.

	private: typedef system<RealT,UIntT> self_type;
	private: typedef RealT real_type;
	private: typedef UIntT uint_type;
	private: typedef ::dcs::des::batch_means::engine<real_type> des_engine_type;
	private: typedef typename des_engine_type::event_source_type des_event_source_type;
	private: typedef typename des_engine_type::event_type des_event_type;
	private: typedef typename des_engine_type::engine_context_type des_engine_context_type;
	private: typedef ::dcs::des::base_analyzable_statistic<real_type> output_statistic_type;


	// Default values.
	// NOTE: C++ <= 98 does not allow floating-point constants

	private: static const uint_type default_num_machines; // = 16u;
	private: static const uint_type default_seed; // = 1435748658uL;
	private: static /*const*/ real_type default_arrival_rate; // = 0.02;
	private: static const uint_type default_min_task_size; // = 60;
	private: static const uint_type default_max_task_size; // = 600;
	/// Task blocking time for each checkpoint
	private: static /*const*/ real_type default_checkpoint_time; // = 10;
	/// Checkpoints taken every 10 minutes
	private: static const uint_type default_checkpoint_distance; // = 600;


	/// Default constructor.
	public: system()
		: machines_(default_num_machines),
		  rng_(default_seed),
		  //rng_(5489u),
		  arrival_(default_arrival_rate),
		  task_size_(default_min_task_size, default_max_task_size),
		  checkpoint_time_(default_checkpoint_time),
		  checkpoint_distance_(default_checkpoint_distance),
		  ptr_task_arrival_evt_src_(new des_event_source_type()),
		  ptr_task_departure_evt_src_(new des_event_source_type()),
		  ptr_task_suspend_evt_src_(new des_event_source_type()),
		  ptr_task_restart_evt_src_(new des_event_source_type())
	{
		init();
	}


	/// Initialize the average task completion time statistic.
	public: void average_task_completion_time(real_type relative_precision, real_type confidence_level, uint_type max_num_obs)
	{
		ptr_avg_task_completion_time_ = dcs::des::make_analyzable_statistic(
			::dcs::des::mean_estimator<real_type>(confidence_level),
			::dcs::des::spectral::pawlikowski1990_transient_detector<real_type>(),
			::dcs::des::batch_means::pawlikowski1990_batch_size_detector<real_type>(),
			des_engine_,
			relative_precision,
			//confidence_level,
			max_num_obs
		);
	}


	/// Return the average task completion time statistic.
	public: output_statistic_type const& average_task_completion_time() const
	{
		return *(ptr_avg_task_completion_time_);
	}


	/// Initialize the average task waiting time statistic.
	public: void average_task_waiting_time(real_type relative_precision, real_type confidence_level, uint_type max_num_obs)
	{
		ptr_avg_task_waiting_time_ = ::dcs::des::make_analyzable_statistic(
			::dcs::des::mean_estimator<real_type>(confidence_level),
			::dcs::des::spectral::pawlikowski1990_transient_detector<real_type>(),
			::dcs::des::batch_means::pawlikowski1990_batch_size_detector<real_type>(),
			des_engine_,
			relative_precision,
			//confidence_level,
			max_num_obs
		);
	}


	/// Return the average task waiting time statistic.
	public: output_statistic_type const& average_task_waiting_time() const
	{
		return *(ptr_avg_task_waiting_time_);
	}


	/// Simulate the system.
	public: void simulate()
	{
		des_engine_.run();
	}


	/// Initialize the system.
    private: void init()
    {
		ptr_task_arrival_evt_src_->connect(
			::dcs::functional::bind(
				&self_type::process_arrival,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
		ptr_task_departure_evt_src_->connect(
			::dcs::functional::bind(
				&self_type::process_departure,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
		ptr_task_suspend_evt_src_->connect(
			::dcs::functional::bind(
				&self_type::process_suspend,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
		ptr_task_restart_evt_src_->connect(
			::dcs::functional::bind(
				&self_type::process_restart,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
		des_engine_.begin_of_sim_event_source().connect(
			::dcs::functional::bind(
				&self_type::prepare_simulation,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
	}


	/// Prepare simulation
	private: void prepare_simulation(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(evt);

		DCS_DEBUG_TRACE("Processing BEGIN-OF-SIMULATION event: " << evt);

		// Create the machines pool
		machines_.clear();
		for (::std::size_t i = 0; i < machines_.capacity(); ++i)
		{
			machines_.push_back(
				::dcs::shared_ptr< machine<real_type,uint_type> >(
					new machine<real_type,uint_type>(i)
				)
			);
		}

		// Reset statistics
		ptr_avg_task_completion_time_->reset();
		ptr_avg_task_waiting_time_->reset();

		// Schedule the first arrival
		ctx.schedule_event(
			ptr_task_arrival_evt_src_,
			ctx.simulated_time() + ::dcs::math::stats::rand(arrival_, rng_)
		);
	}


    /// Handle the arrival of a task
	private: void process_arrival(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(evt);

		DCS_DEBUG_TRACE("Processing ARRIVAL event: " << evt);

		// Schedule next task arrival
		ctx.schedule_event(
			ptr_task_arrival_evt_src_,
			ctx.simulated_time() + ::dcs::math::stats::rand(arrival_, rng_)
		);

		real_type exec = ::dcs::math::stats::rand(task_size_, rng_);
		// Determine its execution time
		waiting_queue_.push(
			::dcs::shared_ptr< task<real_type,uint_type> >(
				new task<real_type,uint_type>(
					ctx.simulated_time(),
					exec
					//::dcs::math::stats::rand(task_size_, rng_)
				)
			)
		);

		// Call the scheduler
		schedule(ctx);
	}


    /// Handle the departure of a task
	private: void process_departure(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_DEBUG_TRACE("Processing DEPARTURE event: " << evt);

		// Get the task descriptor from the machine descriptor on which it is
		// running.

		uint_type mach_id = evt.template unfolded_state<uint_type>();

		// Update statistics
		real_type time;

		time = ctx.simulated_time() - machines_[mach_id]->current_task->arrival_time;
		(*ptr_avg_task_completion_time_)(time);

		time = machines_[mach_id]->current_task->queue_time;
		(*ptr_avg_task_waiting_time_)(time);

		machines_[mach_id]->current_task.reset();
		machines_[mach_id]->busy = false;

		//call the scheduler
		schedule(ctx);
	}


    /// Handle the suspension of a task for a checkpoint
	private: void process_suspend(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_DEBUG_TRACE("Processing SUSPEND event: " << evt);

		uint_type mach_id = evt.template unfolded_state<uint_type>();

		// Schedule next task arrival
		ctx.schedule_event(
			ptr_task_suspend_evt_src_,
			ctx.simulated_time() + checkpoint_time_,
			mach_id
		);

		real_type time;
		time = ctx.simulated_time() - machines_[mach_id]->current_task->last_restart_time;
		machines_[mach_id]->current_task->remaining_time -= time;
	}


    /// Handle the restart of a task after a checkpoint
	private: void process_restart(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_DEBUG_TRACE("Processing RESTART event: " << evt);

		uint_type mach_id = evt.template unfolded_state<uint_type>();

		machines_[mach_id]->current_task->last_restart_time = ctx.simulated_time();
		if (machines_[mach_id]->current_task->remaining_time <= checkpoint_distance_)
		{
			// Schedule task departure
			ctx.schedule_event(
				ptr_task_departure_evt_src_,
				ctx.simulated_time() + machines_[mach_id]->current_task->remaining_time,
				mach_id
			);
		}
		else
		{
			// Schedule task suspension
			ctx.schedule_event(
				ptr_task_suspend_evt_src_,
				ctx.simulated_time() + checkpoint_distance_,
				mach_id
			);
		}
	}


	/// Schedule a task.
	private: void schedule(des_engine_context_type& ctx)
	{
		if (waiting_queue_.empty())
		{
			return;
		}

		uint_type free_machine = 0;
		bool found = false;

		for (uint_type i=0; i < machines_.size() && !found; ++i)
		{
			if (!machines_[i]->busy)
			{
				found=true;
				free_machine = i;
			}
		}

		if (found)
		{
			// Get the task at the head of the queue
			::dcs::shared_ptr< task<real_type,uint_type> > to_be_run = waiting_queue_.front();
			waiting_queue_.pop();
			to_be_run->queue_time = des_engine_.simulated_time() - to_be_run->arrival_time;
			machines_[free_machine]->current_task = to_be_run;
			machines_[free_machine]->busy = true;

			if (to_be_run->remaining_time <= checkpoint_distance_)
			{
				ctx.schedule_event(
					ptr_task_departure_evt_src_,
					ctx.simulated_time() + to_be_run->remaining_time,
					free_machine
				);
			}
			else
			{
				ctx.schedule_event(
					ptr_task_suspend_evt_src_,
					ctx.simulated_time() + checkpoint_distance_,
					free_machine
				);
			}
		}
	}


	// Data members


	private: ::std::vector< dcs::shared_ptr< machine<real_type,uint_type> > > machines_;
	//private: ::dcs::math::random::uniform_01_generator< ::dcs::math::random::mt19937, real_type > rng_;
	private: ::dcs::math::random::mt19937 rng_;
	private: ::dcs::math::stats::exponential_distribution<real_type> arrival_;
	private: ::dcs::math::stats::discrete_uniform_distribution<uint_type, real_type> task_size_;
	private: real_type checkpoint_time_;
	private: uint_type checkpoint_distance_;
	private: des_engine_type des_engine_;
	private: ::dcs::shared_ptr<des_event_source_type> ptr_task_arrival_evt_src_;
	private: ::dcs::shared_ptr<des_event_source_type> ptr_task_departure_evt_src_;
	private: ::dcs::shared_ptr<des_event_source_type> ptr_task_suspend_evt_src_;
	private: ::dcs::shared_ptr<des_event_source_type> ptr_task_restart_evt_src_;
	private: ::std::queue< ::dcs::shared_ptr< task<real_type,uint_type> > > waiting_queue_;
	private: ::dcs::shared_ptr< output_statistic_type > ptr_avg_task_completion_time_;
	private: ::dcs::shared_ptr< output_statistic_type > ptr_avg_task_waiting_time_;
};


template <typename RealT, typename UIntT>
const UIntT system<RealT,UIntT>::default_num_machines = 16u;

template <typename RealT, typename UIntT>
const UIntT system<RealT,UIntT>::default_seed = 1435748658uL;

template <typename RealT, typename UIntT>
/*const*/ RealT system<RealT,UIntT>::default_arrival_rate = 0.02;

template <typename RealT, typename UIntT>
const UIntT system<RealT,UIntT>::default_min_task_size = 60;

template <typename RealT, typename UIntT>
const UIntT system<RealT,UIntT>::default_max_task_size = 600;

template <typename RealT, typename UIntT>
/*const*/ RealT system<RealT,UIntT>::default_checkpoint_time = 10;

template <typename RealT, typename UIntT>
const UIntT system<RealT,UIntT>::default_checkpoint_distance = 600;

}}}} // Namespace dcs::examples::des::simple_simulator


#endif // DCS_EXAMPLES_DES_SIMPLE_SIMULATOR_SYSTEM_HPP
