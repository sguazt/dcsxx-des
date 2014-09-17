#include <cstddef>
#include <dcs/des/base_analyzable_statistic.hpp>
#include <dcs/des/base_statistic.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/des/mean_estimator.hpp>
#include <dcs/des/quantile_estimator.hpp>
#include <dcs/des/model/qn/fcfs_queueing_strategy.hpp>
#include <dcs/des/model/qn/load_independent_service_strategy.hpp>
#include <dcs/des/model/qn/network_node.hpp>
#include <dcs/des/model/qn/open_customer_class.hpp>
#include <dcs/des/model/qn/probabilistic_routing_strategy.hpp>
#include <dcs/des/model/qn/ps_queueing_strategy.hpp>
#include <dcs/des/model/qn/ps_service_strategy.hpp>
#include <dcs/des/model/qn/queueing_network.hpp>
#include <dcs/des/model/qn/queueing_network_traits.hpp>
#include <dcs/des/model/qn/queueing_station_node.hpp>
#include <dcs/des/model/qn/rr_queueing_strategy.hpp>
#include <dcs/des/model/qn/rr_service_strategy.hpp>
#include <dcs/des/model/qn/sink_node.hpp>
#include <dcs/des/model/qn/source_node.hpp>
#include <dcs/des/model/qn/output_statistic_category.hpp>
#include <dcs/des/replications/engine.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
#include <dcs/math/random/mersenne_twister.hpp>
#include <dcs/math/stats/distributions.hpp>
#include <dcs/memory.hpp>
#include <iostream>
//#include <map>


#define SINGLE_CLASS_TANDEM_FCFS_POLICY 0
#define SINGLE_CLASS_TANDEM_PS_POLICY   1
#define SINGLE_CLASS_TANDEM_RR_POLICY   2

//#undef SINGLE_CLASS_TANDEM_SCHEDULING_POLICY
//#define SINGLE_CLASS_TANDEM_SCHEDULING_POLICY SINGLE_CLASS_TANDEM_FCFS_POLICY
#define SINGLE_CLASS_TANDEM_SCHEDULING_POLICY SINGLE_CLASS_TANDEM_PS_POLICY
//#define SINGLE_CLASS_TANDEM_SCHEDULING_POLICY SINGLE_CLASS_TANDEM_RR_POLICY


enum customer_class_category
{
	OPEN_CLASS
};

enum network_node_category
{
	SOURCE_NODE,
	WEB_SERVER_NODE,
	APP_SERVER_NODE,
	DB_SERVER_NODE,
	SINK_NODE
};


namespace detail { namespace /*<unnamed>*/ {

template <typename EventT, typename ContextT, typename QueueNetT>
void process_sys_init(EventT const& evt, ContextT& ctx, QueueNetT const& qn)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(evt);

	::std::cerr << "BEGIN System Initialization (Clock: " << ctx.simulated_time() << ")" << ::std::endl;

	::std::cerr << "Web Server:" << ::std::endl
				<< "Busy Time: " << qn.get_node(WEB_SERVER_NODE).busy_time() << ::std::endl
				<< "Busy Capacity: " << qn.get_node(WEB_SERVER_NODE).busy_capacity() << ::std::endl;

	::std::cerr << "END System Initialization (Clock: " << ctx.simulated_time() << ")" << ::std::endl;
}

template <typename EventT, typename ContextT, typename QueueNetT>
void process_sys_finit(EventT const& evt, ContextT& ctx, QueueNetT const& qn)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(evt);

	::std::cerr << "BEGIN System Finalization (Clock: " << ctx.simulated_time() << ")" << ::std::endl;

	::std::cerr << "Web Server:" << ::std::endl
				<< "Busy Time: " << qn.get_node(WEB_SERVER_NODE).busy_time() << ::std::endl
				<< "Busy Capacity: " << qn.get_node(WEB_SERVER_NODE).busy_capacity() << ::std::endl;

	::std::cerr << "END System Finalization (Clock: " << ctx.simulated_time() << ")" << ::std::endl;
}

}} // Namespace detail::<unnamed>


