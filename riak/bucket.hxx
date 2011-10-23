/*!
 * \file
 * Implements the "bucket" concept from a Riak store.
 *
 * \author Andres Jaan Tack <ajtack@gmail.com>
 */
#pragma once
#include <boost/system/error_code.hpp>
#include <boost/thread/future.hpp>
#include <riak/core_types.hxx>
#include <riak/object.hxx>
#include <riak/object_access_parameters.hxx>
#include <riak/request_failure_parameters.hxx>

//=============================================================================
namespace riak {
//=============================================================================

class store;

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
    /*!
     * Generates a reference to an object (possibly as-yet nonexistent) mapped at the given key.
     * With this reference, you can access and make changes to the store itself.
     */
          object::reference operator[] (const key& k);
    const object::reference operator[] (const key& k) const { return const_cast<bucket&>(*this)[k]; }
    
    typedef std::function<void(const boost::system::error_code&, bool)> deletion_result_handler;
    
    /*! Deletes any value mapped at the given key asynchronously, with results given as per the returned future. */
    boost::unique_future<void> unmap (const key& k);
    
    /*!
     * Deletes any value mapped at the given key asynchronously, with the result given to a callback.
     * \param h assuming no error, will be called with the second parameter indicating whether an item was deleted or not.
     */
    void unmap (const key& k, const deletion_result_handler& h);
    
    // boost::shared_future<void> set_properties (const properties& p);
    // boost::shared_future<std::pair<key, std::shared_ptr<properties>>> fetch_properties () const;
    
    const ::riak::key& key () const { return key_; }
    
  protected:
    friend class store;
    friend class object;
    
    /*!
     * \param k is the key which indexed this particular bucket in the store.
     * \param p will be used for unmapping operations by default. It will also be given as cconfiguration
     *     overrides for any objects referenced through this bucket.
     */
    bucket (store& s, const ::riak::key& k, const request_failure_parameters& fp, const object_access_parameters& p)
      : store_(s),
        key_(k),
        default_request_failure_parameters_(fp),
        overridden_access_parameters_(p)
    {   }
    
  private:
    store& store_;
    const ::riak::key key_;
    request_failure_parameters default_request_failure_parameters_;
    object_access_parameters overridden_access_parameters_;
};

//=============================================================================
}   // namespace riak
//=============================================================================
