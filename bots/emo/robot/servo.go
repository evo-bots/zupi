package robot

import (
	"github.com/hybridgroup/gobot/platforms/i2c"
	"github.com/robotalks/mqhub.go/mqhub"
)

// Servo represents a servo controller
type Servo struct {
	Component

	driver  *i2c.AdafruitMotorHatDriver
	channel int
	state   mqhub.DataPoint
	angle   mqhub.Reactor
}

// ServoCtl is a message to control a servo
type ServoCtl struct {
	Angle int `json:"angle"`
}

// NewServo creates a new servo controller
func NewServo(b *Board, id string, channel int) *Servo {
	s := &Servo{
		driver:  i2c.NewAdafruitMotorHatDriver(b.Pi(), id),
		channel: channel,
		state:   mqhub.DataPoint{Name: "state", Retain: true},
		angle:   mqhub.Reactor{Name: "angle"},
	}
	s.angle.DoFunc(s.setAngleMessage)
	s.Init(b, id, &s.state, &s.angle)
	return s
}

// Start implements Device
func (s *Servo) Start() error {
	return Errs(s.driver.Start())
}

// Stop implements Device
func (s *Servo) Stop() error {
	return Errs(s.driver.Halt())
}

func (s *Servo) setAngleMessage(msg mqhub.Message) mqhub.Future {
	// TODO
	return nil
}
