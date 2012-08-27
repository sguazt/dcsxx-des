/**
 * \file dcs/des/batch_means/pawlikowski1990_batch_size_detector.hpp
 *
 * \brief Sequential procedure for batch size detection.
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

#ifndef DCS_DES_BATCH_MEANS_PAWLIKOWSKI1990_BATCH_SIZE_DETECTOR_HPP
#define DCS_DES_BATCH_MEANS_PAWLIKOWSKI1990_BATCH_SIZE_DETECTOR_HPP


#include <boost/numeric/ublas/expression_types.hpp>
#include <boost/numeric/ublas/traits.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublasx/operation/size.hpp>
#include <cmath>
#include <cstddef>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/weighted_mean_estimator.hpp>
#include <dcs/math/constants.hpp>
#include <dcs/math/function/sqr.hpp>
#include <dcs/math/stats/distribution/normal.hpp>
#include <stdexcept>
#include <vector>


namespace dcs { namespace des { namespace batch_means {

namespace detail { namespace /*<unnamed>*/ {

/**
 * \brief Calculate an estimator of the autocovariance of lag \a k for the
 *  values in the vector \a x from \a n0 to \a n1 - 1.
 */
template <typename RealT, typename VectorT, typename UIntT>
//static RealT autocovariance(VectorT const& x, UIntT k, UIntT n0, UIntT n1)
static RealT autocovariance(::boost::numeric::ublas::vector_expression<VectorT> const& x, UIntT k)
{
	namespace ublas = ::boost::numeric::ublas;
	namespace ublasx = ::boost::numeric::ublasx;

	typedef RealT real_type;
	typedef typename ublas::promote_traits<
				UIntT,
				typename ublas::vector_traits<VectorT>::size_type
			>::promote_type size_type;

	//size_type kb = (n1 - n0);
	size_type kb = ublasx::size(x);

	real_type sum = 0;
	for (size_type i = 0; i < kb; ++i)
	{
		sum += x()(i);
	}
	real_type mean = sum / real_type(kb);

	sum = 0;
	for (size_type i = k; i < kb; ++i)
	{
		sum += (x()(i) - mean) * (x()(i-k) - mean);
	}

	return sum / real_type(kb - k);
}


/**
 * \brief Calculate ordinary estimator of autocorrelation
 *  coefficient of lag \a k for the values in the \a x.
 */
template <typename RealT, typename VectorT, typename UIntT>
//static RealT autocorrelation(VectorT const& v, UIntT k, UIntT n0, UIntT n1)
static RealT autocorrelation(::boost::numeric::ublas::vector_expression<VectorT> const& x, UIntT k)
{
	//return autocovariance<RealT, UIntT, VectorT>(x, k, n0, n1) / autocovariance<RealT, UIntT, VectorT>(x, 0, n0, n1);
	return autocovariance<RealT>(x, k) / autocovariance<RealT>(x, 0);
}


/**
 * \brief Calculate jacknife estimator of autocorrelation coefficient of lag
 *  \a k for the values in the vector \a x.
 */
template <typename RealT, typename VectorT, typename UIntT>
//static RealT autocorrelation_jacknife_estimator(VectorT const& x, UIntT k, UIntT N) 
static RealT autocorrelation_jacknife_estimator(::boost::numeric::ublas::vector_expression<VectorT> const& x, UIntT k) 
{
	namespace ublas = ::boost::numeric::ublas;
	namespace ublasx = ::boost::numeric::ublasx;

	typedef typename ublas::promote_traits<
				UIntT,
				typename ublas::vector_traits<VectorT>::size_type
			>::promote_type size_type;

	size_type N = ublasx::size(x);
	size_type n = N / 2;
	return RealT(2)
			//* autocorrelation<RealT>(x, k, 0, N)
			* autocorrelation<RealT>(x, k)
			- (
				//autocorrelation<RealT>(x, k, 0, n)
				autocorrelation<RealT>(
					ublas::subrange(x(), 0, n),
					k
				)
				//+ autocorrelation<RealT>(x, k, n, N)
				+ autocorrelation<RealT>(
					ublas::subrange(x(), n, N),
					k
				)
			  )
			/ RealT(2);
}

}} // Namespace detail::<unnamed>


