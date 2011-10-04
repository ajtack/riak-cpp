/*!
 * \file
 * Implements the core workhorse of the Riak system: an active and asynchronous proxy for individual
 * mapped objects in the key-value store.
 *
 * \author Andres Jaan Tack <ajtack@gmail.com>
 */
#pragma once
#include <boost/optional.hpp>
#include <boost/thread/future.hpp>
#include <core_types.hxx>
#include <object_access_parameters.hxx>
#include <unordered_map>
#include <vector>

//=============================================================================
namespace riak {
//=============================================================================

class store;
class bucket;

class object
{ 
  public:
    /*! Every object body must be convertible to and from a string. */
    struct content
    {
        content (const std::string& v)
          : value(v)
        {   }
        
        struct link {
            boost::optional<std::string> bucket;
            boost::optional<std::string> key;
            boost::optional<std::string> tag;
        };
        
        struct timestamp {
            uint32_t seconds;
            uint32_t microseconds;
        };
        
        std::string value;
        
        boost::optional<std::string> content_type;
        boost::optional<std::string> charset;
        boost::optional<std::string> content_encoding;
        boost::optional<std::string> vtag;
        std::vector<link> links;
        boost::optional<timestamp> last_mod;
        std::vector<std::pair<std::string, std::string>> usermeta;  /*!< Key/Value metadata pairs; null value is allowed. */
    };
          
    /*!
     * \return The time (in milliseconds) required to complete the store.
     */
    boost::shared_future<size_t> put (
            const content& b,
            const boost::optional<std::string>& vector_clock = boost::none );
            // const object_access_parameters& p = store_.object_access_defaults() );
            
    /*!
     * \return The time (in milliseconds) required to complete the store together with the body
     *     found on the store.
     */
    boost::shared_future<std::tuple<size_t, content>> put_returning_body (
            const content& b,
            const boost::optional<std::string>& vector_clock = boost::none );
            // const object_access_parameters& p = store_.object_access_defaults() );
        
    typedef content sibling;
    struct fetched_value {
        boost::optional<std::list<sibling>> siblings;
        boost::optional<std::string> vclock;  /*!< The clock which must be given by subsequent puts
                                                   to resolve siblings. */
        boost::optional<bool> unchanged;      /*!< Whether writes beyond the given vector clock were detected. */
    };
    
    /*!
     * Begins an asynchronous read of this value from the store. The retrieved value will not be
     * cached; the only memory of it will be given with the returned future.
     */
    boost::shared_future<boost::optional<fetched_value>>
    fetch ( const boost::optional<std::string>& vector_clock = boost::optional<std::string>() ) const;
            // const object_access_parameters& p = store_.object_access_defaults() ) const;
    
  protected:
    friend class bucket;
    object (store& s, bucket& b, const key& k)
      : store_(s),
        bucket_(b),
        key_(k)
    {   }
    
  private:
    store& store_;
    bucket& bucket_;
    const key& key_;
    
    typedef std::unordered_map<key, object::content> object_table;
    typedef std::unordered_map<key, object_table> bucket_table;
    static bucket_table buckets;
    static boost::mutex mutex;
    object_table& get_bucket (store& s, const key& k) const;
};

//=============================================================================
}   // namespace riak
//=============================================================================
