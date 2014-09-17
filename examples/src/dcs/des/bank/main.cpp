#include "bank.hpp"
#include <cstddef>


int main()
{
	typedef double real_type;

	std::size_t num_tellers = 5;
	real_type arr_rate = 1.0;
	real_type svc_rate = 1.0/4.0;

	dcs::examples::des::bank::bank<real_type> the_bank(num_tellers, arr_rate, svc_rate, true);

	the_bank.simulate();
}