/**
 * \brief Sequential procedure for batch size detection.
 *
 * \tparam RealT The type used for real numbers.
 * \tparam UIntT The type used for unsigned integral numbers.
 *
 * This batch size detection is based on methods described in
 *
 *  K. Pawlikowski. "Steady state simulation of queueing processes: A survey of
 *  problems and solutions", ACM Computing Surveys, 22(2):123-170 (1990)
 *
 * \author Cosimo Anglano (cosimo.anglano@di.unipmn.it)
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename RealT=double, typename UIntT=::std::size_t>
class pawlikowski1990_batch_size_detector
{
	public: typedef RealT real_type;
	public: typedef UIntT uint_type;
	public: typedef ::std::vector<real_type> vector_type;
	private: typedef ::boost::numeric::ublas::vector<real_type> internal_vector_type;


	/// Constant for setting the duration of batch size determination
	/// to infinity.
	//Note: the use of a type-dependent value for representing the infinity
	//constant is not a limitation to the simulation length since if we exceed
	//that value the observation counter will overflow
	//public: static const uint_type num_obs_infinity = 0;
	public: static const uint_type num_obs_infinity; // = ::dcs::math::constants::infinity<uint_type>::value;


	// Default values according to [Pawlikowski, 1990].

	public: static const uint_type default_m0 = 50;
	public: static const uint_type default_k_b0 = 100;
	public: static const real_type default_beta;// = 0.1;
	//public: static const uint_type default_n_max = default_m0*default_k_b0;
	public: static const uint_type default_n_max; // = num_obs_infinity;


	/**
	 * \brief A constructor.
	 *
	 * \param m0 Initial batch size. The final batch size chosen will be a
	 *  multiple of this size.
	 * \param k_b0 Length of the sequence of batch means tested for
	 *  autocorrelation during the batch size selection phase.
	 * \param beta Significance level below which autocorrelation between batch
	 *  means is considered small enough to accept the batch size being tested.
	 */
	public: explicit pawlikowski1990_batch_size_detector(uint_type n_max=default_n_max, uint_type m0=default_m0, uint_type k_b0=default_k_b0, real_type beta=default_beta)
		: batch_num_obs_(0),
		  tot_num_obs_(0),
		  m0_(m0),
		  m_star_(m0),
		  s_(1),
		  acceptable_size_(false),
		  k_b0_(k_b0),
		  cur_anal_seq_len_(0),
		  anal_seq_(k_b0),
		  cur_ref_seq_len_(0),
		  //ref_seq_len_incr_(10*k_b0),
		  ref_seq_len_incr_(0.1*k_b0*m0),
		  //ref_seq_(k_b0*m0),
		  ref_seq_(k_b0),
		  //ref_seq_(ref_seq_len_incr_),
		  //ref_seq_sum_(0),
		  //ref_seq_sum_sq_(0),
		  //batch_mean_(0),
		  batch_size_detected_(false),
		  beta_(beta),
		  n_max_(n_max),
		  detect_aborted_(false)
	{
		//post-conditions
		DCS_ASSERT(
			beta_ > 0 && beta_ < 1,
			throw ::std::range_error("Autocorrelation significance level is out of range.")
		);

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
		if (!batch_size_detected_)
		{
			if (n_max_ != num_obs_infinity && tot_num_obs_ >= n_max_)
			{
				DCS_DEBUG_TRACE("Batch size detection aborted: reached maximum number of observations.");
				detect_aborted_ = true;
				return false;
			}

#ifdef DCS_DEBUG
//			++tot_num_obs_;
			// Don't print too many times ;)
			if ((tot_num_obs_ % 1000) == 0)
			{
				DCS_DEBUG_TRACE("Batch size determination is in progress (total observation #" << tot_num_obs_ << " / batch observation #: " << (batch_num_obs_+1) << "). Next check at " << (s_*m0_*k_b0_));
			}
#endif // DCS_DEBUG

			++tot_num_obs_;

			//batch_mean_ += obs;
			batch_mean_(obs, weight);
			++batch_num_obs_;

			//DCS_DEBUG_TRACE("Batch Detection - Observation["<<batch_num_obs_<<"/"<<tot_num_obs_<<"]: " << obs << " ---> SUM: " << batch_mean_ );//XXX

//FIXME: these are two alternatives; the first (true part) collect at most
// m0_*k_b0_ observations into the Reference Sequence. The second one (false
// part) allows the Reference Sequence to indefinitively grow.
// At the moment we prefer the first one since avoid memory exhaustion.
#if true
			if (batch_num_obs_ <= m0_*k_b0_)
			{
				if ((batch_num_obs_ % m0_) == 0)
				{
//					// Check if there is enough room in the Reference Sequence.
//					if (cur_ref_seq_len_ == ref_seq_.size())
//					{
//						ref_seq_.resize(ref_seq_.size() + ref_seq_len_incr_);
//					}

					// Compute batch mean and append to the Reference Sequence..
					////batch_mean_ /= real_type(batch_num_obs_);
					//batch_mean_ /= real_type(m0_);
					if (cur_ref_seq_len_ == ref_seq_.size())
					{
						ref_seq_.resize(ref_seq_.size()+k_b0_, true);
					}
					//ref_seq_(cur_ref_seq_len_) = batch_mean_;
					ref_seq_(cur_ref_seq_len_) = batch_mean_.estimate();
					++cur_ref_seq_len_;

					//DCS_DEBUG_TRACE("Batch[#" << cur_ref_seq_len_ << "]: size = " << m_star_ << ", observations: " << batch_num_obs_ << ", mean = " << batch_mean_);

					//ref_seq_sum_ += batch_mean_;
					//ref_seq_sum_sq_ += detail::sqr(batch_mean);
					// Reset the state of this batch
					//batch_mean_ = 0;
					batch_mean_.reset();
				}
			}
			else
			{
				// Consolidate batches and test them for autocorrelation.

				DCS_DEBUG_TRACE("Batch size determination is in progress. Trying with batch size: " << (s_*m0_));

				batch_num_obs_ = 0;

//				for (uint_type i=0;i<cur_ref_seq_len_;++i)//XXX
//				{//XXX
//					DCS_DEBUG_TRACE("Before Consolidation -- REF_SEQ[" << i << "]: " <<ref_seq_(i));//XXX
//				}//XXX
//				for (uint_type i=0;i<cur_anal_seq_len_;++i)//XXX
//				{//XXX
//					DCS_DEBUG_TRACE("Before Consolidation -- ANAL_SEQ[" << i << "]: " <<anal_seq_(i));//XXX
//				}//XXX

				consolidate_batches();

//				for (uint_type i=0;i<cur_ref_seq_len_;++i)//XXX
//				{//XXX
//					DCS_DEBUG_TRACE("After Consolidation -- REF_SEQ[" << i << "]: " <<ref_seq_(i));//XXX
//				}//XXX
//				for (uint_type i=0;i<cur_anal_seq_len_;++i)//XXX
//				{//XXX
//					DCS_DEBUG_TRACE("After Consolidation -- ANAL_SEQ[" << i << "]: " <<anal_seq_(i));//XXX
//				}//XXX

				bool ok = false;
				if (uncorrelated())
				{
					if (acceptable_size_)
					{
						/// Batches are uncorrelated for two consecutive times
						/// So assume this size is good.
						//accept_batch_size();

						ok = true;
					}
					else
					{
						// Autocorrelations for the current batch size are negligible but
						// they were not for the previous batch size.
						// Thus the next batch size should be considered.

						DCS_DEBUG_TRACE("Batch size: " << s_*m0_ << " acceptable... but need more checking.");

						acceptable_size_ = true;
					}
				}
				else
				{
					DCS_DEBUG_TRACE("Batch size: " << s_*m0_ << " rejected.");
				}

				if (ok)
				{
					DCS_DEBUG_TRACE("Batch size: " << s_*m0_ << " accepted.");

					//batch_mean_ = 0;
					batch_mean_.reset();
					m_star_ = s_*m0_;
					batch_size_detected_ = true;
					//ref_seq_.clear();

					// Copy the analyzed batch means into the Reference Sequence
					// for later use.
					ref_seq_.resize(cur_anal_seq_len_);
					::std::copy(
						anal_seq_.begin(),
						anal_seq_.begin() + cur_anal_seq_len_,
						ref_seq_.begin()
					);
					cur_ref_seq_len_ = cur_anal_seq_len_;
				}
				else
				{
					// Re-initialize the counter variable for the next autocorrelation test
					++s_;
				}
			}
#else // true
			//if (batch_num_obs_ >= m0_)
			//if ((batch_num_obs_ % m0_) == 0)
			if (batch_num_obs_ == m_star_)
			{
				// Collected the needed number of observations to form a batch.
				// Going to process this new batch.
				process_batch_mean();
			}
#endif // true
		}

		return batch_size_detected_;
	}


	/**
	 * Tells if the batch size has been detected.
	 *
	 * \return \c true if batch size has been detected; \c false otherwise.
	 */
	public: bool detected() const
	{
		return batch_size_detected_;
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
		return detect_aborted_;
	}


	/// Reset the state of the detector.
	public: void reset()
	{
		batch_num_obs_ = cur_anal_seq_len_
					   = cur_ref_seq_len_
					   = uint_type(0);

		s_ = uint_type(1);

//#ifdef DCS_DEBUG
		tot_num_obs_ = uint_type(0);
//#endif // DCS_DEBUG

		m_star_ = m0_;

		//batch_mean_ = real_type(0);
		batch_mean_.reset();

		acceptable_size_ = batch_size_detected_
						 = detect_aborted_
						 = false;

		//anal_seq_.resize(k_b0_);
		anal_seq_.clear();
		//ref_seq_.resize(ref_seq_len_incr_, false);
		ref_seq_.resize(k_b0_*m0_, false);
		ref_seq_.clear();
	}


