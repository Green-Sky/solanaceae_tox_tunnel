# Port forwarding in tox
(maybe more generic?)

This specification describes how 2 or more tox clients can setup and run tcp/udp traffic forwarding.
A common usecase for this is securely exposing a local webserver to a friend or forwarding game traffic without setting up a vpn or punching holes into a firewall.

## Simple Setup flow

A successful setup is performed, if
- peer `A` sends an `Offer` to `B`,
- `B` sends an `Accept` back
- and `A` Acknowlages with an `Ack-Acc`.

After that, `Data` packets can flow in both directions, until either side sends a `Kill` packet.

There might be an unspecified delay between `Offer` and `Accept`, since the user needs to manually accept, or might just ignore it all together.

For tcp, `Accept` should only be sent after the acceptor has received a local tcp connection.
Otherwise we would have to implement connect and ack twice in the protocol.

Instead of `Accept`, a `Kill` might also be sent instead to tell the `Offer` is no longer valid (from the offerer) or is not wanted right now (offer receiver).

TODO: server config, preconfiguring local client ports

TODO: other tcp control signals?

## Packets

### Offer Port Mapping
- random `msg_id` (namespace is shared between sender and receiver, but not anyone else)
- type (tcp/udp) (can be both)
- suggested port (usually the port the offerer used, 0 for none/random)
- name (human readable, might be an application name or other to identify the purpose)
- reliable

### Accept Port Mapping
- the `msg_id` of the offer
- reliable

### Ack-Acc
- the `msg_id` of the accept/offer
- reliable

### Kill Port Mapping
- the `msg_id` of the accept/offer/running mapping
- reliable

### Data (udp/lossy)
- the `msg_id` of the running mapping
- lossy

Since its for udp, too large packets can be dropped. (no reassembly support for now)

### Data (tcp/lossless)
- the `msg_id` of the running mapping
- reliable

Might need reassambly (TODO)

## Contact Specifics

### ToxFriend
- LossLess pkgid: `167` (`0xA7`)
  - second byte subdivies
- Lossy pkgid: `200` (`0xC8`) (same as simple tox udp tunnel, gonna change proto there)
  - no second byte sub division, since only packet type

TODO: use file streams instead for the actual connection?

### ToxGroup (NGC)
- Offer and Kill can be both private and public messages, everything else is always private
- LossLess pkgid: `0x9000`-`0x9003`
- Lossy pkgid: `0x9000`-`0x9003`

### ToxConference (ToxFriend specialcase)
see ToxFriend

## Changelog

- 08.05.2024 Green-Sky: wrote down inital spec (v0.1)
- 05.12.2025 Green-Sky: draft (v0.2)

