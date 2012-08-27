#!/bin/bash

##
## Shell utility functions.
##
## Authors:
##  Marco Guazzone (marco.guazzone@gmail.com)
##


## Get the major version number of bash.
function dcs_bash_major_version()
{
	n=0
	found=0
	for ((i=0;i<${#BASH_VERSION} && !$found;i++))
	do
		ch=${BASH_VERSION:$i:1}
		case "$ch" in
			[0-9])
				n=$((n+$ch))
				;;
			'.')
				found=1
				;;
		esac
	done

	echo -n $n
}


## Convert to lower case
function dcs_tolower()
{
	ver=$(dcs_bash_major_version)

	s=
	if [ "$ver" -ge "4" ]; then
		declare -l s=$1
	else
		s=$(echo $1 | tr '[A-Z]' '[a-z]')
	fi
	echo -n $s
}


## Format a date
function dcs_format_date()
{
	echo $(date +'%F %T (%Z)')
}


## Initialize log file
function dcs_log_init()
{
	local logfile=$1

	> $logfile
}


## Log an error to the log-file.
function dcs_log_error()
{
	local logfile=$1

	date=$(dcs_format_date)
	echo "ERROR ($date)>> $2" >> $logfile
}


## Log an error to the log-file.
function dcs_log_info()
{
	local logfile=$1

	date=$(dcs_format_date)
	echo "INFO ($date)>> $2" >> $logfile
}


## Log an error to the log-file.
function dcs_log_warn()
{
	local logfile=$1

	date=$(dcs_format_date)
	echo "WARNING ($date)>> $2" >> $logfile
}


## Return yes or no
function dcs_yes_no()
{
	#declare -l v=$1
	v=$(dcs_tolower $1)
	if [ "$v" = "yes" ] || [ "$v" = "true" ] || [ "$v" = "1" ]; then
		echo -n "yes"
	else
		echo -n "no"
	fi
}


## Check if your system has the Boost libraries installed
function dcs_check_boost()
{
	local inc_path=$1
	local lib_path=$2
	local compile_test=''
	local ret=0
	local src_temp_file=`mktemp tmp.XXXXXXXXXX`
	local out_temp_file=`mktemp tmp.XXXXXXXXXX`

	cat > $src_temp_file <<EOT
#include <boost/version.hpp>

int main(int argc, char* argv[])
{
	return 0;
}
EOT

	if [ ! -z "$inc_path" ]; then
		inc_path="-I$inc_path"
	fi
	if [ ! -z "$lib_path" ]; then
		lib_path="-L$lib_path"
	fi
	compile_test=$($CXX $CXXFLAGS $inc_path -x 'c++' -o $out_temp_file $src_temp_file $LDFLAGS $lib_path 2>&1 >/dev/null)
	ret=$?

	rm -f $out_temp_file $src_temp_file

	if [ $ret == 0 ]; then
		echo -n "yes"
	else
		echo -n "no"
	fi
}


## Check if your system has a specific version of the Boost libraries installed
function dcs_check_boost_ver()
{
	local ver=$1
	local inc_path=$2
	local lib_path=$3
	local compile_test=''
	local ret=0
	local src_temp_file=`mktemp tmp.XXXXXXXXXX`
	local out_temp_file=`mktemp tmp.XXXXXXXXXX`

	cat > $src_temp_file <<EOT
#include <boost/version.hpp>

#if BOOST_VERSION < $ver
# error Boost version too old.
#endif // BOOST_VERSION

int main(int argc, char* argv[])
{
	return 0;
}
EOT

	if [ ! -z "$inc_path" ]; then
		inc_path="-I$inc_path"
	fi
	if [ ! -z "$lib_path" ]; then
		lib_path="-L$lib_path"
	fi
	compile_test=$($CXX $CXXFLAGS $inc_path -x 'c++' -o $out_temp_file $src_temp_file $LDFLAGS $lib_path 2>&1 >/dev/null)
	ret=$?

	rm -f $out_temp_file $src_temp_file

	if [ $ret == 0 ]; then
		echo -n "yes"
	else
		echo -n "no"
	fi
}


## Check if your system has the lp_solve libraries installed
function dcs_check_lpsolve()
{
	local compile_test=''
	local ret=0
	local src_temp_file=`mktemp tmp.XXXXXXXXXX`
	local out_temp_file=`mktemp tmp.XXXXXXXXXX`

	cat > $src_temp_file <<EOT
#include <lpsolve/lp_lib.h>

int main(int argc, char* argv[])
{
	return 0;
}
EOT

	compile_test=$($CXX $CXXFLAGS -x 'c++' -o $out_temp_file $src_temp_file $LDFLAGS -llpsolve55 -lm 2>&1 >/dev/null)
	ret=$?

	rm -f $out_temp_file $src_temp_file

	if [ $ret == 0 ]; then
		echo -n "yes"
	else
		echo -n "no"
	fi
}


## Check if your system has the CBC libraries installed
function dcs_check_cbc()
{
	local inc_path=$1
	local lib_path=$2
	local compile_test=''
	local ret=0
	local src_temp_file=`mktemp tmp.XXXXXXXXXX`
	local out_temp_file=`mktemp tmp.XXXXXXXXXX`

	cat > $src_temp_file <<EOT
#include <CbcModel.hpp>

int main(int argc, char* argv[])
{
	return 0;
}
EOT

	if [ ! -z "$inc_path" ]; then
		inc_path="-I$inc_path"
	fi
	if [ ! -z "$lib_path" ]; then
		lib_path="-L$lib_path"
	fi
	compile_test=$($CXX $CXXFLAGS $inc_path -x 'c++' -o $out_temp_file $src_temp_file $LDFLAGS $lib_path -lCbcSolver -lCbc -lCgl -lOsiClp -lClp -lOsi -lCoinUtils -lbz2 -lz -llapack -lblas -lm 2>&1 >/dev/null)
	ret=$?

	rm -f $out_temp_file $src_temp_file

	if [ $ret == 0 ]; then
		echo -n "yes"
	else
		echo -n "no"
	fi
}


## Check if your system has the CPLEX libraries installed
function dcs_check_cplex()
{
	local inc_path=$1
	local lib_path=$2
	local compile_test=''
	local ret=0
	local src_temp_file=`mktemp tmp.XXXXXXXXXX`
	local out_temp_file=`mktemp tmp.XXXXXXXXXX`

	cat > $src_temp_file <<EOT
#include <cplex.h>

int main(int argc, char* argv[])
{
	return 0;
}
EOT

	if [ ! -z "$inc_path" ]; then
		inc_path="-I$inc_path"
	fi
	if [ ! -z "$lib_path" ]; then
		lib_path="-L$lib_path"
	fi
	compile_test=$($CXX $CXXFLAGS $inc_path -x 'c++' -o $out_temp_file $src_temp_file $LDFLAGS $lib_path -lcplex124 -lm 2>&1 >/dev/null)
	ret=$?

	rm -f $out_temp_file $src_temp_file

	if [ $ret == 0 ]; then
		echo -n "yes"
	else
		echo -n "no"
	fi
}


## Check if your system has the FLOPC++ libraries installed
function dcs_check_flopcpp()
{
	local inc_path=$1
	local lib_path=$2
	local compile_test=''
	local ret=0
	local src_temp_file=`mktemp tmp.XXXXXXXXXX`
	local out_temp_file=`mktemp tmp.XXXXXXXXXX`

	cat > $src_temp_file <<EOT
#include <flopc.hpp>

int main(int argc, char* argv[])
{
	return 0;
}
EOT

	if [ ! -z "$inc_path" ]; then
		inc_path="-I$inc_path"
	fi
	if [ ! -z "$lib_path" ]; then
		lib_path="-L$lib_path"
	fi
	compile_test=$($CXX $CXXFLAGS $inc_path -x 'c++' -o $out_temp_file $src_temp_file $LDFLAGS $lib_path -lFlopCpp -lOsi -lCoinUtils -lbz2 -lz -llapack -lblas -lm 2>&1 >/dev/null)
	ret=$?

	rm -f $out_temp_file $src_temp_file

	if [ $ret == 0 ]; then
		echo -n "yes"
	else
		echo -n "no"
	fi
}


## Check if your system has the GLPK libraries installed
function dcs_check_glpk()
{
	local inc_path=$1
	local lib_path=$2
	local compile_test=''
	local ret=0
	local src_temp_file=`mktemp tmp.XXXXXXXXXX`
	local out_temp_file=`mktemp tmp.XXXXXXXXXX`

	cat > $src_temp_file <<EOT
#include <glpk/glpk.h>

int main(int argc, char* argv[])
{
	return 0;
}
EOT

	if [ ! -z "$inc_path" ]; then
		inc_path="-I$inc_path"
	fi
	if [ ! -z "$lib_path" ]; then
		lib_path="-L$lib_path"
	fi
	compile_test=$($CXX $CXXFLAGS $inc_path -x 'c++' -o $out_temp_file $src_temp_file $LDFLAGS $lib_path -lglpk -lm 2>&1 >/dev/null)
	ret=$?

	rm -f $out_temp_file $src_temp_file

	if [ $ret == 0 ]; then
		echo -n "yes"
	else
		echo -n "no"
	fi
}


## Check if your system has the Gnu Scientific Library (GSL) libraries installed
function dcs_check_gsl()
{
	local inc_path=$1
	local lib_path=$2
	local compile_test=''
	local ret=0
	local src_temp_file=`mktemp tmp.XXXXXXXXXX`
	local out_temp_file=`mktemp tmp.XXXXXXXXXX`

	cat > $src_temp_file <<EOT
#include <gsl/gsl_types.h>

int main(int argc, char* argv[])
{
	return 0;
}
EOT

	if [ ! -z "$inc_path" ]; then
		inc_path="-I$inc_path"
	fi
	if [ ! -z "$lib_path" ]; then
		lib_path="-L$lib_path"
	fi
	compile_test=$($CXX $CXXFLAGS $inc_path -x 'c++' -o $out_temp_file $src_temp_file $LDFLAGS $lib_path -lgsl -lgslcblas -lm 2>&1 >/dev/null)
	ret=$?

	rm -f $out_temp_file $src_temp_file

	if [ $ret == 0 ]; then
		echo -n "yes"
	else
		echo -n "no"
	fi
}


## Check if your system has the GUROBI libraries installed
function dcs_check_gurobi()
{
	local inc_path=$1
	local lib_path=$2
	local compile_test=''
	local ret=0
	local src_temp_file=`mktemp tmp.XXXXXXXXXX`
	local out_temp_file=`mktemp tmp.XXXXXXXXXX`

	cat > $src_temp_file <<EOT
#include <gurobi_c++.h>

int main(int argc, char* argv[])
{
	return 0;
}
EOT

	if [ ! -z "$inc_path" ]; then
		inc_path="-I$inc_path"
	fi
	if [ ! -z "$lib_path" ]; then
		lib_path="-L$lib_path"
	fi
	compile_test=$($CXX $CXXFLAGS $inc_path -x 'c++' -o $out_temp_file $src_temp_file $LDFLAGS $lib_path -lgurobi50 -lgurobi_c++ -lm 2>&1 >/dev/null)
	ret=$?

	rm -f $out_temp_file $src_temp_file

	if [ $ret == 0 ]; then
		echo -n "yes"
	else
		echo -n "no"
	fi
}


## Check if your system has the MOSEK libraries installed
function dcs_check_mosek()
{
	local inc_path=$1
	local lib_path=$2
	local compile_test=''
	local ret=0
	local src_temp_file=`mktemp tmp.XXXXXXXXXX`
	local out_temp_file=`mktemp tmp.XXXXXXXXXX`

	cat > $src_temp_file <<EOT
#include <mosek.h>

int main(int argc, char* argv[])
{
	return 0;
}
EOT

	if [ ! -z "$inc_path" ]; then
		inc_path="-I$inc_path"
	fi
	if [ ! -z "$lib_path" ]; then
		lib_path="-L$lib_path"
	fi
	#FIXME: Currently we only handle 64bit version of MOSEK. How to handle 32/64bit difference in the library name?
	compile_test=$($CXX $CXXFLAGS $inc_path -x 'c++' -o $out_temp_file $src_temp_file $LDFLAGS $lib_path -lmosek64 -pthread -lm 2>&1 >/dev/null)
	ret=$?

	rm -f $out_temp_file $src_temp_file

	if [ $ret == 0 ]; then
		echo -n "yes"
	else
		echo -n "no"
	fi
}


## Check if your system has the OSI libraries installed
function dcs_check_osi()
{
	local inc_path=$1
	local lib_path=$2
	local compile_test=''
	local ret=0
	local src_temp_file=`mktemp tmp.XXXXXXXXXX`
	local out_temp_file=`mktemp tmp.XXXXXXXXXX`

	cat > $src_temp_file <<EOT
#include <OsiSolverInterface.hpp>

int main(int argc, char* argv[])
{
	return 0;
}
EOT

	if [ ! -z "$inc_path" ]; then
		inc_path="-I$inc_path"
	fi
	if [ ! -z "$lib_path" ]; then
		lib_path="-L$lib_path"
	fi
	compile_test=$($CXX $CXXFLAGS $inc_path -x 'c++' -o $out_temp_file $src_temp_file $LDFLAGS $lib_path -lOsi -lCoinUtils -lbz2 -lz -llapack -lblas -lm 2>&1 >/dev/null)
	ret=$?

	rm -f $out_temp_file $src_temp_file

	if [ $ret == 0 ]; then
		echo -n "yes"
	else
		echo -n "no"
	fi
}


## Check if your system has the OSI-CBC libraries installed
function dcs_check_osi_cbc()
{
	local inc_path=$1
	local lib_path=$2
	local compile_test=''
	local ret=0
	local src_temp_file=`mktemp tmp.XXXXXXXXXX`
	local out_temp_file=`mktemp tmp.XXXXXXXXXX`

	cat > $src_temp_file <<EOT
#include <CbcModel.hpp>
#include <OsiCbcSolverInterface.hpp>
#include <OsiSolverInterface.hpp>

int main(int argc, char* argv[])
{
	return 0;
}
EOT

	if [ ! -z "$inc_path" ]; then
		inc_path="-I$inc_path"
	fi
	if [ ! -z "$lib_path" ]; then
		lib_path="-L$lib_path"
	fi
	compile_test=$($CXX $CXXFLAGS $inc_path -x 'c++' -o $out_temp_file $src_temp_file $LDFLAGS $lib_path -lOsiCbc -lCbcSolver -lCbc -lCgl -lOsiClp -lClp -lOsi -lCoinUtils -lbz2 -lz -llapack -lblas -lm 2>&1 >/dev/null)
	ret=$?

	rm -f $out_temp_file $src_temp_file

	if [ $ret == 0 ]; then
		echo -n "yes"
	else
		echo -n "no"
	fi
}


## Check if your system has the CPLEX libraries installed
function dcs_check_osi_cplex()
{
	local inc_path=$1
	local lib_path=$2
	local compile_test=''
	local ret=0
	local src_temp_file=`mktemp tmp.XXXXXXXXXX`
	local out_temp_file=`mktemp tmp.XXXXXXXXXX`

	cat > $src_temp_file <<EOT
#include <cplex.h>
#include <OsiCpxSolverInterface.hpp>
#include <OsiSolverInterface.hpp>

int main(int argc, char* argv[])
{
	return 0;
}
EOT

	if [ ! -z "$inc_path" ]; then
		inc_path="-I$inc_path"
	fi
	if [ ! -z "$lib_path" ]; then
		lib_path="-L$lib_path"
	fi
	compile_test=$($CXX $CXXFLAGS $inc_path -x 'c++' -o $out_temp_file $src_temp_file $LDFLAGS $lib_path -lOsiCpx -lcplex124 -lOsi -lCoinUtils -lbz2 -lz -llapack -lblas -lm 2>&1 >/dev/null)
	ret=$?

	rm -f $out_temp_file $src_temp_file

	if [ $ret == 0 ]; then
		echo -n "yes"
	else
		echo -n "no"
	fi
}


## Check if your system has the GLPK libraries installed
function dcs_check_osi_glpk()
{
	local inc_path=$1
	local lib_path=$2
	local compile_test=''
	local ret=0
	local src_temp_file=`mktemp tmp.XXXXXXXXXX`
	local out_temp_file=`mktemp tmp.XXXXXXXXXX`

	cat > $src_temp_file <<EOT
#include <glpk/glpk.h>
#include <OsiGlpkSolverInterface.hpp>
#include <OsiSolverInterface.hpp>

int main(int argc, char* argv[])
{
	return 0;
}
EOT

	if [ ! -z "$inc_path" ]; then
		inc_path="-I$inc_path"
	fi
	if [ ! -z "$lib_path" ]; then
		lib_path="-L$lib_path"
	fi
	compile_test=$($CXX $CXXFLAGS $inc_path -x 'c++' -o $out_temp_file $src_temp_file $LDFLAGS $lib_path -lOsiGlpk -lOsi -lCoinUtils -lbz2 -lz -lglpk -llapack -lblas -lm 2>&1 >/dev/null)
	ret=$?

	rm -f $out_temp_file $src_temp_file

	if [ $ret == 0 ]; then
		echo -n "yes"
	else
		echo -n "no"
	fi
}


## Check if your system has the GUROBI libraries installed
function dcs_check_osi_gurobi()
{
	local inc_path=$1
	local lib_path=$2
	local compile_test=''
	local ret=0
	local src_temp_file=`mktemp tmp.XXXXXXXXXX`
	local out_temp_file=`mktemp tmp.XXXXXXXXXX`

	cat > $src_temp_file <<EOT
#include <gurobi_c++.h>
#include <OsiSolverInterface.hpp>
#include <OsiGrbSolverInterface.hpp>

int main(int argc, char* argv[])
{
	return 0;
}
EOT

	if [ ! -z "$inc_path" ]; then
		inc_path="-I$inc_path"
	fi
	if [ ! -z "$lib_path" ]; then
		lib_path="-L$lib_path"
	fi
	compile_test=$($CXX $CXXFLAGS $inc_path -x 'c++' -o $out_temp_file $src_temp_file $LDFLAGS $lib_path -lOsiSym -lgurobi -lgurobi_c++ -lCgl -lOsiClp -lClp -lOsi -lCoinUtils -lbz2 -lz -llapack -lblas -lm 2>&1 >/dev/null)
	ret=$?

	rm -f $out_temp_file $src_temp_file

	if [ $ret == 0 ]; then
		echo -n "yes"
	else
		echo -n "no"
	fi
}


## Check if your system has the MOSEK libraries installed
function dcs_check_osi_mosek()
{
	local inc_path=$1
	local lib_path=$2
	local compile_test=''
	local ret=0
	local src_temp_file=`mktemp tmp.XXXXXXXXXX`
	local out_temp_file=`mktemp tmp.XXXXXXXXXX`

	cat > $src_temp_file <<EOT
#include <mosek.h>
#include <OsiMskSolverInterface.hpp>
#include <OsiSolverInterface.hpp>

int main(int argc, char* argv[])
{
	return 0;
}
EOT

	if [ ! -z "$inc_path" ]; then
		inc_path="-I$inc_path"
	fi
	if [ ! -z "$lib_path" ]; then
		lib_path="-L$lib_path"
	fi
	#FIXME: Currently we only handle 64bit version of MOSEK. How to handle 32/64bit difference in the library name?
	compile_test=$($CXX $CXXFLAGS $inc_path -x 'c++' -o $out_temp_file $src_temp_file $LDFLAGS $lib_path -lOsiMsk -lmosek64 -lpthread -lOsi -lCoinUtils -lbz2 -lz -llapack -lblas -lm 2>&1 >/dev/null)
	ret=$?

	rm -f $out_temp_file $src_temp_file

	if [ $ret == 0 ]; then
		echo -n "yes"
	else
		echo -n "no"
	fi
}


## Check if your system has the SOPLEX libraries installed
function dcs_check_osi_soplex()
{
	local inc_path=$1
	local lib_path=$2
	local compile_test=''
	local ret=0
	local src_temp_file=`mktemp tmp.XXXXXXXXXX`
	local out_temp_file=`mktemp tmp.XXXXXXXXXX`

	cat > $src_temp_file <<EOT
#include <soplex.h>
#include <OsiSpxSolverInterface.hpp>
#include <OsiSolverInterface.hpp>

int main(int argc, char* argv[])
{
	return 0;
}
EOT

	if [ ! -z "$inc_path" ]; then
		inc_path="-I$inc_path"
	fi
	if [ ! -z "$lib_path" ]; then
		lib_path="-L$lib_path"
	fi
	compile_test=$($CXX $CXXFLAGS $inc_path -x 'c++' -o $out_temp_file $src_temp_file $LDFLAGS $lib_path -lOsiSpx -lsoplex -lOsi -lCoinUtils -lbz2 -lz -llapack -lblas -lm 2>&1 >/dev/null)
	ret=$?

	rm -f $out_temp_file $src_temp_file

	if [ $ret == 0 ]; then
		echo -n "yes"
	else
		echo -n "no"
	fi
}


## Check if your system has the SYMPHONY libraries installed
function dcs_check_osi_symphony()
{
	local inc_path=$1
	local lib_path=$2
	local compile_test=''
	local ret=0
	local src_temp_file=`mktemp tmp.XXXXXXXXXX`
	local out_temp_file=`mktemp tmp.XXXXXXXXXX`

	cat > $src_temp_file <<EOT
#include <OsiSolverInterface.hpp>
#include <OsiSymSolverInterface.hpp>
#include <symphony.h>

int main(int argc, char* argv[])
{
	return 0;
}
EOT

	if [ ! -z "$inc_path" ]; then
		inc_path="-I$inc_path"
	fi
	if [ ! -z "$lib_path" ]; then
		lib_path="-L$lib_path"
	fi
	compile_test=$($CXX $CXXFLAGS $inc_path -x 'c++' -o $out_temp_file $src_temp_file $LDFLAGS $lib_path -lOsiSym -lSym -lCgl -lOsiClp -lClp -lOsi -lCoinUtils -lbz2 -lz -llapack -lblas -lm 2>&1 >/dev/null)
	ret=$?

	rm -f $out_temp_file $src_temp_file

	if [ $ret == 0 ]; then
		echo -n "yes"
	else
		echo -n "no"
	fi
}


## Check if your system has the SOPLEX libraries installed
function dcs_check_soplex()
{
	local inc_path=$1
	local lib_path=$2
	local compile_test=''
	local ret=0
	local src_temp_file=`mktemp tmp.XXXXXXXXXX`
	local out_temp_file=`mktemp tmp.XXXXXXXXXX`

	cat > $src_temp_file <<EOT
#include <soplex.h>

int main(int argc, char* argv[])
{
	return 0;
}
EOT

	if [ ! -z "$inc_path" ]; then
		inc_path="-I$inc_path"
	fi
	if [ ! -z "$lib_path" ]; then
		lib_path="-L$lib_path"
	fi
	compile_test=$($CXX $CXXFLAGS $inc_path -x 'c++' -o $out_temp_file $src_temp_file $LDFLAGS $lib_path -lsoplex -lm 2>&1 >/dev/null)
	ret=$?

	rm -f $out_temp_file $src_temp_file

	if [ $ret == 0 ]; then
		echo -n "yes"
	else
		echo -n "no"
	fi
}


## Check if your system has the SYMPHONY libraries installed
function dcs_check_symphony()
{
	local inc_path=$1
	local lib_path=$2
	local compile_test=''
	local ret=0
	local src_temp_file=`mktemp tmp.XXXXXXXXXX`
	local out_temp_file=`mktemp tmp.XXXXXXXXXX`

	cat > $src_temp_file <<EOT
#include <symphony.h>

int main(int argc, char* argv[])
{
	return 0;
}
EOT

	if [ ! -z "$inc_path" ]; then
		inc_path="-I$inc_path"
	fi
	if [ ! -z "$lib_path" ]; then
		lib_path="-L$lib_path"
	fi
	compile_test=$($CXX $CXXFLAGS $inc_path -x 'c++' -o $out_temp_file $src_temp_file $LDFLAGS $lib_path -lSym -lCgl -lOsiClp -lClp -lOsi -lCoinUtils -lbz2 -lz -llapack -lblas -lm 2>&1 >/dev/null)
	ret=$?

	rm -f $out_temp_file $src_temp_file

	if [ $ret == 0 ]; then
		echo -n "yes"
	else
		echo -n "no"
	fi
}
