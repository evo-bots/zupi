package brain

import (
	"fmt"
	"net"
	"os"
	"strconv"
	"time"

	log "github.com/mgutz/logxi/v1"
	tbus "github.com/robotalks/tbus/go/tbus"
)

// Port is listening port
const Port = 6565

type Brain struct {
	logger   log.Logger
	listener net.Listener
	host     *tbus.RemoteDeviceHost
}

func NewBrain() (*Brain, error) {
	listener, err := net.Listen("tcp", ":"+strconv.Itoa(Port))
	if err != nil {
		return nil, err
	}
	host := tbus.NewRemoteDeviceHost(listener)
	return &Brain{logger: log.NewLogger(log.NewConcurrentWriter(os.Stderr), "zpi1.robot.brain"), host: host, listener: listener}, nil
}

func (b *Brain) Run() error {
	b.logger.Info("listening", "addr", b.listener.Addr())
	go b.cast()
	go b.host.Run()
	dev := <-b.host.AcceptChan()
	NewMaster(dev).Run()
	b.listener.Close()
	return nil
}

func (b *Brain) cast() {
	conn, err := net.ListenUDP("udp", &net.UDPAddr{IP: net.IPv4zero, Port: Port})
	if err != nil {
		b.logger.Error("listen broadcast failed", "err", err.Error())
		return
	}
	b.logger.Info("casting", "addr", conn.LocalAddr())
	msg := make([]byte, 4096)
	for {
		n, addr, err := conn.ReadFromUDP(msg)
		if err != nil {
			b.logger.Error("read broadcast failed", "err", err.Error())
			return
		}
		name := string(msg[0:n])
		b.logger.Info("offer", "name", name, "to", addr)
		conn.WriteTo([]byte("brain"), addr)
	}
}

// Device IDs
const (
	DeviceLED    uint32 = 0x01
	DeviceMotorL uint32 = 0x10
	DeviceMotorR uint32 = 0x11
	DeviceServoP uint32 = 0x20
	DeviceServoT uint32 = 0x21
)

// DeviceCtl bits
const (
	CtlBitLED    = 0x01
	CtlBitMotorL = 0x02
	CtlBitMotorR = 0x04
	CtlBitServoP = 0x08
	CtlBitServoT = 0x10

	CtlBits = CtlBitLED | CtlBitMotorL | CtlBitMotorR | CtlBitServoP | CtlBitServoT
)

type Master struct {
	logger    log.Logger
	device    tbus.RemoteDevice
	master    tbus.Master
	busctl    *tbus.BusCtl
	ledctl    *tbus.LEDCtl
	motors    []*tbus.MotorCtl
	servoPan  *tbus.ServoCtl
	servoTilt *tbus.ServoCtl
}

func NewMaster(device tbus.RemoteDevice) *Master {
	m := &Master{
		logger: log.New("master"),
		device: device,
		master: tbus.NewLocalMaster(device),
		motors: make([]*tbus.MotorCtl, 2),
	}
	m.busctl = tbus.NewBusCtl(m.master)
	return m
}

func (m *Master) Run() {
	go m.device.Run()
	err := m.run()
	if err != nil {
		m.logger.Error("run failed", "err", err)
	}
	m.device.Close()
}

func (m *Master) run() error {
	m.logger.Debug("bus enumeration")
	enum, err := m.busctl.Enumerate()
	if err != nil {
		return err
	}
	devices := enum.GetDevices()
	bits := 0
	for _, info := range devices {
		switch info.ClassId {
		case tbus.LEDClassID:
			m.ledctl = tbus.NewLEDCtl(m.master).SetAddress(info.DeviceAddress())
			bits |= CtlBitLED
			m.logger.Info("led", "addr", info.Address)
		case tbus.MotorClassID:
			switch info.DeviceId {
			case DeviceMotorL:
				m.motors[0] = tbus.NewMotorCtl(m.master).SetAddress(info.DeviceAddress())
				bits |= CtlBitMotorL
				m.logger.Info("motorL", "addr", info.Address)
			case DeviceMotorR:
				m.motors[1] = tbus.NewMotorCtl(m.master).SetAddress(info.DeviceAddress())
				bits |= CtlBitMotorR
				m.logger.Info("motorR", "addr", info.Address)
			}
		case tbus.ServoClassID:
			switch info.DeviceId {
			case DeviceServoP:
				m.servoPan = tbus.NewServoCtl(m.master).SetAddress(info.DeviceAddress())
				bits |= CtlBitServoP
				m.logger.Info("servoP", "addr", info.Address)
			case DeviceServoT:
				m.servoTilt = tbus.NewServoCtl(m.master).SetAddress(info.DeviceAddress())
				bits |= CtlBitServoT
				m.logger.Info("servoT", "addr", info.Address)
			}
		}
	}
	if bits != CtlBits {
		return fmt.Errorf("device map mismatch %02x (expect %02x)", bits, CtlBits)
	}
	ledOn := true
	for i := 0; i < 10; i++ {
		m.ledctl.SetOn(ledOn)
		time.Sleep(500 * time.Millisecond)
		ledOn = !ledOn
	}
	return nil
}
