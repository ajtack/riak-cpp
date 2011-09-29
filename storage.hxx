/*!
 * \file
 * Defines a key/value storage class with a hash table backend.
 *
 * \author Andres Jaan Tack <andres.jaan.tack@eesti.ee>
 */
#include <boost/thread/future.hpp>
#include <memory>
#include <stdexcept>

class storage
{
  public:
    typedef std::string key;
    typedef std::string value;
  
    /*! \post the table is empty. */
    storage ();
    
    /*! An exception type indicating that an assigment at a key has failed. */
    class failed_assignment;
    
    /*! A return type for accessors which may be able to respond, "no item here." */
    class optional_value;
    
    /*!
     * Returns the value mapped by the given key. May return a value evaluating to "false", in which case
     * there was no value at this key. The value returned is directly assignable, so storage["foo"] = "bar"
     * constitutes setting the value at "foo" to "bar", irrespective of any previous values present.
     */
    optional_value& operator[] (const key&);
    const optional_value& operator[] (const key&) const;
    
  private:
    class implementation;
    std::unique_ptr<implementation> pimpl_;
};


class storage::optional_value
{
  protected:
    friend class storage;
    
    /*! Builds an optional value that evaluates to False. */
    optional_value (const storage::key& k);
    
  public:
    /*! \returns True iff a value exists here. False means "no value". */
    operator bool () const;
    
    /*! A type which supports asynchronous assignments via std::future. */
    class async_value_proxy;
    
    /*!
     * \pre this object must evaluate to true.
     * \returns The set value made available.
     */
    std::unique_ptr<async_value_proxy> operator* ();
    const storage::value& operator* () const;
    std::unique_ptr<async_value_proxy> operator-> ();
    const storage::value& operator-> () const;
    
  private:
    class implementation;
    std::shared_ptr<implementation> pimpl_;  /*!< The single concrete value referred to by all copies by a particular key. */
};


class storage::optional_value::async_value_proxy
{
  public:
    virtual const storage::value& cached_value () const = 0;

    /*!
     * Sends an asynchronous request to assign to this value.
     * \return a future value which will return "true" to is_ready() when the request has been successfully completed. This future
     *     will raise a failed_assignment exception
     */
    virtual boost::shared_future<std::unique_ptr<async_value_proxy>> operator= (std::string& new_value) = 0;
};


struct storage::failed_assignment
      : public std::exception
{
    failed_assignment (const std::string& key, const std::string& value)
      : key(key),
        value(value)
    {   }
    
    ~failed_assignment () throw()
    {   }
    
    virtual const char* what () const throw();
    
    const std::string key;
    const std::string value;
};
