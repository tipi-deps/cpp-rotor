#include "catch.hpp"
#include "rotor.hpp"
#include "supervisor_test.h"

namespace r = rotor;
namespace rt = r::test;

struct ping_t{};
struct pong_t{};

struct pinger_t : public r::actor_base_t {
    std::uint32_t ping_sent;
    std::uint32_t pong_received;

  pinger_t(r::supervisor_t &sup) : r::actor_base_t{sup}{
      ping_sent = pong_received = 0;
  }

  void set_ponger_addr(const r::address_ptr_t &addr) { ponger_addr = addr; }

  void on_initialize(r::message_t<r::payload::initialize_actor_t>
                         &msg) noexcept override {
    r::actor_base_t::on_initialize(msg);
    subscribe(&pinger_t::on_pong);
  }

  void on_start(r::message_t<r::payload::start_actor_t> &msg) noexcept override {
      ++ping_sent;
      r::actor_base_t::on_start(msg);
      send<ping_t>(ponger_addr);
  }

  void on_pong(r::message_t<pong_t> &) noexcept {
      ++pong_received;
  }

  r::address_ptr_t ponger_addr;
};

struct ponger_t : public r::actor_base_t {
    std::uint32_t ping_received;
    std::uint32_t pong_sent;

  ponger_t(r::supervisor_t &sup) : r::actor_base_t{sup} {
      ping_received = pong_sent = 0;
  }

  void set_pinger_addr(const r::address_ptr_t &addr) { pinger_addr = addr; }

  void on_initialize(r::message_t<r::payload::initialize_actor_t>
                         &msg) noexcept override {
    r::actor_base_t::on_initialize(msg);
    subscribe(&ponger_t::on_ping);
  }

  void on_ping(r::message_t<ping_t> &) noexcept {
    ++ping_received;
    send<pong_t>(pinger_addr);
    ++pong_sent;
  }

private:
  r::address_ptr_t pinger_addr;
};



TEST_CASE("ping-pong", "[supervisor]") {
    r::system_context_t system_context;

    auto sup = system_context.create_supervisor<rt::supervisor_test_t>();
    auto pinger = sup->create_actor<pinger_t>();
    auto ponger = sup->create_actor<ponger_t>();

    pinger->set_ponger_addr(ponger->get_address());
    ponger->set_pinger_addr(pinger->get_address());

    sup->do_start();
    sup->do_process();
    REQUIRE(pinger->ping_sent == 1);
    REQUIRE(pinger->pong_received == 1);
    REQUIRE(ponger->pong_sent == 1);
    REQUIRE(ponger->ping_received == 1);

    sup->do_shutdown();
    sup->do_process();
}

