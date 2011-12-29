/*!
 * \file
 * Implements the "bucket" concept from a Riak store.
 *
 * \author Andres Jaan Tack <ajtack@gmail.com>
 */
#pragma once
#include <riak/core_types.hxx>
#include <riak/object_access_parameters.hxx>
#include <riak/request_failure_parameters.hxx>
#include <riak/response_handlers.hxx>
#include <riak/sibling_resolution.hxx>
#include <riak/transport.hxx>

//=============================================================================
namespace riak {
//=============================================================================

/*!
 * A remote collection of "objects" indexed by "key". Copying a particular instance does not modify
 * the remote store; this is a _reference_ to that bucket and allows access to underlying data.
 * 
 * As per http://wiki.basho.com/What-is-Riak%3F.html:
 * "Buckets are essentially a flat namespace in Riak and have little significance beyond their
 * ability to allow the same key name to exist in multiple buckets and to provide some per-bucket
 * configurability for things like replication factor and pre/post-commit hooks."
 */
class bucket
{
  public:
    /*! \return the key addressing this bucket. */
    const ::riak::key& name () const { return name_; }
    
  protected:
    friend class client;
    
    /*!
     * \param n is the key which indexed this particular bucket in the cluster.
     * \param p will be used for unmapping operations by default. It will also be given as cconfiguration
     *     overrides for any objects referenced through this bucket.
     */
    bucket (transport::delivery_provider& d,
            const ::riak::key& n,
            sibling_resolution& sr,
            const request_failure_parameters& fp,
            const object_access_parameters& p)
      : deliver_request_(d),
        name_(n),
        resolve_siblings_(sr),
        default_request_failure_parameters_(fp),
        overridden_access_parameters_(p)
    {   }
    
  private:
    transport::delivery_provider deliver_request_;
    const ::riak::key name_;
    sibling_resolution resolve_siblings_;
    request_failure_parameters default_request_failure_parameters_;
    object_access_parameters overridden_access_parameters_;
};

//=============================================================================
}   // namespace riak
//=============================================================================
