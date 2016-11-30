package robot

import (
	"github.com/easeway/langx.go/errors"
	"github.com/hybridgroup/gobot/platforms/gpio"
	"github.com/robotalks/mqhub.go/mqhub"
)

// Motor controls a single motor
type Motor struct {
	Component

	driver *gpio.MotorDriver
	state  mqhub.DataPoint
	speed  mqhub.Reactor
}

// MotorCtl is a message to control the motor
type MotorCtl struct {
	Speed   byte `json:"speed"`
	Reverse bool `json:"reverse"`
}

// NewMotor creates a motor
func NewMotor(b *Board, id string, pin, dirPin string) *Motor {
	m := &Motor{
		driver: gpio.NewMotorDriver(b.MCP(), id, pin),
		state:  mqhub.DataPoint{Name: "state", Retain: true},
		speed:  mqhub.Reactor{Name: "speed"},
	}
	m.driver.DirectionPin = dirPin
	m.driver.CurrentMode = "analog"
	m.speed.DoFunc(m.speedMessage)
	m.Init(b, id, &m.state, &m.speed)
	return m
}

// Start implements Device
func (m *Motor) Start() error {
	return Errs(m.driver.Start())
}

// Stop implements Device
func (m *Motor) Stop() error {
	return Errs(m.driver.Halt())
}

func (m *Motor) speedMessage(msg mqhub.Message) mqhub.Future {
	var ctl MotorCtl
	err := msg.As(&ctl)
	if err == nil {
		switch {
		case ctl.Speed == 0:
			err = m.driver.Off()
		case ctl.Reverse:
			err = m.driver.Backward(ctl.Speed)
		default:
			err = m.driver.Forward(ctl.Speed)
		}
	}
	return &mqhub.ImmediateFuture{Error: err}
}

// Motors controls left/right motors
type Motors struct {
	Component
	mqhub.CompositeBase

	L     *Motor
	R     *Motor
	speed mqhub.Reactor
}

// NewMotors creates motors
func NewMotors(b *Board) *Motors {
	m := &Motors{
		L:     NewMotor(b, "L", "9", "7"),
		R:     NewMotor(b, "R", "10", "8"),
		speed: mqhub.Reactor{Name: "speed"},
	}
	m.speed.DoFunc(m.speedMessage)
	m.Init(b, "motors", &m.speed)
	m.AddComponents(m.L, m.R)
	return m
}

// Start implements Device
func (m *Motors) Start() error {
	errs := &errors.AggregatedError{}
	return errs.AddMany(m.L.Start(), m.R.Start()).Aggregate()
}

// Stop implements Device
func (m *Motors) Stop() error {
	errs := &errors.AggregatedError{}
	return errs.AddMany(m.R.Stop(), m.L.Stop()).Aggregate()
}

func (m *Motors) speedMessage(msg mqhub.Message) mqhub.Future {
	m.L.speedMessage(msg)
	m.R.speedMessage(msg)
	return nil
}
