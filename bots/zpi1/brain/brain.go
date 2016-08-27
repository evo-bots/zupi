package brain

import (
	"bufio"
	"encoding/json"
	"fmt"
	"net"
	"os"
	"os/exec"
	"strconv"
	"sync"
	"time"

	zupi "github.com/evo-bots/zupi/bots/zpi1/brain/zupi"
	log "github.com/mgutz/logxi/v1"
	tbus "github.com/robotalks/tbus/go/tbus"
)

// Port is listening port
const Port = 6565

// Brain listens for robot connections
type Brain struct {
	logger   log.Logger
	listener net.Listener
	host     *tbus.RemoteDeviceHost
}

// NewBrain creates a brain instance
func NewBrain() (*Brain, error) {
	listener, err := net.Listen("tcp", ":"+strconv.Itoa(Port))
	if err != nil {
		return nil, err
	}
	host := tbus.NewRemoteDeviceHost(listener)
	return &Brain{logger: log.NewLogger(log.NewConcurrentWriter(os.Stderr), "zpi1.robot.brain"), host: host, listener: listener}, nil
}

// Run starts the listener
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
	DeviceCamera uint32 = 0x100
)

// DeviceCtl bits
const (
	CtlBitLED    = 0x01
	CtlBitMotorL = 0x02
	CtlBitMotorR = 0x04
	CtlBitServoP = 0x08
	CtlBitServoT = 0x10
	CtlBitCamera = 0x20

	CtlBits = CtlBitLED |
		CtlBitMotorL | CtlBitMotorR |
		CtlBitServoP | CtlBitServoT |
		CtlBitCamera
)

// Master manages device controllers
type Master struct {
	logger    log.Logger
	device    tbus.RemoteDevice
	master    tbus.Master
	busctl    *tbus.BusCtl
	ledctl    *tbus.LEDCtl
	motors    []*tbus.MotorCtl
	servoPan  *tbus.ServoCtl
	servoTilt *tbus.ServoCtl
	camera    *zupi.RtspCameraCtl
}

// NewMaster creates a new master
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

// Run starts the communication
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
		case zupi.RtspCameraClassID:
			switch info.DeviceId {
			case DeviceCamera:
				m.camera = zupi.NewRtspCameraCtl(m.master).SetAddress(info.DeviceAddress())
				bits |= CtlBitCamera
				m.logger.Info("camera", "addr", info.Address)
			}
		}
	}
	if bits != CtlBits {
		return fmt.Errorf("device map mismatch %02x (expect %02x)", bits, CtlBits)
	}

	var wg sync.WaitGroup
	wg.Add(2)
	go m.ledBlink(&wg)
	go m.visionAnalyzeLoop(&wg)
	wg.Wait()
	return nil
}

func (m *Master) ledBlink(wg *sync.WaitGroup) {
	ledOn := true
	for {
		m.ledctl.SetOn(ledOn)
		time.Sleep(500 * time.Millisecond)
		ledOn = !ledOn
	}
}

// Point is a position
type Point struct {
	X int `json:"x"`
	Y int `json:"y"`
}

// Size is 2d size
type Size struct {
	W int `json:"w"`
	H int `json:"h"`
}

// Rect is a rectangle range
type Rect struct {
	Point
	Size
}

type visionAnalyticObject struct {
	Type  string `json:"type"`
	Range Rect   `json:"range"`
}

type visionAnalyticEvent struct {
	Size    Size                    `json:"size"`
	Objects []*visionAnalyticObject `json:"objects"`
}

func (m *Master) visionAnalyzeLoop(wg *sync.WaitGroup) {
	for {
		err := m.visionAnalyze()
		if err != nil {
			m.logger.Error("vision analyze error", "err", err)
		}
		time.Sleep(time.Second)
	}
}

func (m *Master) visionAnalyze() error {
	state, err := m.camera.SetState(&zupi.CameraState{
		Mode: zupi.CameraState_Video,
	})
	if err != nil || state.Rtsp == nil {
		return err
	}

	cmd := exec.Command("zpi1-vision", fmt.Sprintf("rtsp://%s:%d/unicast", state.Rtsp.Host, state.Rtsp.Port))
	m.logger.Info("vision connect", "url", cmd.Args[1])
	cmd.Env = os.Environ()
	cmd.Stderr = os.Stderr
	reader, err := cmd.StdoutPipe()
	if err != nil {
		return err
	}
	if err = cmd.Start(); err != nil {
		reader.Close()
		return err
	}

	scanner := bufio.NewScanner(reader)
	for scanner.Scan() {
		var event visionAnalyticEvent
		if json.Unmarshal([]byte(scanner.Text()), &event) == nil {
			m.processObjects(event.Size, event.Objects)
		}
	}
	return cmd.Wait()
}

func (m *Master) processObjects(size Size, objects []*visionAnalyticObject) {
	for _, obj := range objects {
		m.logger.Info("object", "type", obj.Type, "range", obj.Range)
	}
}
