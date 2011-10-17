/*!
 * \file
 * Implements the core workhorse of the Riak system: an active and asynchronous proxy for individual
 * mapped objects in the key-value store.
 *
 * \author Andres Jaan Tack <ajtack@gmail.com>
 */
#pragma once
#include <boost/asio/streambuf.hpp>
#include <boost/optional.hpp>
#include <boost/thread/future.hpp>
#include <chrono>
#include <core_types.hxx>
#include <memory>
#include <object_access_parameters.hxx>
#include <riakclient.pb.h>
#include <request_failure_parameters.hxx>
#include <string>
#include <vector>

//=============================================================================
namespace riak {
//=============================================================================

class store;

class object
      : public std::enable_shared_from_this<object>
{ 
  public:
    typedef std::shared_ptr<object> reference;
    typedef ::RpbContent value;
    const ::riak::key& key () const { return key_; }
    
    /*!
     * If this object is cold (never been fetched), performs a fetch and then a store using the vector
     * clock therefrom retrieved. If this object has ever been fetched, that vector clock will be used
     * for the subsequent put.
     * 
     * Be aware that a vector clock is maintained among copies of an object instance, but new instances
     * (gotten from bucket::operator[]) will be cold.
     *
     * \return A future that will "have a value" (that value being still void) when the put query
     *     receives a response.
     */
    boost::shared_future<void> put (const value& b);
            // const object_access_parameters& p = store_.object_access_defaults() );
            
    typedef value sibling;
    typedef ::google::protobuf::RepeatedPtrField<sibling> siblings;
    
    /*!
     * If this object is cold (never been fetched), performs a fetch and then a store using the vector
     * clock therefrom retrieved. If this object has ever been fetched, that vector clock will be used
     * for the subsequent put.
     * 
     * Be aware that a vector clock is maintained among copies of an object instance, but new instances
     * (gotten from bucket::operator[]) will be cold.
     * 
     * \return The body found on the store, as per future semantics.
     */
    boost::shared_future<siblings> put_returning_body (const value& b);
            // const object_access_parameters& p = store_.object_access_defaults() );
    
    /*!
     * Begins an asynchronous read of this value from the store. The retrieved value will not be
     * cached; the only memory of it will be given with the returned future.
     */
    boost::shared_future<boost::optional<siblings>> fetch () const;
            // const object_access_parameters& p = store_.object_access_defaults() ) const;
    
  protected:
    friend class bucket;
    object (store& s, const ::riak::key& bucket, const ::riak::key& k, const request_failure_parameters& fp, const object_access_parameters& p)
      : store_(s),
        bucket_(bucket),
        key_(k),
        default_request_failure_parameters_(fp),
        overridden_access_parameters_(p)
    {   }
    
  private:
    store& store_;
    const ::riak::key& bucket_;
    const ::riak::key& key_;
    
    request_failure_parameters default_request_failure_parameters_;
    object_access_parameters overridden_access_parameters_;
    
    mutable boost::mutex mutex_;
    mutable boost::optional<siblings> cached_siblings_;
    mutable boost::optional<std::string> cached_vector_clock_;
    
    void on_fetch_response (
            std::shared_ptr<boost::promise<boost::optional<object::siblings>>>&,
            const boost::system::error_code&,
            std::size_t,
            boost::asio::streambuf&) const;
};

//=============================================================================
}   // namespace riak
//=============================================================================
