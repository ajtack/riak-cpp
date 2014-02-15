#pragma once

#ifdef _WIN32
    // Solves a problem in which BOOST_PP_... is missing in included files.
#   include <boost/preprocessor/repetition/enum_binary_params.hpp>
#endif

#include <boost/log/sources/severity_channel_logger.hpp>
#include <memory>
#include <riak/log.hxx>
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
 * A client is the primary point at which a Riak store is accessed. The targeted endpoint
 * (i.e. the address of the target Riak cluster) is determined by the provided transport.
 * This object rather executes high-level Riak operations and performs sibling resolution
 * steps using that link.
 */
class client
{
  public:
    /*!
     * \param dp will be used to deliver requests.
     * \param sr will be applied as a default to all cases of sibling resolution.
     * \param ios will be burdened with query transmission and reception events.
     * \return a new Riak client which is ready to access the database endpoint targeted by dp.
     */
    client (const transport::delivery_provider&& dp,
            const sibling_resolution&& sr,
            boost::asio::io_service& ios,
            const request_failure_parameters& = failure_defaults,
            const object_access_parameters& = access_override_defaults);

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

    /*! Logs all riak request-related activity (identified by riak::log::channel::core). */
    boost::log::sources::severity_channel_logger_mt<log::severity, log::channel> log_;

  protected:
    class request_runner;
    friend class request_runner;
};

//=============================================================================
}   // namespace riak
//=============================================================================