//	//[XXX]: deprecated by estimated_size
//	/// \deprecated by estimated_size
//	/// \todo Decide what to do when this method is called and acceptable_size_
//	///  is \c false.
//	public: uint_type estimated_batch_size() const
//	{
//		return m_star_;
//	}
//	//[/XXX]


	/// \todo Decide what to do when this method is called and acceptable_size_
	///  is \c false.
	public: uint_type estimated_size() const
	{
		return m_star_;
	}


//	//[XXX]: deprecated by computed_means
//	/// \deprecated by computed_estimators
//	public: vector_type const& batch_means() const
//	{
//		return ref_seq_;
//	}
//	//[/XXX]


	public: vector_type computed_estimators() const
	{
		return vector_type(ref_seq_.begin(), ref_seq_.end());
	}


	/**
	 * \brief Append current batch mean to the Reference Sequence and test for
	 *  autocorrelation with already collected batch means.
	 */
	private: void process_batch_mean()
	{
		// Check if there is enough room in the Reference Sequence.
		if (cur_ref_seq_len_ == ref_seq_.size())
		{
			ref_seq_.resize(ref_seq_.size() + ref_seq_len_incr_);
		}

		// Compute batch mean and append to the Reference Sequence..
		//batch_mean_ /= real_type(batch_num_obs_);
		//ref_seq_(cur_ref_seq_len_) = batch_mean_;
		ref_seq_(cur_ref_seq_len_) = batch_mean_.reset();
		++cur_ref_seq_len_;

		//DCS_DEBUG_TRACE("Batch[#" << cur_ref_seq_len_ << "]: size = " << m_star_ << ", observations: " << batch_num_obs_ << ", mean = " << batch_mean_);

		//ref_seq_sum_ += batch_mean_;
		//ref_seq_sum_sq_ += detail::sqr(batch_mean);
		// Reset the state of this batch
		//batch_mean_ = 0;
		batch_mean_.reset();
		batch_num_obs_ = 0;
		if (cur_ref_seq_len_ == (s_ * k_b0_))
		{
			// Consolidate batches and test them for autocorrelation.

			DCS_DEBUG_TRACE("Batch size determination is in progress. Trying with batch size: " << (s_*m0_));

			test_for_correlation();
		}
	}


	/*
	 *   Consolidate batches and test them for autocorrelation.
	 */
	private: void test_for_correlation()
	{
		consolidate_batches();

		if (uncorrelated())
		{
			if (acceptable_size_)
			{
				/// Batches are uncorrelated for two consecutive times
				/// So assume this size is good.
				accept_batch_size();

				DCS_DEBUG_TRACE("Batch size: " << s_*m0_ << " accepted.");

				return;
			}

			// Autocorrelations for the current batch size are negligible but
			// they were not for the previous batch size.
			// Thus the next batch size should be considered.
	
			DCS_DEBUG_TRACE("Batch size: " << s_*m0_ << " acceptable... but need more checking.");

			acceptable_size_ = true;
		}

		DCS_DEBUG_TRACE("Batch size: " << s_*m0_ << " rejected.");

		// Re-initialize the counter variable for the next autocorrelation test
		++s_;
	}


	/**
	 * \brief Consolidate groups of \c s_ batch means from the "Reference
	 *  Sequence" into the "Analyzed Sequence" (which will be tested for
	 *  autocorrelation).
	 */
	private: void consolidate_batches()
	{
		cur_anal_seq_len_ = 0;
		uint_type i = 0;
		while (cur_anal_seq_len_ < k_b0_)
		{
			real_type sum = 0;
			for (uint_type j = 0; j < s_; ++j)
			{
				//sum += ref_seq_(cur_anal_seq_len_*s_+j);
				sum += ref_seq_(i);
				++i;
			}
			anal_seq_(cur_anal_seq_len_) = sum / real_type(s_);
			++cur_anal_seq_len_;
		}
	}


	/**
	 * \brief Test whether the batch means in the "Analyzed Sequence" are
	 *  sufficiently uncorrelated.
	 */
	private: bool uncorrelated()
	{
		// Note: In the original paper this test is done over k_b0_ batch
		// means. However, here we are performing the test in an incremental
		// way, so we take only cur_anal_seq_len_ batch means.
		uint_type k_b0 = cur_anal_seq_len_;

		// The number of correlation coefficients to compute.
		// If autocorrelation coefficients monotonically decrease with the value
		// of lag then L = 1 else L = 0.1*k_b0
		uint_type L = k_b0 / 10;

		// Test whether all L autocorrelation coefficients are statistically
		// negligible each at the beta_k significance level

		real_type beta_k = beta_ / real_type(L);
		real_type z;
		internal_vector_type r(L);

		::dcs::math::stats::normal_distribution<real_type> n01_dist;
		z = n01_dist.quantile(real_type(1)-beta_k/real_type(2));

		for (uint_type k = 0; k < L; ++k)
		{
			// compute the jacknife estimator of the autocorrelation coefficient
			// at lag k
//			r(k) = detail::autocorrelation_jacknife_estimator<real_type, uint_type, internal_vector_type>(
//				anal_seq_,
//				k,
//				k_b0
//			);
			r(k) = detail::autocorrelation_jacknife_estimator<real_type>(
				::boost::numeric::ublas::subrange(anal_seq_, 0, k_b0),
				k+1
			);
		}
		for (uint_type k = 0; k < L; ++k)
		{
			real_type sigma_sq;

			if (k == 0)
			{
				sigma_sq = real_type(1) / real_type(k_b0);
			}
			else
			{
				real_type sum = 0;
				//uint_type u_end = k-1;
				////for (uint_type u = 1; u < k; ++u)
				//for (uint_type u = 0; u < u_end; ++u)
				for (uint_type u = 0; u < k; ++u)
				{
					sum += ::dcs::math::sqr(r(u));
				}

				sigma_sq = (real_type(1) + real_type(2) * sum) / real_type(k_b0);
			}

			real_type threshold = z * ::std::sqrt(sigma_sq);

			//DCS_DEBUG_TRACE("Uncorrelated?: K_B0: "<<k_b0<<" - L: "<<L<< " - beta: " <<beta_<< " - t: " << (real_type(1)-beta_k/real_type(2)) <<" - z: " <<z<< " - r["<<k<< "]: "<<r(k)<<" - sigma_sq: " << sigma_sq << " - threshold: " <<threshold);//XXX

			if (::std::abs(r(k)) < threshold)
			{
				// the lag k autocorrelation is statistically negligible
				// at the confidence level 1 - beta_k
				continue;
			}
			else
			{
				// Exit as soon as find a k such that autocorrelation is
				// non-negligible
				return false;
			}
		}

		return true;
	}


