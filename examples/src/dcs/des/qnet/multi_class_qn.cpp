#include <boost/smart_ptr.hpp>
//#include <boost/numeric/ublas/matrix.hpp>
#include <cstddef>
//#include <dcs/des/model/qn/customer_class.hpp>
//#include <dcs/des/model/qn/fcfs_policy.hpp>
//#include <dcs/des/model/qn/is_station.hpp>
#include <dcs/des/base_statistic.hpp>
#include <dcs/des/mean_estimator.hpp>
#include <dcs/des/model/qn/base_routing_strategy.hpp>
#include <dcs/des/model/qn/customer_class.hpp>
#include <dcs/des/model/qn/open_customer_class.hpp>
#include <dcs/des/model/qn/closed_customer_class.hpp>
#include <dcs/des/model/qn/fcfs_queueing_strategy.hpp>
#include <dcs/des/model/qn/load_independent_service_strategy.hpp>
#include <dcs/des/model/qn/network_node.hpp>
#include <dcs/des/model/qn/probabilistic_routing_strategy.hpp>
#include <dcs/des/model/qn/queueing_network.hpp>
#include <dcs/des/model/qn/queueing_network_traits.hpp>
#include <dcs/des/model/qn/queueing_station_node.hpp>
#include <dcs/des/model/qn/queueing_strategy.hpp>
//#include <dcs/des/model/qn/service_station.hpp>
//#include <dcs/des/model/qn/service_strategy.hpp>
#include <dcs/des/model/qn/sink_node.hpp>
#include <dcs/des/model/qn/source_node.hpp>
#include <dcs/des/null_transient_detector.hpp>
#include <dcs/des/replications/constant_num_replications_detector.hpp>
//#include <dcs/des/replications/constant_replication_size_detector.hpp>
#include <dcs/des/replications/fixed_duration_replication_size_detector.hpp>
#include <dcs/des/replications/engine.hpp>
#include <dcs/math/constants.hpp>
#include <dcs/math/random/mersenne_twister.hpp>
#include <dcs/math/stats/distribution/exponential.hpp>
#include <dcs/math/stats/distribution/pareto.hpp>
#include <iostream>


//namespace ublas = boost::numeric::ublas;
namespace des = dcs::des;
namespace des_qn = dcs::des::model::qn;

const unsigned long seed = 5489UL;


