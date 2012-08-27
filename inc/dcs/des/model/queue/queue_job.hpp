/**
 * \file dcs/des/model/queue/queue_job.hpp
 *
 * \brief Model for jobs in queue model.
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
#ifndef DCS_DES_MODEL_QUEUE_JOB_HPP
#define DCS_DES_MODEL_QUEUE_JOB_HPP


#include <iostream>


namespace dcs { namespace des { namespace model {

/**
 * \brief Model for jobs in queue model.
 * \tparam RealT The type used for real numbers.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename RealT>
class queue_job
{
	/// The type used for real numbers;
	typedef RealT real_type;


	public: void arrival_time(real_type value)
	{
		arr_time_ = value;
	}


	public: real_type arrival_time() const
	{
		return arr_time_;
	}


	public: void departure_time(real_type value)
	{
		dep_time_ = value;
	}


	public: real_type departure_time() const
	{
		return dep_time_;
	}


	public: void waiting_time(real_type value)
	{
		wait_time_ = value;
	}


	public: real_type waiting_time() const
	{
		return wait_time_;
	}


	public: void service_time(real_type value)
	{
		svc_time_ = value;
	}


	public: real_type service_time() const
	{
		return svc_time_;
	}


	public: real_type response_time() const
	{
		return dep_time_ - arr_time_;
	}


	private: real_type arr_time_;
	private: real_type dep_time_;
	private: real_type svc_time_;
	private: real_type wait_time_;
};


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, queue_job<RealT> const& job)
{
	os 	<< "<"
		<< "arrival:" << job.arrival_time()
		<< ", waiting:" << job.waiting_time()
		<< ", service:" << job.service_time()
		<< ", response:" << job.response_time()
		<< ", departure:" << job.departure_time()
		<< ">"
		<< ::std::flush;
	return os;
}

}}} // Namespace dcs::des::model


#endif // DCS_DES_MODEL_QUEUE_JOB_HPP
