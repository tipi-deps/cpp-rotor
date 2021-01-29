#pragma once

//
// Copyright (c) 2019-2021 Ivan Baidakou (basiliscos) (the dot dmol at gmail dot com)
//
// Distributed under the MIT Software License
//

#include <string>
#include <system_error>

namespace rotor {

/** \brief fatal error codes in rotor */
enum class error_code_t {
    success = 0,
    cancelled,
    request_timeout,
    supervisor_defined,
    already_registered,
    actor_misconfigured,
    actor_not_linkable,
    already_linked,
    failure_escalation,
    registration_failed,
    discovery_failed,
    unknown_service,
};

/** \brief actor shutdown reasons as error code */
enum class shutdown_code_t {
    normal = 0,
    supervisor_shutdown,
    child_down,
    child_init_failed,
    init_failed,
    link_failed,
    unlink_requested,
};

namespace details {

class error_code_category : public std::error_category {
  public:
    /** error category name */
    virtual const char *name() const noexcept override;

    /** message for error code */
    virtual std::string message(int c) const override;
};

class shutdown_code_category : public std::error_category {
  public:
    /** error category name */
    virtual const char *name() const noexcept override;

    /** message for error code */
    virtual std::string message(int c) const override;
};

} // namespace details

/** \brief returns error code category for `rotor` error codes */
const details::error_code_category &error_code_category();

/** \brief returns error code category for `rotor` shutdown codes */
const details::shutdown_code_category &shutdown_code_category();

/** \brief makes `std::error_code` from rotor error code enumerations */
inline std::error_code make_error_code(const error_code_t e) { return {static_cast<int>(e), error_code_category()}; }

/** \brief makes `std::error_code` from rotor shutdown code enumerations */
inline std::error_code make_error_code(const shutdown_code_t e) {
    return {static_cast<int>(e), shutdown_code_category()};
}

} // namespace rotor

namespace std {
template <> struct is_error_code_enum<rotor::error_code_t> : std::true_type {};
template <> struct is_error_code_enum<rotor::shutdown_code_t> : std::true_type {};
} // namespace std
