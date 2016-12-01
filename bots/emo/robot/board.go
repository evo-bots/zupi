package robot

import (
	"github.com/easeway/langx.go/errors"
	"github.com/hybridgroup/gobot/platforms/firmata"
	"github.com/hybridgroup/gobot/platforms/raspi"
	"github.com/robotalks/mqhub.go/mqhub"
)

// Board contains all components in the robot
type Board struct {
	Component
	mqhub.CompositeBase

	Options Options

	LED     *LED
	Btn     *Button
	Motion  *Motors
	Camera  *Camera
	CamSupp *CameraSupport

	arduino     *firmata.FirmataAdaptor
	pi          *raspi.RaspiAdaptor
	runningDevs []Device
}

// Options defined configurations
type Options struct {
	FirmataDevice  string
	VideoCapDevice string
	VideoCapWidth  int
	VideoCapHeight int
}

// NewOptions creates options using default value
func NewOptions() *Options {
	return &Options{
		FirmataDevice:  "/dev/ttyAMA0",
		VideoCapDevice: "/dev/video0",
		VideoCapWidth:  640,
		VideoCapHeight: 480,
	}
}

// NewBoard creates the robot
func NewBoard(options *Options) *Board {
	if options == nil {
		options = NewOptions()
	}
	b := &Board{
		Options: *options,
		arduino: firmata.NewFirmataAdaptor("arduino", options.FirmataDevice),
		pi:      raspi.NewRaspiAdaptor("pi"),
	}
	b.Init(b, "robot")

	b.LED = NewLED(b)
	b.Btn = NewButton(b)
	b.Motion = NewMotors(b)
	b.Camera = NewCamera(b)
	b.CamSupp = NewCameraSupport(b)

	b.AddComponents(
		b.LED,
		b.Btn,
		b.Motion,
		b.Camera,
		b.CamSupp,
	)
	return b
}

// Start implements Device
func (b *Board) Start() error {
	b.runningDevs = nil
	errs := &errors.AggregatedError{}
    logger.Info("Connect", "platform", "arduino")
	errs.Add(Errs(b.arduino.Connect()))
    logger.Info("Connect", "platform", "pi")
	errs.Add(Errs(b.pi.Connect()))
	if errs.Aggregate() == nil {
		for _, comp := range b.Components() {
			if dev, ok := comp.(Device); ok {
				logger.Info("Start", "id", comp.ID())
				if errs.Add(dev.Start()) {
					break
				} else {
					b.runningDevs = append(b.runningDevs, dev)
				}
			}
		}
	}
	return errs.Aggregate()
}

// Stop implements Device
func (b *Board) Stop() error {
	errs := &errors.AggregatedError{}
	n := len(b.runningDevs)
	for i := n - 1; i >= 0; i-- {
		logger.Info("Stop", "id", b.runningDevs[i].(mqhub.Component).ID())
		errs.Add(b.runningDevs[i].Stop())
	}

	errs.Add(Errs(b.arduino.Finalize()))
	errs.Add(Errs(b.pi.Finalize()))

	return errs.Aggregate()
}

// Pi returns connection to host
func (b *Board) Pi() *raspi.RaspiAdaptor {
	return b.pi
}

// MCP returns connection to micro-controller
func (b *Board) MCP() *firmata.FirmataAdaptor {
	return b.arduino
}