int main()
{
	typedef double real_type;
	typedef std::size_t uint_type;
	//typedef ublas::matrix<real_type> matrix_type;
	typedef des::replications::engine<real_type> des_engine_type;
	//typedef des_engine_type::event_type des_event_type;
	typedef dcs::math::random::mt19937 random_generator_type;
	typedef des_qn::queueing_network<
				real_type,
				random_generator_type,
				des_engine_type
			> network_type;
	typedef des_qn::queueing_network_traits<network_type> network_traits_type;
	typedef boost::shared_ptr< des_qn::customer_class<network_traits_type> > customer_class_pointer;
	typedef boost::shared_ptr< des_qn::network_node<network_traits_type> > network_node_pointer;
	typedef boost::shared_ptr< des_qn::queueing_strategy<network_traits_type> > queueing_strategy_pointer;
	typedef boost::shared_ptr< des_qn::base_service_strategy<network_traits_type> > service_strategy_pointer;
	typedef des::base_statistic<real_type> output_statistic_type;
	typedef boost::shared_ptr<output_statistic_type> output_statistic_pointer;


	// Create the simulator engine

	::boost::shared_ptr<des_engine_type> ptr_des_engine(new des_engine_type());
	::boost::shared_ptr<random_generator_type> ptr_rng(new random_generator_type(seed));


	// Create the Queueing Network

	//uint_type node_id = 0;
	//uint_type class_id = 0;
	//uint_type num_classes = 2;
	//uint_type num_nodes = 2;

	enum customer_classes { OPEN_CLASS, CLOSED_CLASS };
	enum network_nodes { SOURCE_NODE, WEB_SERVER_NODE, APP_SERVER_NODE, DB_SERVER_NODE, SINK_NODE };

	//network_type qn(num_classes, num_stations, ptr_rng, ptr_des_engine);
	network_type qn(ptr_rng, ptr_des_engine);

	// Set-up routing
	boost::shared_ptr< des_qn::probabilistic_routing_strategy<network_traits_type> > ptr_routing;
	ptr_routing = boost::make_shared< des_qn::probabilistic_routing_strategy<network_traits_type> >(ptr_rng);
	ptr_routing->add_route(SOURCE_NODE, OPEN_CLASS, WEB_SERVER_NODE, OPEN_CLASS, real_type(1));
	ptr_routing->add_route(WEB_SERVER_NODE, OPEN_CLASS, APP_SERVER_NODE, OPEN_CLASS, real_type(1));
	ptr_routing->add_route(WEB_SERVER_NODE, OPEN_CLASS, SINK_NODE, OPEN_CLASS, real_type(1));
	ptr_routing->add_route(APP_SERVER_NODE, OPEN_CLASS, DB_SERVER_NODE, OPEN_CLASS, real_type(1));
	ptr_routing->add_route(APP_SERVER_NODE, OPEN_CLASS, WEB_SERVER_NODE, OPEN_CLASS, real_type(1));
	ptr_routing->add_route(APP_SERVER_NODE, OPEN_CLASS, SINK_NODE, OPEN_CLASS, real_type(1));
	ptr_routing->add_route(DB_SERVER_NODE, OPEN_CLASS, APP_SERVER_NODE, OPEN_CLASS, real_type(1));
	ptr_routing->add_route(DB_SERVER_NODE, OPEN_CLASS, SINK_NODE, OPEN_CLASS, real_type(1));

	// Set-up nodes

	network_node_pointer ptr_node;
	std::vector< dcs::math::stats::any_distribution<real_type> > svc_distrs;

	ptr_node = network_node_pointer(
			new des_qn::source_node<network_traits_type>(
					SOURCE_NODE,
					"Source",
					ptr_routing
			)
		);
	qn.add_node(ptr_node);

	ptr_node = network_node_pointer(
			new des_qn::sink_node<network_traits_type>(
				SINK_NODE,
				"Sink"
			)
		);
	qn.add_node(ptr_node);

	svc_distrs.clear();
	svc_distrs.push_back(
		dcs::math::stats::make_any_distribution(dcs::math::stats::exponential_distribution<real_type>(1))
	);
	svc_distrs.push_back(
		dcs::math::stats::make_any_distribution(dcs::math::stats::pareto_distribution<real_type>(3, 1))
	);
	ptr_node = network_node_pointer(
			new des_qn::queueing_station_node<network_traits_type>(
				WEB_SERVER_NODE,
				"Web Server",
				queueing_strategy_pointer(
					new des_qn::fcfs_queueing_strategy<network_traits_type>()
				),
				service_strategy_pointer(
					new des_qn::load_independent_service_strategy<network_traits_type>(
						svc_distrs.begin(),
						svc_distrs.end()
					)
				),
				ptr_routing
			)
		);
	qn.add_node(ptr_node);

	svc_distrs.clear();
	svc_distrs.push_back(
		dcs::math::stats::make_any_distribution(dcs::math::stats::exponential_distribution<real_type>(1))
	);
	svc_distrs.push_back(
		dcs::math::stats::make_any_distribution(dcs::math::stats::pareto_distribution<real_type>(3, 1))
	);
	ptr_node = network_node_pointer(
			new des_qn::queueing_station_node<network_traits_type>(
				APP_SERVER_NODE,
				"Application Server",
				queueing_strategy_pointer(
					new des_qn::fcfs_queueing_strategy<network_traits_type>()
				),
				service_strategy_pointer(
					new des_qn::load_independent_service_strategy<network_traits_type>(
						svc_distrs.begin(),
						svc_distrs.end()
					)
				),
				ptr_routing
			)
		);
	qn.add_node(ptr_node);

	svc_distrs.clear();
	svc_distrs.push_back(
		dcs::math::stats::make_any_distribution(dcs::math::stats::exponential_distribution<real_type>(1))
	);
	svc_distrs.push_back(
		dcs::math::stats::make_any_distribution(dcs::math::stats::pareto_distribution<real_type>(3, 1))
	);
	ptr_node = network_node_pointer(
			new des_qn::queueing_station_node<network_traits_type>(
				DB_SERVER_NODE,
				"Database Server",
				queueing_strategy_pointer(
					new des_qn::fcfs_queueing_strategy<network_traits_type>()
				),
				service_strategy_pointer(
					new des_qn::load_independent_service_strategy<network_traits_type>(
						svc_distrs.begin(),
						svc_distrs.end()
					)
				),
				ptr_routing
			)
		);
	qn.add_node(ptr_node);


	// Set-up classes
	customer_class_pointer ptr_class;

	ptr_class = customer_class_pointer(
			new des_qn::open_customer_class<network_traits_type>(
				OPEN_CLASS,
				"Open Class",
				dcs::math::stats::exponential_distribution<real_type>(1.5)
			)
		);
	ptr_class->reference_node(SOURCE_NODE);
	qn.add_class(ptr_class);

//	ptr_class = customer_class_pointer(
//			new des_qn::closed_customer_class<network_traits_type>(
//				CLOSED_CLASS,
//				"Closed Class",
//				10
//			)
//		);
//	qn.add_class(ptr_class);

	// Set some event hook

//	ptr_des_engine->begin_of_replication_event_source().connect(
//		&process_begin_of_replication,
//		dcs::functional::placeholders::_1,
//		dcs::functional::placeholders::_2
//	);
//	ptr_des_engine->end_of_replication_event_source().connect(
//		&process_end_of_replication,
//		dcs::functional::placeholders::_1,
//		dcs::functional::placeholders::_2
//	);

	// Setup output statistics
	const real_type ci_level = 0.95;
	const uint_type repl_size = 1000;
	const uint_type num_repls = 5;
	const real_type rel_prec = dcs::math::constants::infinity<real_type>::value;
	output_statistic_pointer ptr_stat;

//	ptr_stat = output_statistic_pointer(
//		new ::dcs::des::mean_estimator<real_type,uint_type>(ci_level)
//	);
	ptr_stat = des::make_analyzable_statistic(
				des::mean_estimator<real_type>(ci_level),
				des::null_transient_detector<real_type>(),
				des::replications::fixed_duration_replication_size_detector<real_type,des_engine_type>(repl_size, ptr_des_engine),
				des::replications::constant_num_replications_detector<real_type>(num_repls),
				*ptr_des_engine,
				rel_prec,
				dcs::math::constants::infinity<uint_type>::value
	);
	qn.statistic(des_qn::net_response_time_statistic_category, ptr_stat);

//	ptr_stat = output_statistic_pointer(
//		new ::dcs::des::mean_estimator<real_type>(ci_level)
//	);
//	ptr_stat = des::make_analyzable_statistic(
//				des::mean_estimator<real_type>(ci_level),
//				des::null_transient_detector<real_type>(),
//				des::replications::fixed_duration_replication_size_detector<real_type,des_engine_type>(repl_size, ptr_des_engine),
//				des::replications::constant_num_replications_detector<real_type>(num_repls),
//				*ptr_des_engine,
//				rel_prec,
//				dcs::math::constants::infinity<uint_type>::value
//	);
//	qn.statistic(des_qn::net_throughput_statistic_category, ptr_stat);

	// Run the simulation

	//ptr_des_engine->stop_at_time(5000);
	ptr_des_engine->run();

	std::cout << "System-wide # Arrivals: " << qn.num_arrivals() << std::endl;
	std::cout << "System-wide # Departures: " << qn.num_departures() << std::endl;
	std::cout << "System-wide # Discards: " << qn.num_discards() << std::endl;
	std::cout << "System-wide Response Time: " << *(qn.statistic(des_qn::net_response_time_statistic_category)[0]) << std::endl;
//	std::cout << "System-wide Throughput: " << *(qn.statistic(des_qn::net_throughput_statistic_category)[0]) << std::endl;
}
