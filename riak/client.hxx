#pragma once
#include <memory>
#include <riak/message.hxx>
#include <riak/object_access_parameters.hxx>
#include <riak/request_failure_parameters.hxx>
#include <riak/response_handlers.hxx>
#include <riak/sibling_resolution.hxx>
#include <riak/transport.hxx>

namespace boost {
    namespace asio { class io_service; }
}

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
    
    void get_object (const key& bucket, const key& k, get_response_handler);
    void delete_object (const key& bucket, const key& k, delete_response_handler h);

  private:
    transport::delivery_provider deliver_request_;
    sibling_resolution resolve_siblings_;
    const object_access_parameters access_overrides_;
    const request_failure_parameters request_failure_defaults_;
    boost::asio::io_service& ios_;

  protected:
    friend std::shared_ptr<client> make_client (
            transport::delivery_provider,
            sibling_resolution sr,
            boost::asio::io_service&,
            const request_failure_parameters&,
            const object_access_parameters&);
    
    client (transport::delivery_provider& d,
            sibling_resolution sr,
            boost::asio::io_service& ios,
            const request_failure_parameters& = failure_defaults,
            const object_access_parameters& = access_override_defaults);
};

/*!
 * \param dp will be used to deliver requests.
 * \param sr will be applied as a default to all cases of sibling resolution.
 * \param ios will be burdened with query transmission and reception events.
 * \return a new Riak client which is ready to access shared resources.
 */
std::shared_ptr<client> make_client (
        transport::delivery_provider dp,
        sibling_resolution sr,
        boost::asio::io_service& ios,
        const request_failure_parameters& = client::failure_defaults,
        const object_access_parameters& = client::access_override_defaults);

//=============================================================================
}   // namespace riak
//=============================================================================
