#include <internal/facts/osx/networking_resolver.hpp>
#include <facter/facts/collection.hpp>
#include <facter/facts/fact.hpp>
#include <facter/facts/scalar_value.hpp>
#include <leatherman/execution/execution.hpp>
#include <boost/algorithm/string.hpp>
#include <net/if_dl.h>
#include <net/if.h>

using namespace std;
using namespace leatherman::execution;

namespace facter { namespace facts { namespace osx {

    bool networking_resolver::is_link_address(sockaddr const* addr) const
    {
        return addr && addr->sa_family == AF_LINK;
    }

    uint8_t const* networking_resolver::get_link_address_bytes(sockaddr const* addr) const
    {
        if (!is_link_address(addr)) {
            return nullptr;
        }
        sockaddr_dl const* link_addr = reinterpret_cast<sockaddr_dl const*>(addr);
        if (link_addr->sdl_alen != 6 && link_addr->sdl_alen != 20) {
            return nullptr;
        }
        return reinterpret_cast<uint8_t const*>(LLADDR(link_addr));
    }

    uint8_t networking_resolver::get_link_address_length(sockaddr const* addr) const
    {
        if (!is_link_address(addr)) {
            return 0;
        }
        sockaddr_dl const* link_addr = reinterpret_cast<sockaddr_dl const*>(addr);
        return link_addr->sdl_alen;
    }

    boost::optional<uint64_t> networking_resolver::get_link_mtu(string const& interface, void* data) const
    {
        if (!data) {
            return boost::none;
        }
        return reinterpret_cast<if_data const*>(data)->ifi_mtu;
    }

    string networking_resolver::get_primary_interface() const
    {
        string interface;
        each_line("route", { "-n", "get",  "default" }, [&interface](string& line){
            boost::trim(line);
            if (boost::starts_with(line, "interface: ")) {
                interface = line.substr(11);
                boost::trim(interface);
                return false;
            }
            return true;
        });
        return interface;
    }

    map<string, string> networking_resolver::find_dhcp_servers() const
    {
        // We don't parse dhclient information on OSX
        return map<string, string>();
    }

    string networking_resolver::find_dhcp_server(string const& interface) const
    {
        // Use ipconfig to get the server identifier
        auto exec = execute("ipconfig", { "getoption", interface, "server_identifier" });
        if (!exec.success) {
            return {};
        }
        return exec.output;
    }

}}}  // namespace facter::facts::osx
