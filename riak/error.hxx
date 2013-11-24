#pragma once
#include <system_error>
#include <cstdint>

//=============================================================================
namespace riak {
//=============================================================================

struct errc {
    static const errc no_error;
    static const errc response_was_nonsense;
    
    operator std::uint8_t () const { assert(valid_); return value_; }
    
    errc ()
      : valid_(false)
    {   }
    
    errc& operator= (uint8_t val) {
        value_ = val;
        valid_ = true;
        return *this;
    }
    
    explicit errc (uint8_t v)
      : value_(v),
        valid_(true)
    {   }
    
  private:
    uint8_t value_;
    bool valid_;
};

const std::error_category& server_error ();

std::error_code make_server_error(const errc code = errc::no_error);

//=============================================================================
}   // namespace riak
//=============================================================================
