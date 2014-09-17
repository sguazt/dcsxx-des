/**
 * \file dcs/des/detail/config/boost.hpp
 *
 * \brief Configurations for Boost libraries.
 *
 * [2014-09-17 17:07:59 (CEST)]
 * This is an autogenerated file. Do not edit.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */

#ifndef DCS_DES_DETAIL_CONFIG_BOOST_HPP
#define DCS_DES_DETAIL_CONFIG_BOOST_HPP


//#include <dcs/des/macro.hpp>


/// Inform if we have Boost
#if true
# ifndef DCS_DES_DETAIL_CONFIG_HAVE_BOOST
#  define DCS_DES_DETAIL_CONFIG_HAVE_BOOST
# endif // DCS_DES_DETAIL_CONFIG_HAVE_BOOST
#else
# undef DCS_DES_DETAIL_CONFIG_HAVE_BOOST
#endif // true

/// Get the Boost version macros
#ifdef DCS_DES_DETAIL_CONFIG_HAVE_BOOST
# include <boost/version.hpp>
# define DCS_DES_DETAIL_CONFIG_BOOST_VERSION BOOST_VERSION
# define DCS_DES_DETAIL_CONFIG_BOOST_MAJOR_VERSION (BOOST_VERSION / 100000)
# define DCS_DES_DETAIL_CONFIG_BOOST_MINOR_VERSION (BOOST_VERSION / 100 % 1000)
# define DCS_DES_DETAIL_CONFIG_BOOST_PATCH_VERSION (BOOST_VERSION % 100)
#endif // DCS_DES_DETAIL_CONFIG_HAVE_BOOST

#define DCS_DES_DETAIL_CONFIG_BOOST_CHECK_VERSION(v) \
	(defined(DCS_DES_DETAIL_CONFIG_HAVE_BOOST) && (DCS_DES_DETAIL_CONFIG_BOOST_VERSION >= (v)))

// This is a macro for macro generation... actually it seem not possible :(
//#define DCS_DES_DETAIL_CONFIG_BOOST_NEED_VERSION(v)
//	DCS_MACRO_HASHIFY(if) DCS_DES_DETAIL_CONFIG_BOOST_CHECK_VERSION(v)
//		DCS_MACRO_HASHIFY(error) "Required Boost library version >= " #v
//	DCS_MACRO_HASHIFY(endif)

#endif // DCS_DES_DETAIL_CONFIG_BOOST_HPP
