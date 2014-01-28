/* ***** BEGIN LICENSE BLOCK *****
 *
 *   Copyright (C) 2014, Peter Hatina <phatina@redhat.com>
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation, either version 2.1 of the
 *   License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *   MA 02110-1301 USA
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef LMIWBEM_CLASS_NAME_H
#define LMIWBEM_CLASS_NAME_H

#include <string>
#include <boost/python/object.hpp>

namespace bp = boost::python;

class CIMClassName
{
public:
    CIMClassName();
    CIMClassName(
        const bp::object &classname,
        const bp::object &namespace_,
        const bp::object &hostname);

    static void init_type();
    static bp::object create(
        const bp::object &classname,
        const bp::object &namespace_,
        const bp::object &hostname);

    std::string repr();

    void setClassname(const bp::object &classname);
    void setNamespace(const bp::object &namespace_);
    void setHostname(const bp::object &hostname);

private:
    static bp::object s_class;

    std::string m_classname;
    std::string m_namespace;
    std::string m_hostname;
};

#endif // LMIWBEM_CLASS_NAME_H
