#include <riak/bucket.hxx>
#include <memory>
#include <riak/message.hxx>
#include <riak/object_access_parameters.hxx>
#include <riak/request_failure_parameters.hxx>

namespace boost {
    namespace asio { class io_service; }
}
namespace riak { class transport; }

//=============================================================================
namespace riak {
//=============================================================================

/*!
 * A client is the primary point at which a Riak store is accessed. As it is insufficient to
 * allocate such a client on the stack, such an object must be created using riak::make_client.
 */
class client
      : public std::enable_shared_from_this<client>
{
  public:    
    /*! Defaults that allow total control to the database administrators. */
    static const object_access_parameters access_override_defaults;
    
    /*! A sane set of failure defaults; someone who doesn't care to tune their application won't touch these. */
    static const request_failure_parameters failure_defaults;
    
    /*! Yields the object access defaults with which this client was instantiated. */
    const object_access_parameters& object_access_override_defaults () const;
    
    /*!
     * Returns the value mapped by the given key. May return a value evaluating to "false", in which case
     * there was no value at this key. The value returned is directly assignable, so store["foo"] = "bar"
     * constitutes setting the value at "foo" to "bar", irrespective of any previous values present.
     */
          ::riak::bucket bucket (const key& k);
    const ::riak::bucket bucket (const key& k) const     { return const_cast<client*>(this)->bucket(k); }
    
  private:
    transport& transport_;
    const object_access_parameters access_overrides_;
    const request_failure_parameters request_failure_defaults_;
    boost::asio::io_service& ios_;
    
  protected:
    friend class bucket;
    friend class object;

    void transmit_request(const std::string& r, message::buffering_handler& h, std::chrono::milliseconds timeout);

    friend std::shared_ptr<client> make_client (
            transport&,
            boost::asio::io_service&,
            const request_failure_parameters&,
            const object_access_parameters&);
    
    client (transport& t,
            boost::asio::io_service& ios,
            const request_failure_parameters& = failure_defaults,
            const object_access_parameters& = access_override_defaults);
};

/*!
 * \param id should identify an actor in such a way as it is consistently different from other 
 *     actors in the system. This should usually identify a particular server process across
 *     restarts. A single identifier should not identify more than one concurrent process.
 * \param t will be used to deliver requests. It must survive at least as long as this client.
 * \param ios will be burdened with query transmission and reception events.
 * \return a new Riak client which is ready to access shared resources.
 */
std::shared_ptr<client> make_client (
        transport& t,
        boost::asio::io_service& ios,
        const request_failure_parameters& = client::failure_defaults,
        const object_access_parameters& = client::access_override_defaults);

//=============================================================================
}   // namespace riak
//=============================================================================
