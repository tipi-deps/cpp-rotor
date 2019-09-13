//
// Copyright (c) 2019 Ivan Baidakou (basiliscos) (the dot dmol at gmail dot com)
//
// Distributed under the MIT Software License
//

#include "rotor.hpp"
#include "rotor/asio.hpp"
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace asio = boost::asio;
namespace pt = boost::posix_time;
namespace ra = rotor::asio;

struct ping_t {};
struct pong_t {};

struct pinger_t : public rotor::actor_base_t {
    using timepoint_t = std::chrono::time_point<std::chrono::high_resolution_clock>;

    pinger_t(rotor::supervisor_t &sup, std::size_t pings)
        : rotor::actor_base_t{sup}, pings_left{pings}, pings_count{pings} {}

    void set_ponger_addr(const rotor::address_ptr_t &addr) { ponger_addr = addr; }

    void init_start() noexcept override {
        std::cout << "pinger_t::init_start\n";
        subscribe(&pinger_t::on_pong);
        rotor::actor_base_t::init_start();
    }

    void on_start(rotor::message_t<rotor::payload::start_actor_t> &) noexcept override {
        std::cout << "pings start (" << pings_left << ")\n";
        start = std::chrono::high_resolution_clock::now();
        send_ping();
    }

    void on_pong(rotor::message_t<pong_t> &) noexcept {
        // std::cout << "pinger_t::on_pong\n";
        send_ping();
    }

  private:
    void send_ping() {
        if (pings_left) {
            send<ping_t>(ponger_addr);
            --pings_left;
        } else {
            using namespace std::chrono;
            auto end = high_resolution_clock::now();
            std::chrono::duration<double> diff = end - start;
            double freq = ((double)pings_count) / diff.count();
            std::cout << "pings finishes (" << pings_left << ") in " << diff.count() << "s"
                      << ", freq = " << std::fixed << std::setprecision(10) << freq << ", real freq = " << std::fixed
                      << std::setprecision(10) << freq * 2 << "\n";
            supervisor.shutdown();
        }
    }

    timepoint_t start;
    rotor::address_ptr_t ponger_addr;
    std::size_t pings_left;
    std::size_t pings_count;
};

struct ponger_t : public rotor::actor_base_t {

    ponger_t(rotor::supervisor_t &sup) : rotor::actor_base_t{sup} {}

    void set_pinger_addr(const rotor::address_ptr_t &addr) { pinger_addr = addr; }

    void init_start() noexcept override {
        std::cout << "pinger_t::init_start\n";
        subscribe(&ponger_t::on_ping);
        rotor::actor_base_t::init_start();
    }

    void on_ping(rotor::message_t<ping_t> &) noexcept { send<pong_t>(pinger_addr); }

  private:
    rotor::address_ptr_t pinger_addr;
};

int main(int argc, char **argv) {

    asio::io_context io_context{1};
    try {
        std::uint32_t count = 10000;
        if (argc > 1) {
            boost::conversion::try_lexical_convert(argv[1], count);
        }

        auto system_context = ra::system_context_asio_t::ptr_t{new ra::system_context_asio_t(io_context)};
        auto stand = std::make_shared<asio::io_context::strand>(io_context);
        auto timeout = boost::posix_time::milliseconds{10};
        ra::supervisor_config_asio_t conf{timeout, std::move(stand)};
        auto supervisor = system_context->create_supervisor<ra::supervisor_asio_t>(conf);

        auto pinger = supervisor->create_actor<pinger_t>(timeout, count);
        auto ponger = supervisor->create_actor<ponger_t>(timeout);
        pinger->set_ponger_addr(ponger->get_address());
        ponger->set_pinger_addr(pinger->get_address());

        supervisor->start();
        io_context.run();
    } catch (const std::exception &ex) {
        std::cout << "exception : " << ex.what();
    }

    std::cout << "exiting...\n";
    return 0;
}
