package connector

import (
	"fmt"
	"net"
	"sync/atomic"
	"time"

	"github.com/evo-bots/zupi/bots/zpi1/brain"
)

const (
	maxDiscoverBuffer = 4096
)

var (
	// DiscoverInterval is the interval sending broadcast
	DiscoverInterval = time.Second
)

// Discoverer discovering offers from brains
type Discoverer struct {
	Connector *Connector
	Interval  time.Duration
	paused    int32
}

// Offer represents a brain ready to connect
type Offer struct {
	Name string `json:"name"`
	Host string `json:"host"`
	Port int    `json:"port"`
}

// Remote returns the remote string
func (o *Offer) Remote() string {
	return fmt.Sprintf("%s:%d", o.Host, o.Port)
}

// String returns the string representative of Offer
func (o *Offer) String() string {
	return o.Name + "@" + o.Remote()
}

// NewDiscoverer creates a discoverer
func NewDiscoverer(conn *Connector) *Discoverer {
	d := &Discoverer{
		Connector: conn,
		Interval:  DiscoverInterval,
	}
	return d
}

// Enable enable/disable broadcasting discovery request
func (d *Discoverer) Enable(en bool) {
	var val int32
	if !en {
		val = 1
	}
	atomic.StoreInt32(&d.paused, val)
}

// Run runs the discoverer
func (d *Discoverer) Run() error {
	conn, err := net.ListenUDP("udp", &net.UDPAddr{})
	if err != nil {
		return err
	}
	go d.recv(conn)
	bcastAddr := &net.UDPAddr{IP: net.IPv4bcast, Port: brain.Port}
	for {
		if atomic.LoadInt32(&d.paused) == 0 {
			conn.WriteTo([]byte(d.Connector.Name), bcastAddr)
		}
		<-time.After(d.Interval)
	}
}

func (d *Discoverer) recv(conn *net.UDPConn) {
	buf := make([]byte, maxDiscoverBuffer)
	for {
		conn.SetReadDeadline(time.Now().Add(d.Interval))
		n, addr, err := conn.ReadFromUDP(buf)
		if err != nil {
			if opErr, ok := err.(*net.OpError); !ok || !opErr.Timeout() {
				logger.Warn("Recv error", "err", err)
			}
			continue
		}
		offer := &Offer{
			Name: string(buf[:n]),
			Host: addr.IP.String(),
			Port: addr.Port,
		}
		logger.Debug("Recv offer", "offer", offer.String())
		d.Connector.Notify(offer)
	}
}
