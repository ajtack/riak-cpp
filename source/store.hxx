/*!
 * \file
 * Defines a key/value storage class with a hash table backend.
 *
 * \author Andres Jaan Tack <andres.jaan.tack@eesti.ee>
 */
#include <boost/thread/future.hpp>
#include <bucket.hxx>
#include <memory>
#include <object_access_parameters.hxx>
#include <stdexcept>

//=============================================================================
namespace riak {
//=============================================================================

/*!
 * This key value store may be thought of as a simple hash table, only accessible by key, with one primary
 * exception: Assignments to this hash table return immediately, but resolve asynchronously to a future value.
 * You may, for instance, block on a write to this hash table, and thus be sure the value was delivered:
 *
 *    store s;
 *    auto result = s["breakfast"].set("ham and eggs");
 *    result.wait();
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
    /*! A sane set of defaults that should work fine for buckets with N=3. */
    static const object_access_parameters access_defaults;
  
    /*! \post the table is empty. */
    store (const object_access_parameters& = access_defaults);
    ~store ();
    
    /*! Yields the object access defaults with which this store was instantiated. */
    const object_access_parameters& object_access_defaults () const;
    
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
    class implementation;
    std::unique_ptr<implementation> pimpl_;
    const object_access_parameters access_defaults_;
};

//=============================================================================
}   // namespace riak
//=============================================================================
