package robot

import (
	"github.com/hybridgroup/gobot/platforms/gpio"
	"github.com/robotalks/mqhub.go/mqhub"
)

// LED represents the led on the robot
type LED struct {
	Component

	device *gpio.LedDriver
	state  mqhub.DataPoint
	power  mqhub.Reactor
}

// LEDCtl is the message to control LED
type LEDCtl struct {
	On         bool  `json:"on"`
	Brightness *byte `json:"brightness"`
}

// NewLED creates the led
func NewLED(b *Board) *LED {
	led := &LED{
		device: gpio.NewLedDriver(b.MCP(), "led0", "13"),
		state:  mqhub.DataPoint{Name: "state", Retain: true},
		power:  mqhub.Reactor{Name: "power"},
	}
	led.power.DoFunc(led.powerMessage)
	led.Init(b, "led0", &led.state, &led.power)
	return led
}

// Start implement Device
func (led *LED) Start() error {
	return Errs(led.device.Start())
}

// Stop implement Device
func (led *LED) Stop() error {
	return Errs(led.device.Halt())
}

func (led *LED) powerMessage(msg mqhub.Message) mqhub.Future {
	var ctl LEDCtl
	err := msg.As(&ctl)
	if err == nil {
		switch {
		case !ctl.On:
			err = led.device.Off()
		case ctl.Brightness != nil:
			err = led.device.Brightness(*ctl.Brightness)
		default:
			err = led.device.On()
		}
	}
	return &mqhub.ImmediateFuture{Error: err}
}
