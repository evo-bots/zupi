package robot

import (
	"github.com/hybridgroup/gobot"
	"github.com/hybridgroup/gobot/platforms/gpio"
	"github.com/robotalks/mqhub.go/mqhub"
)

// Button represents the button on the robot
type Button struct {
	Component

	device  *gpio.ButtonDriver
	state   mqhub.DataPoint
	eventCh chan *gobot.Event
	reverse bool
}

// NewButton creates the button
func NewButton(b *Board) *Button {
	btn := &Button{
		device:  gpio.NewButtonDriver(b.MCP(), "btn0", "12"),
		state:   mqhub.DataPoint{Name: "state", Retain: true},
		reverse: true,
	}
	btn.Init(b, "btn0", &btn.state)
	return btn
}

// Start implements Device
func (b *Button) Start() (err error) {
	if err = Errs(b.device.Start()); err != nil {
		return
	}
	b.eventCh = b.device.Subscribe()
	go b.run(b.eventCh)
	return
}

// Stop implements Device
func (b *Button) Stop() (err error) {
	err = Errs(b.device.Halt())
	if ch := b.eventCh; ch != nil {
		b.eventCh = nil
		close(ch)
	}
	return
}

func (b *Button) run(eventCh chan *gobot.Event) {
	for {
		event, ok := <-eventCh
		if !ok {
			break
		}
		if val, ok := event.Data.(int); ok {
			if val != 0 {
				val = 1
			}
			if b.reverse {
				val = 1 - val
			}
			logger.Debug("Button", "value", val)
			b.state.Update(val)
		}
	}
}
