package robot

import (
	"github.com/hybridgroup/gobot/platforms/i2c"
	"github.com/robotalks/mqhub.go/mqhub"
)

// Servo represents a servo controller
type Servo struct {
	Component

	driver  *i2c.AdafruitMotorHatDriver
	channel byte
    freq    int
    pulseMin int
    pulseMax int
    angleMin float32
    angleMax float32
	state   mqhub.DataPoint
	angle   mqhub.Reactor
}

// ServoCtl is a message to control a servo
type ServoCtl struct {
	Angle float32 `json:"angle"`
    Pulse *int `json:"pulse"`
}

// NewServo creates a new servo controller
func NewServo(b *Board, id string, channel byte) *Servo {
	s := &Servo{
		driver:  i2c.NewAdafruitMotorHatDriver(b.Pi(), id),
		channel: channel,
        freq: 60,
        pulseMin: 133,
        pulseMax: 513,
        angleMin: 0,
        angleMax: 180,
		state:   mqhub.DataPoint{Name: "state", Retain: true},
		angle:   mqhub.Reactor{Name: "angle"},
	}
	s.angle.DoFunc(s.setAngleMessage)
	s.Init(b, id, &s.state, &s.angle)
	return s
}

// Start implements Device
func (s *Servo) Start() error {
	err := Errs(s.driver.Start())
    if err == nil {
        err = s.driver.SetServoMotorFreq(float64(s.freq))
    }
    if err == nil {
        err = s.driver.SetServoMotorPulse(s.channel, 0, int32(s.pulseMin + s.pulseMax)/2)
    }
    if err == nil {
        s.state.Update(90)
    }
    return err
}

// Stop implements Device
func (s *Servo) Stop() error {
	return Errs(s.driver.Halt())
}

func (s *Servo) setAngleMessage(msg mqhub.Message) mqhub.Future {
    var ctl ServoCtl
	err := msg.As(&ctl)
	if err == nil {
        if ctl.Angle >= s.angleMin && ctl.Angle <= s.angleMax {
            pulse := s.pulseMin + int(float32(s.pulseMax - s.pulseMin) * ctl.Angle / 180)
            logger.Debug("Angle", "id", s.ID(), "angle", ctl.Angle, "pulse", pulse)
            err = s.driver.SetServoMotorPulse(s.channel, 0, int32(pulse))
            if err == nil {
                s.state.Update(ctl.Angle)
            }
        }
        // For debug purpose only
        if ctl.Pulse != nil {
            logger.Debug("Pulse", "id", s.ID(), "pulse", *ctl.Pulse)
            err = s.driver.SetServoMotorPulse(s.channel, 0, int32(*ctl.Pulse))
            if err == nil {
                angle := float32(*ctl.Pulse - s.pulseMin) * 180 / float32(s.pulseMax - s.pulseMin)
                s.state.Update(angle)
            }
        }
	}
	return &mqhub.ImmediateFuture{Error: err}
}
