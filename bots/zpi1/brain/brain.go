package brain

import (
	"math/rand"
	"net"
	"os"
	"path/filepath"
	"strconv"
	"sync"
	"time"

	log "github.com/mgutz/logxi/v1"
	tbus "github.com/robotalks/tbus/go/tbus"
)

// Port is listening port
const Port = 6565

// Brain listens for robot connections
type Brain struct {
	Name   string
	Accept []string

	logger   log.Logger
	listener net.Listener
	host     *tbus.RemoteDeviceHost
}

// NewBrain creates a brain instance
func NewBrain(name string, accept []string) (*Brain, error) {
	listener, err := net.Listen("tcp", ":"+strconv.Itoa(Port))
	if err != nil {
		return nil, err
	}
	host := tbus.NewRemoteDeviceHost(listener)
	return &Brain{
		Name:     name,
		Accept:   accept,
		logger:   log.NewLogger(log.NewConcurrentWriter(os.Stderr), "zpi1.robot.brain"),
		host:     host,
		listener: listener,
	}, nil
}

// Run starts the listener
func (b *Brain) Run() error {
	b.logger.Info("listening", "addr", b.listener.Addr())
	rand.Seed(int64(time.Now().Unix()))
	go b.cast()
	go b.host.Run()
	for {
		dev := <-b.host.AcceptChan()
		ctrl := NewBotCtrl()
		go b.runController(ctrl, dev)
	}
}

func (b *Brain) cast() {
	conn, err := net.ListenUDP("udp", &net.UDPAddr{Port: Port})
	if err != nil {
		b.logger.Error("listen broadcast failed", "err", err)
		return
	}
	b.logger.Info("casting", "addr", conn.LocalAddr())
	msg := make([]byte, 4096)
	for {
		n, addr, err := conn.ReadFromUDP(msg)
		if err != nil {
			b.logger.Error("read broadcast failed", "err", err)
			return
		}
		name := string(msg[0:n])
		accept := false
		for _, a := range b.Accept {
			if accept = a == name; !accept {
				accept, _ = filepath.Match(a, name)
			}
			if accept {
				break
			}
		}
		if accept {
			b.logger.Info("offer", "me", b.Name, "name", name, "to", addr)
			conn.WriteTo([]byte(b.Name), addr)
		}
	}
}

func (b *Brain) runController(c Controller, dev tbus.RemoteDevice) {
	// start communication with device
	go dev.Run()
	defer dev.Close()

	// try to connect to device
	err := c.Connect(tbus.NewLocalMaster(dev))
	if err == nil {
		var wg sync.WaitGroup
		logics := buildLogics(c)
		wg.Add(len(logics))
		for _, logic := range logics {
			go func(l ControlLogic) {
				l.Run()
				wg.Done()
			}(logic)
		}
		err = c.Run()
		if err == nil {
			wg.Wait()
		}
	}
	if err != nil {
		b.logger.Error("controller failure", "err", err)
	}
}
