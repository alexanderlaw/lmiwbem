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

#include <config.h>
#include <algorithm>
#include <cstring>
#include <boost/python/borrowed.hpp>
#include <boost/python/handle.hpp>
#include <boost/python/list.hpp>
#include <boost/python/object.hpp>
#include <Pegasus/Common/CIMType.h>
#include "lmiwbem.h"
#include "lmiwbem_class.h"
#include "lmiwbem_connection.h"
#include "lmiwbem_constants.h"
#include "lmiwbem_extract.h"
#include "lmiwbem_class.h"
#include "lmiwbem_instance.h"
#include "lmiwbem_instance_name.h"
#include "lmiwbem_types.h"
#include "lmiwbem_util.h"

boost::shared_ptr<CIMTypeConv::CIMTypeHolder> CIMTypeConv::CIMTypeHolder::s_instance;

CIMTypeConv::CIMTypeHolder *CIMTypeConv::CIMTypeHolder::instance()
{
    if (!s_instance)
        s_instance.reset(new CIMTypeHolder());
    return s_instance.get();
}

CIMTypeConv::CIMTypeHolder::CIMTypeHolder()
{
    m_type_string[Pegasus::CIMTYPE_BOOLEAN] = "boolean";
    m_type_string[Pegasus::CIMTYPE_UINT8] = "uint8";
    m_type_string[Pegasus::CIMTYPE_SINT8] = "sint8";
    m_type_string[Pegasus::CIMTYPE_UINT16] = "uint16";
    m_type_string[Pegasus::CIMTYPE_SINT16] = "sint16";
    m_type_string[Pegasus::CIMTYPE_UINT32] = "uint32";
    m_type_string[Pegasus::CIMTYPE_SINT32] = "sint32";
    m_type_string[Pegasus::CIMTYPE_UINT64] = "uint64";
    m_type_string[Pegasus::CIMTYPE_SINT64] = "sint64";
    m_type_string[Pegasus::CIMTYPE_REAL32] = "real32";
    m_type_string[Pegasus::CIMTYPE_REAL64] = "real64";
    m_type_string[Pegasus::CIMTYPE_CHAR16] = "char16";
    m_type_string[Pegasus::CIMTYPE_STRING] = "string";
    m_type_string[Pegasus::CIMTYPE_DATETIME] = "CIMDateTime";
    m_type_string[Pegasus::CIMTYPE_REFERENCE] = "reference";
    m_type_string[Pegasus::CIMTYPE_OBJECT] = "object";
    m_type_string[Pegasus::CIMTYPE_INSTANCE] = "instance";

    m_string_type["bool"] = Pegasus::CIMTYPE_BOOLEAN;
    m_string_type["uint8"] = Pegasus::CIMTYPE_UINT8;
    m_string_type["sint8"] = Pegasus::CIMTYPE_SINT8;
    m_string_type["uint16"] = Pegasus::CIMTYPE_UINT16;
    m_string_type["sint16"] = Pegasus::CIMTYPE_SINT16;
    m_string_type["uint32"] = Pegasus::CIMTYPE_UINT32;
    m_string_type["sint32"] = Pegasus::CIMTYPE_SINT32;
    m_string_type["uint64"] = Pegasus::CIMTYPE_UINT64;
    m_string_type["sint64"] = Pegasus::CIMTYPE_SINT64;
    m_string_type["real32"] = Pegasus::CIMTYPE_REAL32;
    m_string_type["real64"] = Pegasus::CIMTYPE_REAL64;
    m_string_type["char16"] = Pegasus::CIMTYPE_CHAR16;
    m_string_type["string"] = Pegasus::CIMTYPE_STRING;
    m_string_type["CIMDateTime"] = Pegasus::CIMTYPE_DATETIME;
    m_string_type["reference"] = Pegasus::CIMTYPE_REFERENCE;
    m_string_type["object"] = Pegasus::CIMTYPE_OBJECT;
    m_string_type["instance"] = Pegasus::CIMTYPE_INSTANCE;
}

std::string CIMTypeConv::CIMTypeHolder::get(Pegasus::CIMType type)
{
    if (m_type_string.find(type) == m_type_string.end())
        return std::string();
    return m_type_string[type];
}

Pegasus::CIMType CIMTypeConv::CIMTypeHolder::get(const std::string &type)
{
    return m_string_type[type];
}

