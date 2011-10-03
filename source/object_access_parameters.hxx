/*!
 * \file
 * Defines the interface by which a client of the library can apply per-request (or global)
 * parameters that would constitute configuration. For instance, the number of read replicas
 * required for a successful read is required on a protocol level for most commands, but it
 * is not central to the desired behavior.
 *
 * \author Andres Jaan Tack <ajtack@gmail.com>
 */
#pragma once
#include <cstdint>

//=============================================================================
namespace riak {
//=============================================================================

struct object_access_parameters
{
    // All of the below are named canonically, such that they agree with Basho's
    // API documentation (http://wiki.basho.com/Basic-Riak-API-Operations.html)
    
    /*!
     * \defgroup quorum_controls
     * These options control the consistency requirements of various operations. For all of these
     * parameters, a value greater than the replication factor will lead to errors, as such values
     * are fundamentally unsatisfiable.
     */
    ///@{
    std::uint32_t r;   /*!< (read quorum) how many replicas need to _agree_ when retrieving an object. */
    std::uint32_t pr;  /*!< (primary read quorum) how many primary replicas need to be _available_ when
                       retrieving the object. */
    
    std::uint32_t w;   /*!< (write quorum) how many replicas must confirm writes before returning a
                       successful response. */
    std::uint32_t pw;  /*!< (primary write quorum) As primary read quorum, but for object writes. */
    std::uint32_t dw;  /*!< How many replicas must commit to durable storage to constitute success. */

    std::uint32_t rw;  /*!< how many replicas to delete before returning a successful response. */
    ///@}
    
    bool basic_quorum;  /*!< If true, will return early if an unacceptable number of replicas
                             return in error before a full set of accesses has completed.
                             e.g. with r=1, receiving 2 errors and a success would return an
                             error with basic_quorum=true) */
    bool notfound_ok;   /*!< whether to treat notfounds as successful reads for the purposes of "r" */
    
    /*!
     * \defgroup parameter_amendments
     * These methods return a parameter set that is equivalent to *this with the exception of the
     * indicated value. Such calls may be chained (defaults.with_r(4).with_pr(6)) to specify a
     * group of parameters.
     */
    ///@{
    object_access_parameters with_r  (std::uint32_t) const;
    object_access_parameters with_pr (std::uint32_t) const;
    object_access_parameters with_w  (std::uint32_t) const;
    object_access_parameters with_pw (std::uint32_t) const;
    object_access_parameters with_dw (std::uint32_t) const;
    object_access_parameters with_rw (std::uint32_t) const;
    
    object_access_parameters with_basic_quorum () const;
    object_access_parameters without_basic_quorum () const;
    object_access_parameters with_notfound_ok () const;
    object_access_parameters without_notfound_ok () const;
    ///@}
};

//------------------------------- Here be inline definitions! ---------------------------------

// Generated inline expansions for with_r, with_rw, etc., because these get repetitive.
#ifndef ap_amendment_for_uint
#define ap_amendment_for_uint(variable) \
inline                                                                         \
object_access_parameters object_access_parameters::with_##variable (std::uint32_t new_value) const \
{                                                                              \
    object_access_parameters new_ap(*this);                                    \
    new_ap.variable = new_value;                                               \
    return new_ap;                                                             \
}
#endif

ap_amendment_for_uint(r);
ap_amendment_for_uint(pr);
ap_amendment_for_uint(w);
ap_amendment_for_uint(dw);
ap_amendment_for_uint(pw);
ap_amendment_for_uint(rw);

#undef ap_amendment_for_uint


#ifndef ap_amendment_with
#define ap_amendment_with(variable) \
inline                                                                         \
object_access_parameters object_access_parameters::with_##variable () const    \
{                                                                              \
    object_access_parameters new_ap(*this);                                    \
    new_ap.variable = true;                                                    \
    return new_ap;                                                             \
}
#endif

ap_amendment_with(basic_quorum);
ap_amendment_with(notfound_ok);

#undef ap_amendment_with


#ifndef ap_amendment_without
#define ap_amendment_without(variable) \
inline                                                                         \
object_access_parameters object_access_parameters::without_##variable () const \
{                                                                              \
    object_access_parameters new_ap(*this);                                    \
    new_ap.variable = false;                                                   \
    return new_ap;                                                             \
}
#endif

ap_amendment_without(basic_quorum);
ap_amendment_without(notfound_ok);

#undef ap_amendment_without

//=============================================================================
}   // namespace riak
//=============================================================================
