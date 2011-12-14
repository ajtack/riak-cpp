#pragma once
#include <boost/optional.hpp>
#include <boost/thread/future.hpp>
#include <memory>
#include <riak/core_types.hxx>
#include <riak/message.hxx>
#include <riak/object_access_parameters.hxx>
#include <riak/riakclient.pb.h>
#include <riak/request_failure_parameters.hxx>
#include <string>

//=============================================================================
namespace riak {
//=============================================================================

class client;

/*!
 * Represents a value stored in Riak at a key in a bucket. Stores and Gets to the Riak server(s)
 * are coordinated via such an object reference.
 */
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
            
    typedef value sibling;
    typedef ::google::protobuf::RepeatedPtrField<sibling> siblings;
    
    /*!
     * Begins an asynchronous read of this value from the store. The retrieved value will not be
     * cached; the only memory of it will be given with the returned future.
     */
    boost::shared_future<boost::optional<siblings>> fetch () const;
    
  protected:
    friend class bucket;
    object (std::shared_ptr<client> c,
            const ::riak::key& bucket,
            const ::riak::key& k,
            const request_failure_parameters& fp,
            const object_access_parameters& p)
      : client_(c),
        bucket_(bucket),
        key_(k),
        default_request_failure_parameters_(fp),
        overridden_access_parameters_(p),
        cache_is_hot_(false)
    {   }
    
  private:
    std::shared_ptr<client> client_;
    const ::riak::key bucket_;
    const ::riak::key key_;
    
    request_failure_parameters default_request_failure_parameters_;
    object_access_parameters overridden_access_parameters_;
    
    mutable boost::mutex mutex_;
    mutable boost::optional<siblings> cached_siblings_;
    mutable boost::optional<std::string> cached_vector_clock_;
    mutable bool cache_is_hot_;
    
    //
    // Internal helpers to reduce code duplication.
    //

    void fetch_to (message::buffering_handler& response_handler) const;
    void put_with_cached_vector_clock (std::shared_ptr<boost::promise<void>>&, const object::value&);

    //
    // Asynchronous handlers for object operations.
    //

    bool on_fetch_response (
            std::function<void()> proceed_with_next_step,
            std::function<void(const std::exception&)> fail,
            const std::error_code&,
            std::size_t,
            const std::string&) const;
    
    bool on_put_response (
            std::shared_ptr<boost::promise<void>>&,
            const std::error_code&,
            std::size_t,
            const std::string&) const;
};

//=============================================================================
}   // namespace riak
//=============================================================================
