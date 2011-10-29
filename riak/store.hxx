/*!
 * \file
 * Defines a key/value storage class with a hash table backend.
 *
 * \author Andres Jaan Tack <andres.jaan.tack@eesti.ee>
 */
#include <riak/bucket.hxx>
#include <functional>
#include <memory>
#include <riak/object_access_parameters.hxx>
#include <riak/request_failure_parameters.hxx>
#include <riak/request_with_timeout.hxx>

namespace boost {
    namespace asio { class io_service; }
}

//=============================================================================
namespace riak {
//=============================================================================

/*!
 * This key value store may be thought of as a simple hash table, only accessible by key, with one primary
 * exception: Assignments to this hash table return immediately, but resolve asynchronously to a future value.
 * You may, for instance, block on a write to this hash table, and thus be sure the value was delivered:
 *
 *    store s;
 *    auto result = s["breakfast"].set("ham and eggs", [error]{
 *        if (not error)
 *            cerr << "success!";      // Handle events asynchronously by "default"
 *        else
 *            cerr << "failure. :(";
 *    });
 *    result.wait();   // We can be synchronous if we want.
 *    if (result.has_value())
 *        assert(*s["breakfast"] == "ham and eggs")
 *    else
 *        cerr << "Write failed!" << endl;
 *
 * You may also "fire and forget" the write, by eliding everything after set(). A failure will be logged by
 * the implementation, but it will not invalidate future accesses of any keys in s.
 */
class store
{
  public:    
    /*! Defaults that allow total control to the database administrators. */
    static const object_access_parameters access_override_defaults;
    
    /*! A sane set of failure defaults; someone who doesn't care to tune their application won't touch these. */
    static const request_failure_parameters failure_defaults;
  
    /*!
     * \param node_address should provide the location of a Riak node at which requests may be made.
     *     Will be resolved via DNS.
     * \param ios will be burdened with query transmission and reception events.
     * \post A connection to node_address is made eagerly at the given location. The store is ready for
     *     access.
     */
    store ( const std::string& node_address,
            uint16_t port,
            boost::asio::io_service& ios,
            const request_failure_parameters& = failure_defaults,
            const object_access_parameters& = access_override_defaults);
    ~store ();
    
    /*! Yields the object access defaults with which this store was instantiated. */
    const object_access_parameters& object_access_override_defaults () const;
    
    /*!
     * Returns the value mapped by the given key. May return a value evaluating to "false", in which case
     * there was no value at this key. The value returned is directly assignable, so store["foo"] = "bar"
     * constitutes setting the value at "foo" to "bar", irrespective of any previous values present.
     */
          ::riak::bucket bucket (const key& k);
    const ::riak::bucket bucket (const key& k) const     { return const_cast<store*>(this)->bucket(k); }
     
          ::riak::bucket operator[] (const key& k)       { return                           bucket(k); }
    const ::riak::bucket operator[] (const key& k) const { return const_cast<store*>(this)->bucket(k); }
    
  private:
    const object_access_parameters access_overrides_;
    const request_failure_parameters request_failure_defaults_;
    const std::string& node_address_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::io_service& ios_;
    
  protected:
    friend class bucket;
    friend class object;
    typedef request_with_timeout::response_handler response_handler;
    void transmit_request(const std::string& request, response_handler& h, std::chrono::milliseconds timeout);
};

//=============================================================================
}   // namespace riak
//=============================================================================
