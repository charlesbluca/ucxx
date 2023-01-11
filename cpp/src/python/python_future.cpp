/**
 * Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See file LICENSE for terms.
 */
#if UCXX_ENABLE_PYTHON
#include <Python.h>

#include <ucp/api/ucp.h>

#include <ucxx/log.h>
#include <ucxx/python/exception.h>
#include <ucxx/python/python_future.h>

namespace ucxx {

namespace python {

Future::Future(std::shared_ptr<Notifier> notifier) : _notifier(notifier) {}

Future::~Future() { Py_XDECREF(_handle); }

void Future::set(ucs_status_t status)
{
  if (_handle == nullptr) throw std::runtime_error("Invalid object or already released");

  ucxx_trace_req(
    "Future::set() this: %p, _handle: %p, status: %s", this, _handle, ucs_status_string(status));
  if (status == UCS_OK)
    future_set_result(_handle, Py_True);
  else
    future_set_exception(
      _handle, get_python_exception_from_ucs_status(status), ucs_status_string(status));
}

void Future::notify(ucs_status_t status)
{
  if (_handle == nullptr) throw std::runtime_error("Invalid object or already released");

  auto s = shared_from_this();

  ucxx_trace_req("Future::notify() this: %p, shared.get(): %p, handle: %p, notifier: %p",
                 this,
                 s.get(),
                 _handle,
                 _notifier.get());
  _notifier->scheduleFutureNotify(shared_from_this(), status);
}

PyObject* Future::getHandle()
{
  if (_handle == nullptr) throw std::runtime_error("Invalid object or already released");

  return _handle;
}

PyObject* Future::release()
{
  if (_handle == nullptr) throw std::runtime_error("Invalid object or already released");

  return std::exchange(_handle, nullptr);
}

}  // namespace python

}  // namespace ucxx

#endif