int main()
{
	typedef double real_type;
	typedef std::size_t uint_type;
	typedef dcs::math::random::mt19937 random_generator_type;
	typedef dcs::des::replications::engine<real_type> des_engine_type;
	typedef dcs::des::base_analyzable_statistic<real_type> analyzable_statistic_type;
	typedef dcs::des::model::qn::queueing_network<real_type,
												  random_generator_type,
												  des_engine_type> network_type;
	typedef dcs::des::model::qn::queueing_network_traits<network_type> network_traits_type;
	typedef dcs::des::model::qn::open_customer_class<network_traits_type> customer_class_type;
	typedef dcs::des::model::qn::network_node<network_traits_type> network_node_type;
#if defined(SINGLE_CLASS_TANDEM_SCHEDULING_POLICY)
# if SINGLE_CLASS_TANDEM_SCHEDULING_POLICY == SINGLE_CLASS_TANDEM_FCFS_POLICY
	typedef dcs::des::model::qn::fcfs_queueing_strategy<network_traits_type> queueing_strategy_type;
	typedef dcs::des::model::qn::load_independent_service_strategy<network_traits_type> service_strategy_type;
# elif SINGLE_CLASS_TANDEM_SCHEDULING_POLICY == SINGLE_CLASS_TANDEM_PS_POLICY
	typedef dcs::des::model::qn::ps_queueing_strategy<network_traits_type> queueing_strategy_type;
	typedef dcs::des::model::qn::ps_service_strategy<network_traits_type> service_strategy_type;
# elif SINGLE_CLASS_TANDEM_SCHEDULING_POLICY == SINGLE_CLASS_TANDEM_RR_POLICY
	typedef dcs::des::model::qn::rr_queueing_strategy<network_traits_type> queueing_strategy_type;
	typedef dcs::des::model::qn::rr_service_strategy<network_traits_type> service_strategy_type;
# else
#  error "Unknown scheduling policy."
# endif // SINGLE_CLASS_TANDEM_SCHEDULING_POLICY == ...
#else
# error "Scheduling policy not defined."
#endif // defined(SINGLE_CLASS_TANDEM_SCHEDULING_POLICY)
	typedef dcs::des::model::qn::probabilistic_routing_strategy<network_traits_type> routing_strategy_type;
	typedef dcs::math::stats::any_distribution<real_type> probability_distribution_type;


	const unsigned long seed(5489UL);
	const uint_type replication_length(100);
	const uint_type num_replications(5);
	const uint_type num_servers(1);
#if SINGLE_CLASS_TANDEM_SCHEDULING_POLICY == SINGLE_CLASS_TANDEM_RR_POLICY
	const real_type quantum(0.01);
#endif // SINGLE_CLASS_TANDEM_SCHEDULING_POLICY
	const real_type arr_rate(5.3);
	const real_type web_svc_time(0.03);
	const real_type app_svc_time(0.06);
	const real_type db_svc_time(0.03);


	dcs::shared_ptr<des_engine_type> ptr_eng = dcs::make_shared<des_engine_type>(replication_length, num_replications);
	dcs::shared_ptr<random_generator_type> ptr_rng = dcs::make_shared<random_generator_type>(seed);

	// Set-up queueing network
	network_type qn(ptr_rng, ptr_eng);

	// - Set-up network routing
	dcs::shared_ptr<routing_strategy_type> ptr_routing = dcs::make_shared<routing_strategy_type>(ptr_rng);
	ptr_routing->add_route(SOURCE_NODE, OPEN_CLASS, WEB_SERVER_NODE, OPEN_CLASS, 1);
	ptr_routing->add_route(WEB_SERVER_NODE, OPEN_CLASS, APP_SERVER_NODE, OPEN_CLASS, 1);
	ptr_routing->add_route(APP_SERVER_NODE, OPEN_CLASS, DB_SERVER_NODE, OPEN_CLASS, 1);
	ptr_routing->add_route(DB_SERVER_NODE, OPEN_CLASS, SINK_NODE, OPEN_CLASS, 1);

	std::vector<probability_distribution_type> svc_distrs;

	// - Set-up nodes
	dcs::shared_ptr<network_node_type> ptr_node;
	// source node
	ptr_node = dcs::make_shared< dcs::des::model::qn::source_node<network_traits_type> >(
			SOURCE_NODE,
			"Source",
			ptr_routing
		);
	qn.add_node(ptr_node);
	// Web server node
	svc_distrs.clear();
	svc_distrs.push_back(
//		dcs::math::stats::make_any_distribution(dcs::math::stats::degenerate_distribution<real_type>(web_svc_time)));
		dcs::math::stats::make_any_distribution(dcs::math::stats::exponential_distribution<real_type>(1.0/web_svc_time)));
	ptr_node = dcs::make_shared< dcs::des::model::qn::queueing_station_node<network_traits_type> >(
			WEB_SERVER_NODE,
			"Web Server",
			dcs::make_shared<queueing_strategy_type>(),
#if SINGLE_CLASS_TANDEM_SCHEDULING_POLICY == SINGLE_CLASS_TANDEM_RR_POLICY
			dcs::make_shared<service_strategy_type>(quantum, num_servers, svc_distrs.begin(), svc_distrs.end()),
#else
			dcs::make_shared<service_strategy_type>(num_servers, svc_distrs.begin(), svc_distrs.end()),
#endif // SINGLE_CLASS_TANDEM_SCHEDULING_POLICY
			ptr_routing
		);
	qn.add_node(ptr_node);
	// App server node
	svc_distrs.clear();
	svc_distrs.push_back(
//		dcs::math::stats::make_any_distribution(dcs::math::stats::degenerate_distribution<real_type>(app_svc_time)));
		dcs::math::stats::make_any_distribution(dcs::math::stats::exponential_distribution<real_type>(1.0/app_svc_time)));
	ptr_node = dcs::make_shared< dcs::des::model::qn::queueing_station_node<network_traits_type> >(
			APP_SERVER_NODE,
			"App Server",
			dcs::make_shared<queueing_strategy_type>(),
#if SINGLE_CLASS_TANDEM_SCHEDULING_POLICY == SINGLE_CLASS_TANDEM_RR_POLICY
			dcs::make_shared<service_strategy_type>(quantum, num_servers, svc_distrs.begin(), svc_distrs.end()),
#else
			dcs::make_shared<service_strategy_type>(num_servers, svc_distrs.begin(), svc_distrs.end()),
#endif // SINGLE_CLASS_TANDEM_SCHEDULING_POLICY
			ptr_routing
		);
	qn.add_node(ptr_node);
	// DB server node
	svc_distrs.clear();
	svc_distrs.push_back(
//		dcs::math::stats::make_any_distribution(dcs::math::stats::degenerate_distribution<real_type>(db_svc_time)));
		dcs::math::stats::make_any_distribution(dcs::math::stats::exponential_distribution<real_type>(1.0/db_svc_time)));
	ptr_node = dcs::make_shared< dcs::des::model::qn::queueing_station_node<network_traits_type> >(
			DB_SERVER_NODE,
			"DB Server",
			dcs::make_shared<queueing_strategy_type>(),
#if SINGLE_CLASS_TANDEM_SCHEDULING_POLICY == SINGLE_CLASS_TANDEM_RR_POLICY
			dcs::make_shared<service_strategy_type>(quantum, num_servers, svc_distrs.begin(), svc_distrs.end()),
#else
			dcs::make_shared<service_strategy_type>(num_servers, svc_distrs.begin(), svc_distrs.end()),
#endif // SINGLE_CLASS_TANDEM_SCHEDULING_POLICY
			ptr_routing
		);
	qn.add_node(ptr_node);
	// sink node
	ptr_node = dcs::make_shared< dcs::des::model::qn::sink_node<network_traits_type> >(
			SINK_NODE,
			"Sink"
		);
	qn.add_node(ptr_node);

	// - Set-up customr classes
	dcs::shared_ptr<customer_class_type> ptr_class = dcs::make_shared<customer_class_type>(
				OPEN_CLASS,
				"Open Class",
				dcs::math::stats::exponential_distribution<real_type>(arr_rate)
		);
	ptr_class->reference_node(SOURCE_NODE);
	qn.add_class(ptr_class);

	// Set-up statistics

	dcs::shared_ptr<analyzable_statistic_type> ptr_stat;

	// - Network-level statistics

	dcs::shared_ptr<analyzable_statistic_type> ptr_rt_stat = ptr_eng->make_analyzable_statistic(dcs::des::mean_estimator<real_type>());
//	dcs::shared_ptr<analyzable_statistic_type> ptr_rt_stat = ptr_eng->make_analyzable_statistic(dcs::des::quantile_estimator<real_type>(0.99));
	qn.statistic(dcs::des::model::qn::net_response_time_statistic_category, ptr_rt_stat);

	dcs::shared_ptr<analyzable_statistic_type> ptr_tput_stat = ptr_eng->make_analyzable_statistic(dcs::des::mean_estimator<real_type>());
//	dcs::shared_ptr<analyzable_statistic_type> ptr_tput_stat = ptr_eng->make_analyzable_statistic(dcs::des::quantile_estimator<real_type>(0.99));
	qn.statistic(dcs::des::model::qn::net_throughput_statistic_category, ptr_tput_stat);

	ptr_stat = ptr_eng->make_analyzable_statistic(dcs::des::mean_estimator<real_type>());
	qn.statistic(dcs::des::model::qn::net_num_arrivals_statistic_category, ptr_stat);

	ptr_stat = ptr_eng->make_analyzable_statistic(dcs::des::mean_estimator<real_type>());
	qn.statistic(dcs::des::model::qn::net_num_departures_statistic_category, ptr_stat);

	// - Node-level statistics

	ptr_stat = ptr_eng->make_analyzable_statistic(dcs::des::mean_estimator<real_type>());
	qn.get_node(WEB_SERVER_NODE).statistic(dcs::des::model::qn::num_arrivals_statistic_category, ptr_stat);
	ptr_stat = ptr_eng->make_analyzable_statistic(dcs::des::mean_estimator<real_type>());
	qn.get_node(WEB_SERVER_NODE).statistic(dcs::des::model::qn::num_departures_statistic_category, ptr_stat);
	ptr_stat = ptr_eng->make_analyzable_statistic(dcs::des::mean_estimator<real_type>());
	qn.get_node(WEB_SERVER_NODE).statistic(dcs::des::model::qn::busy_time_statistic_category, ptr_stat);
	ptr_stat = ptr_eng->make_analyzable_statistic(dcs::des::mean_estimator<real_type>());
	qn.get_node(WEB_SERVER_NODE).statistic(dcs::des::model::qn::response_time_statistic_category, ptr_stat);
	ptr_stat = ptr_eng->make_analyzable_statistic(dcs::des::mean_estimator<real_type>());
	qn.get_node(WEB_SERVER_NODE).statistic(dcs::des::model::qn::throughput_statistic_category, ptr_stat);
	ptr_stat = ptr_eng->make_analyzable_statistic(dcs::des::mean_estimator<real_type>());
	qn.get_node(WEB_SERVER_NODE).statistic(dcs::des::model::qn::num_waiting_statistic_category, ptr_stat);
	ptr_stat = ptr_eng->make_analyzable_statistic(dcs::des::mean_estimator<real_type>());
	qn.get_node(WEB_SERVER_NODE).statistic(dcs::des::model::qn::utilization_statistic_category, ptr_stat);

	ptr_stat = ptr_eng->make_analyzable_statistic(dcs::des::mean_estimator<real_type>());
	qn.get_node(APP_SERVER_NODE).statistic(dcs::des::model::qn::num_arrivals_statistic_category, ptr_stat);
	ptr_stat = ptr_eng->make_analyzable_statistic(dcs::des::mean_estimator<real_type>());
	qn.get_node(APP_SERVER_NODE).statistic(dcs::des::model::qn::num_departures_statistic_category, ptr_stat);
	ptr_stat = ptr_eng->make_analyzable_statistic(dcs::des::mean_estimator<real_type>());
	qn.get_node(APP_SERVER_NODE).statistic(dcs::des::model::qn::busy_time_statistic_category, ptr_stat);
	ptr_stat = ptr_eng->make_analyzable_statistic(dcs::des::mean_estimator<real_type>());
	qn.get_node(APP_SERVER_NODE).statistic(dcs::des::model::qn::response_time_statistic_category, ptr_stat);
	ptr_stat = ptr_eng->make_analyzable_statistic(dcs::des::mean_estimator<real_type>());
	qn.get_node(APP_SERVER_NODE).statistic(dcs::des::model::qn::throughput_statistic_category, ptr_stat);
	ptr_stat = ptr_eng->make_analyzable_statistic(dcs::des::mean_estimator<real_type>());
	qn.get_node(APP_SERVER_NODE).statistic(dcs::des::model::qn::num_waiting_statistic_category, ptr_stat);
	ptr_stat = ptr_eng->make_analyzable_statistic(dcs::des::mean_estimator<real_type>());
	qn.get_node(APP_SERVER_NODE).statistic(dcs::des::model::qn::utilization_statistic_category, ptr_stat);

	ptr_stat = ptr_eng->make_analyzable_statistic(dcs::des::mean_estimator<real_type>());
	qn.get_node(DB_SERVER_NODE).statistic(dcs::des::model::qn::num_arrivals_statistic_category, ptr_stat);
	ptr_stat = ptr_eng->make_analyzable_statistic(dcs::des::mean_estimator<real_type>());
	qn.get_node(DB_SERVER_NODE).statistic(dcs::des::model::qn::num_departures_statistic_category, ptr_stat);
	ptr_stat = ptr_eng->make_analyzable_statistic(dcs::des::mean_estimator<real_type>());
	qn.get_node(DB_SERVER_NODE).statistic(dcs::des::model::qn::busy_time_statistic_category, ptr_stat);
	ptr_stat = ptr_eng->make_analyzable_statistic(dcs::des::mean_estimator<real_type>());
	qn.get_node(DB_SERVER_NODE).statistic(dcs::des::model::qn::response_time_statistic_category, ptr_stat);
	ptr_stat = ptr_eng->make_analyzable_statistic(dcs::des::mean_estimator<real_type>());
	qn.get_node(DB_SERVER_NODE).statistic(dcs::des::model::qn::throughput_statistic_category, ptr_stat);
	ptr_stat = ptr_eng->make_analyzable_statistic(dcs::des::mean_estimator<real_type>());
	qn.get_node(DB_SERVER_NODE).statistic(dcs::des::model::qn::num_waiting_statistic_category, ptr_stat);
	ptr_stat = ptr_eng->make_analyzable_statistic(dcs::des::mean_estimator<real_type>());
	qn.get_node(DB_SERVER_NODE).statistic(dcs::des::model::qn::utilization_statistic_category, ptr_stat);


//	ptr_eng->system_initialization_event_source().connect(
//			dcs::functional::bind(
//					&detail::process_sys_init<event_type,engine_context_type,network_type>,
//					dcs::functional::placeholders::_1,
//					dcs::functional::placeholders::_2,
//					qn
//				)
//		);

//	ptr_eng->system_finalization_event_source().connect(
//			dcs::functional::bind(
//					&detail::process_sys_finit<event_type,engine_context_type,network_type>,
//					dcs::functional::placeholders::_1,
//					dcs::functional::placeholders::_2,
//					qn
//				)
//		);

	std::cout.precision(16);
	std::cerr.precision(16);

	// Run the simulation
	ptr_eng->run();


	// Report statistics
	std::cout << "MEASURED PERFORMANCE INDICES:" << std::endl;
	std::cout << "  Network-level:" << std::endl;
	std::cout << "    Num Arrivals: " << *(qn.statistic(dcs::des::model::qn::net_num_arrivals_statistic_category)[0]) << ::std::endl;
	std::cout << "    Num Departures: " << *(qn.statistic(dcs::des::model::qn::net_num_departures_statistic_category)[0]) << ::std::endl;
	std::cout << "    Response Time: " << *ptr_rt_stat << ::std::endl;
	std::cout << "    Throughput: " << *ptr_tput_stat << ::std::endl;
	std::cout << "  Node-level:" << std::endl;
	std::cout << "    Web Server: " << ::std::endl;
	std::cout << "      Num Arrivals: " << *(qn.get_node(WEB_SERVER_NODE).statistic(dcs::des::model::qn::num_arrivals_statistic_category)[0]) << ::std::endl;
	std::cout << "      Num Departures: " << *(qn.get_node(WEB_SERVER_NODE).statistic(dcs::des::model::qn::num_departures_statistic_category)[0]) << ::std::endl;
	std::cout << "      Busy Time: " << *(qn.get_node(WEB_SERVER_NODE).statistic(dcs::des::model::qn::busy_time_statistic_category)[0]) << ::std::endl;
	std::cout << "      Utilization: " << *(qn.get_node(WEB_SERVER_NODE).statistic(dcs::des::model::qn::utilization_statistic_category)[0]) << ::std::endl;
	std::cout << "      Response Time: " << *(qn.get_node(WEB_SERVER_NODE).statistic(dcs::des::model::qn::response_time_statistic_category)[0]) << ::std::endl;
	std::cout << "      Throughput: " << *(qn.get_node(WEB_SERVER_NODE).statistic(dcs::des::model::qn::throughput_statistic_category)[0]) << ::std::endl;
	std::cout << "      Queue Length: " << *(qn.get_node(WEB_SERVER_NODE).statistic(dcs::des::model::qn::num_waiting_statistic_category)[0]) << ::std::endl;
	std::cout << "    App Server: " << ::std::endl;
	std::cout << "      Num Arrivals: " << *(qn.get_node(APP_SERVER_NODE).statistic(dcs::des::model::qn::num_arrivals_statistic_category)[0]) << ::std::endl;
	std::cout << "      Num Departures: " << *(qn.get_node(APP_SERVER_NODE).statistic(dcs::des::model::qn::num_departures_statistic_category)[0]) << ::std::endl;
	std::cout << "      Busy Time: " << *(qn.get_node(APP_SERVER_NODE).statistic(dcs::des::model::qn::busy_time_statistic_category)[0]) << ::std::endl;
	std::cout << "      Utilization: " << *(qn.get_node(APP_SERVER_NODE).statistic(dcs::des::model::qn::utilization_statistic_category)[0]) << ::std::endl;
	std::cout << "      Response Time: " << *(qn.get_node(APP_SERVER_NODE).statistic(dcs::des::model::qn::response_time_statistic_category)[0]) << ::std::endl;
	std::cout << "      Throughput: " << *(qn.get_node(APP_SERVER_NODE).statistic(dcs::des::model::qn::throughput_statistic_category)[0]) << ::std::endl;
	std::cout << "      Queue Length: " << *(qn.get_node(APP_SERVER_NODE).statistic(dcs::des::model::qn::num_waiting_statistic_category)[0]) << ::std::endl;
	std::cout << "    DB Server: " << ::std::endl;
	std::cout << "      Num Arrivals: " << *(qn.get_node(DB_SERVER_NODE).statistic(dcs::des::model::qn::num_arrivals_statistic_category)[0]) << ::std::endl;
	std::cout << "      Num Departures: " << *(qn.get_node(DB_SERVER_NODE).statistic(dcs::des::model::qn::num_departures_statistic_category)[0]) << ::std::endl;
	std::cout << "      Busy Time: " << *(qn.get_node(DB_SERVER_NODE).statistic(dcs::des::model::qn::busy_time_statistic_category)[0]) << ::std::endl;
	std::cout << "      Utilization: " << *(qn.get_node(DB_SERVER_NODE).statistic(dcs::des::model::qn::utilization_statistic_category)[0]) << ::std::endl;
	std::cout << "      Response Time: " << *(qn.get_node(DB_SERVER_NODE).statistic(dcs::des::model::qn::response_time_statistic_category)[0]) << ::std::endl;
	std::cout << "      Throughput: " << *(qn.get_node(DB_SERVER_NODE).statistic(dcs::des::model::qn::throughput_statistic_category)[0]) << ::std::endl;
	std::cout << "      Queue Length: " << *(qn.get_node(DB_SERVER_NODE).statistic(dcs::des::model::qn::num_waiting_statistic_category)[0]) << ::std::endl;
}
