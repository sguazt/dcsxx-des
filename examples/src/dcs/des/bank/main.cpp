#include "bank.hpp"


int main()
{
	typedef double real_type;
	typedef unsigned long uint_type;

	uint_type num_tellers = 5;
	real_type arr_rate = 1.0;
	real_type svc_rate = 1.0/4.0;

	dcs::examples::des::bank::bank<real_type,uint_type> the_bank(num_tellers, arr_rate, svc_rate, true);

	the_bank.simulate();
}