//	/**
//	 * \brief Calculate jacknife estimator of autocorrelation
//	 *  coefficient of lag k for the values in the anal_seq_.
//	 */
//	private: real_type jacknife_estimator(uint_type k) const
//	{
//		uint_type N = cur_anal_seq_len_;
//		uint_type n = N / 2;
//		//return real_type(2) * autocorrelation(k, 0, N) - (autocorrelation(k, 0, n) + autocorrelation(k, n, N))/real_type(2);
//		return real_type(2) * detail::autocorrelation(anal_seq_, k, 0, N) - (detail::autocorrelation(anal_seq_, k, 0, n) + detail::autocorrelation(anal_seq_, k, n, N))/real_type(2);
//	}


//	/**
//	 * \brief Calculate ordinary estimator of autocorrelation
//	 *  coefficient of lag k for the values in the anal_seq_
//	 *  from n0 to n1-1.
//	 */
//	private: real_type autocorrelation(uint_type k, uint_type n0, uint_type n1)
//	{
//		//return autocovariance(k, n0, n1) / autocovariance(0, n0, n1);
//		return detail::autocovariance<real_type, uint_type, internal_vector_type>(anal_seq_, k, n0, n1) / detail::autocovariance<real_type, uint_type, internal_vector_type>(anal_seq_, 0, n0, n1);
//	}


