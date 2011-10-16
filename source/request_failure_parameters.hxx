#pragma once
#include <chrono>
#include <cstdint>

//=============================================================================
namespace riak {
//=============================================================================

/*!
 * Expresses the set of failures that determines failure speed and behavior for an fpplication.
 * None of these configure the store itself; it is rather what the client expects from that store
 * in terms of responsiveness and failure tolerance.
 */
struct request_failure_parameters
{
    /*! The amount of active (request pending) inactivity permitted before a request is assumed
        to have been lost either by the server or the network layer. Such a timeout counts
        against retries_permitted. */
    std::chrono::milliseconds response_timeout;  
    
    /*! The number of _retries_ (total attempts + 1) to make on a request before failing
        outright. A value greater than zero will allow server-side entropy-reduction techniques
        to improve the long-term success rate of operations. */
    std::size_t retries_permitted;
    
    /*!
     * \defgroup parameter_amendments
     * These methods return a parameter set that is equivalent to *this with the exception of the
     * indicated value. Such calls may be chained (defaults.with_r(4).with_pr(6)) to specify a
     * group of parameters.
     */
    ///@{
    request_failure_parameters with_response_timeout (std::chrono::milliseconds t) const;
    request_failure_parameters with_retries_permitted (std::size_t n) const;
    ///@}
};

//------------------------------- Here be inline definitions! ---------------------------------

inline
request_failure_parameters request_failure_parameters::with_response_timeout (std::chrono::milliseconds new_value) const
{
    request_failure_parameters new_fp(*this);
    new_fp.response_timeout = new_value;
    return new_fp;
}


inline
request_failure_parameters request_failure_parameters::with_retries_permitted (std::size_t new_value) const
{
    request_failure_parameters new_fp(*this);
    new_fp.retries_permitted = new_value;
    return new_fp;
}

//=============================================================================
}   // namespace riak
//=============================================================================
