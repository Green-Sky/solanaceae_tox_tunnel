# Port forwarding in tox
(maybe more generic?)

This specification describes how 2 or more tox clients can setup and run tcp/udp traffic forwarding.
A common usecase for this is securely exposing a local webserver to a friend or forwarding game traffic without setting up a vpn or opening holes into a firewall.

## Tunnel Setup flow

- Alice announces `TunnelInfo` [TunnelID]
- at some point, Bob sends an `InitTunnel` [TunnelID]
- Alice replies with `InitAckTunnel` [TunnelID], (or denies ?)
- Bob replies with `AckTunnel` [TunnelID] and binds a local port and listens
- the tunnel is now setup and primed

## Pipe Creation flow
- Bob gets a local connection or packet from new ip:port (STREAM/DATAGRAM)
- Bob sends `InitPipe` [TunnelID]
- Alice sends `InitAckPipe` [TunnelID,PipeID] , choosing the PipeID (or denies?)
- Bob sends `AckPipe` [PipeID], and starts tx data
- Alice connects/binds (STREAM/DATAGRAM), and starts tx data

`Data` packets now flow in both directions, until either side sends a `Kill` packet.

## Packets

### TunnelInfo Announce
- `TunnelID` (namespace is unique to sender)
- type (STREAM or DATAGRAM)
- name (human readable, might be an application name or other to identify the purpose)
- TODO: suggested port or similar
- keep alive (defaults to true, pretends and queues packets(if reliable) when the tox connection had a hiccup)
- TODO: keep alive for how long? default to 5sec? forever?
- reliable (default is true for stream and flase for datagram, but there are datagram applications that might work better with this)

### InitTunnel
- the `TunnelID` of the offer

### InitAckTunnel
- the `TunnelID` of the init the announcer is acknowlaging

### AckTunnel
- the `TunnelID` of the handshake

### KillTunnel
- the `TunnelID` of the accept/offer/running tunnel
- can be sent by both peers in all situations

### InitPipe
- `TunnelID`

### InitAckPipe
- `TunnelID`
- `PipeID`

### AckPipe
TODO: also tunnel id? is PipeID unique?
- `PipeID`

### KillPipe
TODO: also tunnel id? is PipeID unique?
- `PipeID`
- can be sent by both peers in all situations

### Data
- `PipeID`
- data

### RelData
(Rel -> Reliable)
- `PipeID`
TODO: ack list here? each pkg in tox is expensive. ack bitfield?
- sequence number
- data

### RelDataAck
- `PipeID`
- [sequence numbers]

sequence numbers are 16bit running counters and each side has their own

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
- 12.07.2026 Green-Sky: draft (v0.3)