Pegasus::CIMPropertyList ListConv::asPegasusPropertyList(
    const bp::object &property_list,
    const std::string &message)
{
    Pegasus::CIMPropertyList cim_property_list;

    if (!isnone(property_list)) {
        bp::list py_property_list(lmi::get_or_throw<bp::list>(property_list, message));
        const int cnt = bp::len(py_property_list);
        Pegasus::Array<Pegasus::CIMName> property_arr(cnt);
        for (int i = 0; i < cnt; ++i) {
            std::string property = lmi::extract<std::string>(py_property_list[i]);
            property_arr[i] = Pegasus::CIMName(property.c_str());
        }
        cim_property_list.set(property_arr);
    }

    return cim_property_list;
}

std::string object_as_std_string(const bp::object &obj)
{
    PyObject *str = PyObject_Str(obj.ptr());
    if (!str)
        return std::string();
#  if PY_MAJOR_VERSION < 3
    return std::string(PyString_AsString(str));
#  else
    return std::string(PyUnicode_AsUTF8(str));
#  endif // PY_MAJOR_VERSION
}

std::string pystring_as_std_string(const bp::object &obj, bool &good)
{
    good = true;
    if (isunicode(obj)) {
#  if PY_MAJOR_VERSION < 3
        return std::string(
            PyString_AsString(
                PyUnicode_EncodeUTF8(
                    PyUnicode_AsUnicode(obj.ptr()),
                    PyUnicode_GetSize(obj.ptr()),
                    NULL)));
    } else if (isstring(obj)) {
        return std::string(PyString_AsString(obj.ptr()));
#  else
        return std::string(PyUnicode_AsUTF8(obj.ptr()));
#  endif // PY_MAJOR_VERSION
    }

    good = false;
    return std::string();
}

// Compatibility function
std::string pystring_as_std_string(const bp::object &obj)
{
    bool good;
    return pystring_as_std_string(obj, good);
}

bp::object std_string_as_pyunicode(const std::string &str)
{
    return bp::object(bp::handle<>(PyUnicode_FromString(str.c_str())));
}

bp::object std_string_as_pybool(const std::string &str)
{
    long int b = strtol(str.c_str(), NULL, 10);
    return bp::object(bp::handle<>(PyBool_FromLong(b)));
}

bp::object std_string_as_pyint(const std::string &str)
{
    char *s = strdup(str.c_str());
    bp::object pyint(bp::handle<>(PyInt_FromString(s, NULL, 10)));
    free(static_cast<void*>(s));
    return pyint;
}

bp::object std_string_as_pyfloat(const std::string &str)
{
    double d = strtod(str.c_str(), NULL);
    return bp::object(bp::handle<>(PyFloat_FromDouble(d)));
}

#  if PY_MAJOR_VERSION < 3
bp::object std_string_as_pylong(const std::string &str)
{
    char *s = strdup(str.c_str());
    bp::object pylong(bp::handle<>(PyLong_FromString(s, NULL, 10)));
    free(static_cast<void*>(s));
    return pylong;
}
#  endif // PY_MAJOR_VERSION

DEFINE_TO_CONVERTER(PegasusStringToPythonString, Pegasus::String)
{
    return bp::incref(bp::object(static_cast<const char*>(value.getCString())).ptr());
}

DEFINE_TO_CONVERTER(PegasusCIMNameToPythonString, Pegasus::CIMName)
{
    return PegasusStringToPythonString::convert(value.getString());
}

DEFINE_TO_CONVERTER(PegasusCIMDateteTimeToPythonDateTime, Pegasus::CIMDateTime)
{
    return bp::incref(CIMDateTime::create(bp::object(value.toString())).ptr());
}

DEFINE_TO_CONVERTER(PegasusChar16ToPythonUint16, Pegasus::Char16)
{
    // Pegasus::Char16 stores its data as Uint16
    return bp::incref(Uint16::create(value).ptr());
}

DEFINE_TO_CONVERTER(
    PegasusCIMObjectPathToPythonCIMInstanceName,
    Pegasus::CIMObjectPath)
{
    return bp::incref(CIMInstanceName::create(value).ptr());
}

DEFINE_TO_CONVERTER(PegasusCIMObjectToPythonCIMObject, Pegasus::CIMObject)
{
    if (value.isInstance())
        return bp::incref(CIMInstance::create(value).ptr());
    else
        return bp::incref(CIMClass::create(value).ptr());
}

