#pragma once

#include "envoy/network/proxy_protocol.h"
#include "envoy/network/transport_socket.h"
#include "envoy/stream_info/filter_state.h"

namespace Envoy {
namespace Network {

// A wrapper around another TransportSocketOptions that overrides the ALPN fallback.
class AlpnDecoratingTransportSocketOptions : public TransportSocketOptions {
public:
  AlpnDecoratingTransportSocketOptions(std::vector<std::string>&& alpn,
                                       TransportSocketOptionsConstSharedPtr inner_options,
                                       const Address::InstanceConstSharedPtr local = nullptr,
                                       const Address::InstanceConstSharedPtr remote = nullptr)
      : alpn_fallback_(std::move(alpn)), inner_options_(std::move(inner_options)), local_(local),
        remote_(remote) {}
  // Network::TransportSocketOptions
  const absl::optional<std::string>& serverNameOverride() const override {
    return inner_options_->serverNameOverride();
  }
  const std::vector<std::string>& verifySubjectAltNameListOverride() const override {
    return inner_options_->verifySubjectAltNameListOverride();
  }
  const std::vector<std::string>& applicationProtocolListOverride() const override {
    return inner_options_->applicationProtocolListOverride();
  }
  const std::vector<std::string>& applicationProtocolFallback() const override {
    return alpn_fallback_;
  }
  absl::optional<Network::ProxyProtocolData> proxyProtocolOptions() const override {
    return inner_options_->proxyProtocolOptions();
  }
  void hashKey(std::vector<uint8_t>& key,
               const Network::TransportSocketFactory& factory) const override;
  const Address::InstanceConstSharedPtr getLocal() const override { return local_; };
  const Address::InstanceConstSharedPtr getRemote() const override { return remote_; };

private:
  const std::vector<std::string> alpn_fallback_;
  const TransportSocketOptionsConstSharedPtr inner_options_;
  const Address::InstanceConstSharedPtr local_;
  const Address::InstanceConstSharedPtr remote_;
};

class TransportSocketOptionsImpl : public TransportSocketOptions {
public:
  TransportSocketOptionsImpl(
      absl::string_view override_server_name = "",
      std::vector<std::string>&& override_verify_san_list = {},
      std::vector<std::string>&& override_alpn = {}, std::vector<std::string>&& fallback_alpn = {},
      absl::optional<Network::ProxyProtocolData> proxy_proto_options = absl::nullopt,
      const Address::InstanceConstSharedPtr local = nullptr,
      const Address::InstanceConstSharedPtr remote = nullptr)
      : override_server_name_(override_server_name.empty()
                                  ? absl::nullopt
                                  : absl::optional<std::string>(override_server_name)),
        override_verify_san_list_{std::move(override_verify_san_list)},
        override_alpn_list_{std::move(override_alpn)}, alpn_fallback_{std::move(fallback_alpn)},
        proxy_protocol_options_(proxy_proto_options), local_(local), remote_(remote) {}

  // Network::TransportSocketOptions
  const absl::optional<std::string>& serverNameOverride() const override {
    return override_server_name_;
  }
  const std::vector<std::string>& verifySubjectAltNameListOverride() const override {
    return override_verify_san_list_;
  }
  const std::vector<std::string>& applicationProtocolListOverride() const override {
    return override_alpn_list_;
  }
  const std::vector<std::string>& applicationProtocolFallback() const override {
    return alpn_fallback_;
  }
  absl::optional<Network::ProxyProtocolData> proxyProtocolOptions() const override {
    return proxy_protocol_options_;
  }
  void hashKey(std::vector<uint8_t>& key,
               const Network::TransportSocketFactory& factory) const override;
  const Address::InstanceConstSharedPtr getLocal() const override { return local_; };
  const Address::InstanceConstSharedPtr getRemote() const override { return remote_; };

private:
  const absl::optional<std::string> override_server_name_;
  const std::vector<std::string> override_verify_san_list_;
  const std::vector<std::string> override_alpn_list_;
  const std::vector<std::string> alpn_fallback_;
  const absl::optional<Network::ProxyProtocolData> proxy_protocol_options_;
  const Address::InstanceConstSharedPtr local_;
  const Address::InstanceConstSharedPtr remote_;
};

class TransportSocketOptionsUtility {
public:
  /**
   * Construct TransportSocketOptions from StreamInfo::FilterState, using UpstreamServerName
   * and ApplicationProtocols key in the filter state.
   * @returns TransportSocketOptionsConstSharedPtr a shared pointer to the transport socket options,
   * nullptr if nothing is in the filter state.
   */
  static TransportSocketOptionsConstSharedPtr
  fromFilterState(const StreamInfo::FilterState& stream_info);
};

} // namespace Network
} // namespace Envoy