//	/**
//	 * \brief Calculate an estimator of the autocovariance of
//	 *  lag k for the values in the anal_seq_ from n0 to n1-1.
//	 */
//	private: real_type autocovariance(uint_type k, uint_type n0, uint_type n1)
//	{
//		uint_type kb = (n1 - n0);
//		::dcs::math::la::vector_range<internal_vector_type> x(::dcs::math::la::subrange(anal_seq_, n0, anal_seq_.size()));
//
//		real_type sum = 0;
//		for (uint_type i = 0; i < kb; ++i)
//		{
//			sum += x(i);
//		}
//		real_type mean = sum / real_type(kb);
//
//		sum = 0;
//		for (uint_type i = k; i < kb; ++i)
//		{
//			sum += (x(i) - mean) * (x(i-k) - mean);
//		}
//
//		return sum / real_type(kb - k);
//	}


	/*
	 * \brief Change the batch size being accumulated in the "Reference
	 *  Sequence" to the one currently being used for the "Analyzed Sequence",
	 *  and consolidate the batches in the "Reference Sequence" accordingly.
	 *
	 * At the end of this function, if the batch size m_star has been selected,
	 * then the "Reference Sequence" contains s_*k_b0_ batch means of size
	 * m_star/s_ and the "Analyzed Sequence" contains k_b0_ batch means of size
	 * m_star_.
	 */
	private: void accept_batch_size()
	{
		m_star_ *= s_; // ==> m_star = s_*m0
		batch_size_detected_ = true;

		DCS_DEBUG_TRACE("Batch Size " << m_star_ << ", " << tot_num_obs_);

//		for (uint_type i = 0; i < cur_anal_seq_len_; ++i)
//		{
//			ref_seq_(i) = anal_seq_(i);
//		}
		::std::copy(
			anal_seq_.begin(),
			anal_seq_.begin() + cur_anal_seq_len_,
			ref_seq_.begin()
		);
		cur_ref_seq_len_ = cur_anal_seq_len_;
		//ref_seq_sum_ /= s_;
		//ref_seq_sum_sq_ = sum_squares(ref_seq_, cur_ref_seq_len_);
	}


