/**
 * \file dcs/des/entity.hpp
 *
 * \brief A generic simulation entity.
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

#ifndef DCS_DES_ENTITY_HPP
#define DCS_DES_ENTITY_HPP


namespace dcs { namespace des {

class entity
{
	protected: explicit entity(bool enabled = true)
	: enabled_(enabled)
	{
	}


	public: void enable(bool flag)
	{
		do_enable(flag);

		enabled_ = flag;
	}


	public: bool enabled() const
	{
		return enabled_;
	}


	private: virtual void do_enable(bool flag) = 0;


	private: bool enabled_;
};


}} // Namespac dcs::des


#endif // DCS_DES_ENTITY_HPP
