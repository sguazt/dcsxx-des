#include <cstddef>
#include <dcs/des/base_statistic.hpp>
#include <dcs/des/replications/engine.hpp>
#include <dcs/des/model/queue/queue_M_M_1.hpp>
#include <dcs/des/model/queue/queue_statistics.hpp>
#include <dcs/memory.hpp>
#include <dcs/math/random/mersenne_twister.hpp>
#include <iostream>


const unsigned long seed = 5489UL;


int main()
{
	typedef double real_type;
	typedef std::size_t uint_type;
	typedef dcs::math::random::mt19937 random_generator_type;
	typedef dcs::des::replications::engine<real_type,uint_type> des_engine_type;
	typedef dcs::des::base_statistic<real_type,uint_type> statistic_type;
	typedef dcs::des::base_analyzable_statistic<real_type,uint_type> analyzable_statistic_type;
	typedef dcs::des::model::queue_M_M_1<real_type,
										 uint_type,
										 random_generator_type,
										 des_engine_type,
										 statistic_type> queue_type;


	dcs::shared_ptr<random_generator_type> ptr_rng = dcs::make_shared<random_generator_type>(seed);
	dcs::shared_ptr<des_engine_type> ptr_eng = dcs::make_shared<des_engine_type>(10000, 5);

	real_type lambda(2);
	real_type mu(3);

	queue_type queue(lambda, mu, *ptr_rng, ptr_eng);

	dcs::shared_ptr<analyzable_statistic_type> ptr_rt_stat = ptr_eng->make_analyzable_statistic(dcs::des::mean_estimator<real_type,uint_type>());
	queue.statistic(dcs::des::model::response_time_queue_statistic, ptr_rt_stat);

	dcs::shared_ptr<analyzable_statistic_type> ptr_tput_stat = ptr_eng->make_analyzable_statistic(dcs::des::mean_estimator<real_type,uint_type>());
	queue.statistic(dcs::des::model::throughput_queue_statistic, ptr_tput_stat);

	ptr_eng->run();

	std::cout << "MEASURED PERFORMANCE INDICES:" << std::endl;
	std::cout << "  Response Time: " << *ptr_rt_stat << ::std::endl;
	std::cout << "  Throughput: " << *ptr_tput_stat << ::std::endl;
	std::cout << "EXPECTED PERFORMANCE INDICES:" << std::endl;
	std::cout << "  Response Time: " << (real_type(1)/(mu*(1-lambda/mu))) << ::std::endl;
	std::cout << "  Throughput: " << lambda << ::std::endl;
}
