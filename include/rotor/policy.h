#pragma once

//
// Copyright (c) 2019 Ivan Baidakou (basiliscos) (the dot dmol at gmail dot com)
//
// Distributed under the MIT Software License
//

namespace rotor {

enum class unlink_policy_t {
    ignore,
    escalate,
};

/** \brief how to behave on child actor initialization failures */
enum class supervisor_policy_t {
    /** \brief shutdown supervisor (and all its actors) if a child-actor
     * fails during supervisor initialization phase
     */
    shutdown_self = 1,

    /** \brief shutdown a failed child and continue initialization */
    shutdown_failed,
};

} // namespace rotor