bp::object incref(const bp::object &obj)
{
    bp::incref(obj.ptr());
    return obj;
}

bp::object this_module()
{
    return bp::object(bp::handle<>(bp::borrowed(PyImport_AddModule(PACKAGE_NAME))));
}

bool isnone(const bp::object &obj)
{
#ifdef BOOST_PYTHON_OBJECT_HAS_IS_NONE
    return obj.is_none();
#else
    // Older boost implementations lack api::object_base::is_none(). We need
    // to check for Py_None by empty boost::python::object().
    return obj == bp::object();
#endif // BOOST_PYTHON_OBJECT_HAS_IS_NONE
}

bool isinstance(const bp::object &inst, const bp::object &cls)
{
    return static_cast<bool>(PyObject_IsInstance(inst.ptr(), cls.ptr()));
}

#  if PY_MAJOR_VERSION < 3
bool isstring(const bp::object &obj)
{
    return static_cast<bool>(PyString_Check(obj.ptr()));
}
#  endif // PY_MAJOR_VERSION

bool isunicode(const bp::object &obj)
{
    return static_cast<bool>(PyUnicode_Check(obj.ptr()));
}

bool isbasestring(const bp::object &obj)
{
#  if PY_MAJOR_VERSION < 3
    return isstring(obj) || isunicode(obj);
#  else
    return isunicode(obj);
#  endif // PY_MAJOR_VERSION
}

bool isbool(const bp::object &obj)
{
    return static_cast<bool>(PyBool_Check(obj.ptr()));
}

#  if PY_MAJOR_VERSION < 3
bool isint(const bp::object &obj)
{
    return static_cast<bool>(PyInt_Check(obj.ptr()));
}
#  endif // PY_MAJOR_VERSION

bool islong(const bp::object &obj)
{
    return static_cast<bool>(PyLong_Check(obj.ptr()));
}

bool isfloat(const bp::object &obj)
{
    return static_cast<bool>(PyFloat_Check(obj.ptr()));
}

bool isdict(const bp::object &obj)
{
    return PyDict_Check(obj.ptr());
}

bool iscallable(const bp::object &obj)
{
    return static_cast<bool>(PyCallable_Check(obj.ptr()));
}

bool cim_issubclass(
    const bp::object &ch,
    const bp::object &ns,
    const bp::object &superclass,
    const bp::object &subclass)
{
    WBEMConnection &conn = lmi::extract_or_throw<WBEMConnection&>(ch, "ch");
    std::string std_ns = lmi::extract_or_throw<std::string>(ns, "ns");
    std::string std_superclass = lmi::extract_or_throw<std::string>(
        superclass, "superclass");

    std::string std_subclass;
    std::string std_subsuperclass;
    std::string std_lsubclass;
    std::string std_lsuperclass(std_superclass);
    std::transform(std_lsuperclass.begin(), std_lsuperclass.end(),
        std_lsuperclass.begin(), ::tolower);

    if (isinstance(subclass, CIMClass::type())) {
        const CIMClass &cim_subclass = lmi::extract<CIMClass&>(subclass);
        std_subclass = cim_subclass.getClassname();
        std_subsuperclass = cim_subclass.getSuperClassname();
    } else {
        std_subclass = lmi::extract_or_throw<std::string>(subclass, "subclass");
    }

    while (1) {
        // Matching is case insensitive.
        std_lsubclass = std_subclass;
        std::transform(std_lsubclass.begin(), std_lsubclass.end(),
            std_lsubclass.begin(), ::tolower);

        if (std_lsubclass == std_lsuperclass) {
            // Do subclass and superclass match?
            return true;
        } else if (std_subsuperclass.empty()) {
            // Get minimal subclass.
            bp::object cls = conn.getClass(bp::str(std_subclass.c_str()),
                bp::str(std_ns.c_str()), true, false, false, bp::list());

            const CIMClass &cim_subclass = lmi::extract<CIMClass&>(cls);
            std_subsuperclass = cim_subclass.getSuperClassname();
        }

        if (std_subsuperclass.empty()) {
            // We got a CIMClass without super class.
            return false;
        }

        std_subclass = std_subsuperclass;
        std_subsuperclass.clear();
    }

    return false;
}

