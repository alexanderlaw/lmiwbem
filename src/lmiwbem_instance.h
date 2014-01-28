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

#ifndef LMIWBEM_INSTANCE_H
#define LMIWBEM_INSTANCE_H

#include <list>
#include <string>
#include <boost/python/object.hpp>
#include <Pegasus/Common/CIMInstance.h>
#include "lmiwbem.h"
#include "lmiwbem_refcountedptr.h"

namespace bp = boost::python;

class CIMInstance
{
public:
    CIMInstance();
    CIMInstance(
        const bp::object &classname,
        const bp::object &properties,
        const bp::object &qualifiers,
        const bp::object &path,
        const bp::object &property_list);

    static void init_type();
    static bp::object create(const Pegasus::CIMInstance &instance);
    static bp::object create(const Pegasus::CIMObject &object);

    std::string repr();

    bp::object getClassname();
    bp::object getPath();
    bp::object getProperties();
    bp::object getQualifiers();
    bp::object getPropertyList();

    void setClassname(const bp::object &classname);
    void setPath(const bp::object &path);
    void setProperties(const bp::object &properties);
    void setQualifiers(const bp::object &qualifiers);
    void setPropertyList(const bp::object &property_list);

private:
    void evalProperties();

    static bp::object s_class;

    std::string m_classname;
    bp::object m_path;
    bp::object m_properties;
    bp::object m_qualifiers;
    bp::object m_property_list;

    RefCountedPtr<Pegasus::CIMObjectPath> m_rc_inst_path;
    RefCountedPtr<std::list<Pegasus::CIMConstProperty> > m_rc_inst_properties;
    RefCountedPtr<std::list<Pegasus::CIMConstQualifier> > m_rc_inst_qualifiers;
};

#endif // LMIWBEM_INSTANCE_H
