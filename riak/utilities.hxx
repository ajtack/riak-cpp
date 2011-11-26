/*!
 * \file
 * Includes simple functions which are shared among other library features.
 */
 #pragma once

//=============================================================================
namespace riak {
//=============================================================================

template <typename T>
void fulfill_promise (std::shared_ptr<boost::promise<T>> p, const T& v)
{
    p->set_value(v);
}


template <typename T>
void fail_promise (std::shared_ptr<boost::promise<T>> p, const std::exception& e)
{
    p->set_exception(boost::copy_exception(e));
}

//=============================================================================
}   // namespace riak
//=============================================================================
