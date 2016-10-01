package brain

import "time"

// LiveLEDLogic blinks the LED with fixed interval
type LiveLEDLogic struct {
	Intervals []time.Duration

	ctrl   Controller
	ctrlIf LEDCtrlIf
}

// NewLiveLEDLogic makes a LiveLEDLogic
func NewLiveLEDLogic(ctrl Controller) (*LiveLEDLogic, error) {
	l := &LiveLEDLogic{
		ctrl: ctrl,
		Intervals: []time.Duration{
			100 * time.Millisecond,
			100 * time.Millisecond,
			100 * time.Millisecond,
			100 * time.Millisecond,
			100 * time.Millisecond,
			100 * time.Millisecond,
			100 * time.Millisecond,
			100 * time.Millisecond,
			500 * time.Millisecond,
			100 * time.Millisecond,
		},
	}
	err := ctrl.QueryInterface(&l.ctrlIf)
	return l, err
}

// Run implements ControlLogic
func (l *LiveLEDLogic) Run() error {
	ledOn := true
	i := 0
	for {
		l.ctrlIf.SetOn(ledOn)
		time.Sleep(l.Intervals[i])
		ledOn = !ledOn
		i++
		if i >= len(l.Intervals) {
			i = 0
		}
	}
}
