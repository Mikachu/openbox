// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

%module openbox

%{
#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif

#include "openbox.hh"
#include "screen.hh"
#include "client.hh"
#include "bindings.hh"
#include "actions.hh"
#include "python.hh"
#include "otk/display.hh"
%}

%include "stl.i"
//%include std_list.i
//%template(ClientList) std::list<OBClient*>;

%ignore ob::Openbox::instance;
%inline %{
  ob::Openbox *Openbox_instance() { return ob::Openbox::instance; }
%};

%{
namespace ob {
void python_callback(PyObject *func, MouseData *data)
{
  PyObject *arglist;
  PyObject *result;

  arglist = Py_BuildValue("(O)", SWIG_NewPointerObj((void *) data,
                                                    SWIGTYPE_p_ob__MouseData,
                                                    0));
  
  // call the callback
  result = PyEval_CallObject(func, arglist);
  if (!result || PyErr_Occurred()) {
    // an exception occured in the script, display it
    PyErr_Print();
  }

  Py_XDECREF(result);
  Py_DECREF(arglist);
}

void python_callback(PyObject *func, EventData *data)
{
  PyObject *arglist;
  PyObject *result;

  arglist = Py_BuildValue("(O)", SWIG_NewPointerObj((void *) data,
                                                    SWIGTYPE_p_ob__EventData,
                                                    0));
  
  // call the callback
  result = PyEval_CallObject(func, arglist);
  if (!result || PyErr_Occurred()) {
    // an exception occured in the script, display it
    PyErr_Print();
  }

  Py_XDECREF(result);
  Py_DECREF(arglist);
}

void python_callback(PyObject *func, KeyData *data)
{
  PyObject *arglist;
  PyObject *result;

  arglist = Py_BuildValue("(O)", SWIG_NewPointerObj((void *) data,
                                                    SWIGTYPE_p_ob__KeyData,
                                                    0));
  
  // call the callback
  result = PyEval_CallObject(func, arglist);
  if (!result || PyErr_Occurred()) {
    // an exception occured in the script, display it
    PyErr_Print();
  }

  Py_XDECREF(result);
  Py_DECREF(arglist);
}

}
%}

%ignore ob::OBScreen::clients;
%{
  #include <iterator>
%}
%extend ob::OBScreen {
  OBClient *client(int i) {
    if (i >= (int)self->clients.size())
      return NULL;
    ob::OBClient::List::iterator it = self->clients.begin();
    std::advance(it,i);
    return *it;
  }
  int clientCount() const {
    return (int) self->clients.size();
  }
};

%include "../otk/otk.i"

%import "widgetbase.hh"
%import "actions.hh"

%include "openbox.hh"
%include "screen.hh"
%include "client.hh"
%include "python.hh"

// for Mod1Mask etc
%include "X11/X.h"