//	/**
//	 * \brief Calculate sum of squares of given sequence.
//	 */
//	private: real_type sum_squares(internal_vector_type const& x, uint_type n)
//	{
//		real_type sum_sq = 0;
//		for (uint_type i = 0; i < n; ++i)
//		{
//			sum_sq += ::dcs::math::sqr(x(i));
//		}
//		return sum_sq;
//	}


	/// Number of observation inside a single batch.
	private: uint_type batch_num_obs_;
//#ifdef DCS_DEBUG
	/// Total number of collected observations.
	private: uint_type tot_num_obs_;
//#endif //DCS_DEBUG
	/// Initial size of the batch. The final batch size will be a multiple of
	/// this size.
	private: /*const*/ uint_type m0_;
	/// Estimated size of the current batch.
	private: uint_type m_star_;
	/// The "sequential step": number of entries of the "Reference Sequence"
	/// consolidated into each entry of the "Analyzed Sequence".
	private: uint_type s_;
	/// Tell if an acceptable batch size has been reached.
	private: bool acceptable_size_;
	/// Maximum number of batch means stored in the "Analyzed Sequence".
	private: /*const*/ uint_type k_b0_;
	/// Current number of batch means inserted into the "Analyzed Sequence".
	private: uint_type cur_anal_seq_len_;
	/// The "Analized Sequence": holds batch means to be analyzed
	private: internal_vector_type anal_seq_;
	///// Maximum length of the "Reference Sequence".
	//private: uint_type; max_ref_seq_len_;
	/// Current number of batch means inserted into the "Reference Sequence".
	private: uint_type cur_ref_seq_len_;
	/// Amount by which increment the size of the "Reference Sequence".
	private: /*const*/ uint_type ref_seq_len_incr_;
	/// The "Reference Sequence": holds batch means
	private: internal_vector_type ref_seq_;
	///// Sum of the collected observations.
	//private: real_type ref_seq_sum_;
	///// Sum of the collected observations.
	//private: real_type ref_seq_sum_sq_;
	/// Batch mean of the collected observations.
	//private: real_type batch_mean_;
	private: weighted_mean_estimator<real_type,uint_type> batch_mean_;
	/// Tells if batch size is still to be detected
	private: bool batch_size_detected_;
	/// Significance level threshold of autocorrelation
	private: /*const*/ real_type beta_;
	/// The maximum number of observations to look for batch size detection.
	private: /*const*/ uint_type n_max_;
	/// Tells if batch size detection has been aborted without finding a
	/// right batch size.
	private: bool detect_aborted_;

};

template <typename RealT, typename UIntT>
const RealT pawlikowski1990_batch_size_detector<RealT,UIntT>::default_beta = RealT(0.1);

template <typename RealT, typename UIntT>
const UIntT pawlikowski1990_batch_size_detector<RealT,UIntT>::num_obs_infinity = ::dcs::math::constants::infinity<UIntT>::value;

template <typename RealT, typename UIntT>
const UIntT pawlikowski1990_batch_size_detector<RealT,UIntT>::default_n_max = pawlikowski1990_batch_size_detector<RealT,UIntT>::num_obs_infinity;

}}} // Namespace dcs::des::batch_means


#endif // DCS_DES_BATCH_MEANS_PAWLIKOWSKI1990_BATCH_SIZE_DETECTOR_HPP
