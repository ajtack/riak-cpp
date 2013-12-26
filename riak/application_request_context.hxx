#pragma once
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <riak/compat.hxx>
#include <riak/log.hxx>
#include <riak/object_access_parameters.hxx>
#include <riak/request_failure_parameters.hxx>
#include <riak/transport.hxx>

namespace boost {
    namespace asio { class io_service; }
}

//=============================================================================
namespace riak {
//=============================================================================

/*!
 * Persists all metadata — including unique identifiers, storage access parameters, the
 * transport device in use, or any active statistics measurement devices — which apply to a request
 * as invoked by the application. This metadata is never directly exposed to the application,
 * though it may affect various aspects which are themselves visible.
 */
struct application_request_context
{
    application_request_context (
            const object_access_parameters& oap,
            const request_failure_parameters& rfp,
            transport::delivery_provider& transport,
            boost::asio::io_service& ios);

    ~application_request_context ();
    
    const object_access_parameters access_overrides;
    const request_failure_parameters request_failure_defaults;
    transport::delivery_provider deliver_request;
    boost::asio::io_service& ios;
    boost::uuids::uuid request_id;

    /*!
     * Behaves exactly as boost::log::record_ostream, but guarantees that the record is
     * pushed on termination.
     */
    template <typename Logger>
    class automatic_record_ostream
          : public boost::log::record_ostream
    {
      public:
        automatic_record_ostream (boost::log::record&& record, Logger& logger)
          : record_(std::move(record))
          , stream_(new boost::log::record_ostream(record_))
          , logger_(logger)
        {
            assert(record_);
        }

        automatic_record_ostream (automatic_record_ostream&& other)
          : record_(std::move(other.record_))
          , stream_(std::move(other.stream_))
          , logger_(other.logger_)
        {   }

        ~automatic_record_ostream () RIAK_CPP_NOEXCEPT {
            if (record_)
                logger_.push_record(std::move(record_));
        }

        template <typename T>
        automatic_record_ostream& operator<< (T t) {
            *stream_ << t;
            return *this;
        }

      private:
        boost::log::record record_;
        std::unique_ptr<boost::log::record_ostream> stream_;
        Logger& logger_;
    };

    template <typename Logger>
    automatic_record_ostream<Logger> log (Logger& logger, riak::log::severity severity = riak::log::severity::info)
    {
        automatic_record_ostream<Logger> stream(logger.open_record(), logger);
        stream << boost::log::add_value("Riak/ClientRequestId", request_id);

        return stream;
    }

  private:
    // TODO: Thread-local? Probably so...
    static boost::uuids::random_generator new_uuid;
};

//=============================================================================
}   // namespace riak
//=============================================================================