bool is_error(const bp::object &value)
{
    int ivalue = lmi::extract_or_throw<int>(value, "value");

    switch (ivalue) {
    case CIMConstants::CIM_ERR_FAILED:
    case CIMConstants::CIM_ERR_ACCESS_DENIED:
    case CIMConstants::CIM_ERR_INVALID_NAMESPACE:
    case CIMConstants::CIM_ERR_INVALID_PARAMETER:
    case CIMConstants::CIM_ERR_INVALID_CLASS:
    case CIMConstants::CIM_ERR_NOT_FOUND:
    case CIMConstants::CIM_ERR_NOT_SUPPORTED:
    case CIMConstants::CIM_ERR_CLASS_HAS_CHILDREN:
    case CIMConstants::CIM_ERR_CLASS_HAS_INSTANCES:
    case CIMConstants::CIM_ERR_INVALID_SUPERCLASS:
    case CIMConstants::CIM_ERR_ALREADY_EXISTS:
    case CIMConstants::CIM_ERR_NO_SUCH_PROPERTY:
    case CIMConstants::CIM_ERR_TYPE_MISMATCH:
    case CIMConstants::CIM_ERR_QUERY_LANGUAGE_NOT_SUPPORTED:
    case CIMConstants::CIM_ERR_INVALID_QUERY:
    case CIMConstants::CIM_ERR_METHOD_NOT_AVAILABLE:
    case CIMConstants::CIM_ERR_METHOD_NOT_FOUND:
    case CIMConstants::CON_ERR_OTHER:
    case CIMConstants::CON_ERR_ALREADY_CONNECTED:
    case CIMConstants::CON_ERR_NOT_CONNECTED:
    case CIMConstants::CON_ERR_INVALID_LOCATOR:
    case CIMConstants::CON_ERR_CANNOT_CREATE_SOCKET:
    case CIMConstants::CON_ERR_CANNOT_CONNECT:
    case CIMConstants::CON_ERR_CONNECTION_TIMEOUT:
    case CIMConstants::CON_ERR_SSL_EXCEPTION:
    case CIMConstants::CON_ERR_BIND:
#ifdef HAVE_SLP
    case CIMConstants::SLP_ERR_LANGUAGE_NOT_SUPPORTED:
    case CIMConstants::SLP_ERR_PARSE_ERROR:
    case CIMConstants::SLP_ERR_INVALID_REGISTRATION:
    case CIMConstants::SLP_ERR_SCOPE_NOT_SUPPORTED:
    case CIMConstants::SLP_ERR_AUTHENTICATION_ABSENT:
    case CIMConstants::SLP_ERR_AUTHENTICATION_FAILED:
    case CIMConstants::SLP_ERR_INVALID_UPDATE:
    case CIMConstants::SLP_ERR_REFRESH_REJECTED:
    case CIMConstants::SLP_ERR_NOT_IMPLEMENTED:
    case CIMConstants::SLP_ERR_BUFFER_OVERFLOW:
    case CIMConstants::SLP_ERR_NETWORK_TIMED_OUT:
    case CIMConstants::SLP_ERR_NETWORK_INIT_FAILED:
    case CIMConstants::SLP_ERR_MEMORY_ALLOC_FAILED:
    case CIMConstants::SLP_ERR_PARAMETER_BAD:
    case CIMConstants::SLP_ERR_NETWORK_ERROR:
    case CIMConstants::SLP_ERR_INTERNAL_SYSTEM_ERROR:
    case CIMConstants::SLP_ERR_HANDLE_IN_USE:
    case CIMConstants::SLP_ERR_TYPE_ERROR:
#  ifndef UNICAST_NOT_SUPPORTED
    case CIMConstants::SLP_ERR_RETRY_UNICAST:
#  endif // UNICAST_NOT_SUPPORTED
#endif // HAVE_SLP
        return true;
    default:
        return false;
    }
}

#  if PY_MAJOR_VERSION < 3
int compare(const bp::object &o1, const bp::object &o2)
{
    int result = 1;
    PyObject_Cmp(o1.ptr(), o2.ptr(), &result);
    return result;
}
#  else
bool compare(const bp::object &o1, const bp::object &o2, int cmp_type)
{
    int rval = PyObject_RichCompareBool(o1.ptr(), o2.ptr(), cmp_type);
    if (rval == -1)
        throw bp::error_already_set();
    return static_cast<bool>(rval);
}
#  endif // PY_MAJOR_VERSION
