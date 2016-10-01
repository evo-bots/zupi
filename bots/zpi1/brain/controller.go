package brain

import (
	"fmt"

	"github.com/evo-bots/zupi/bots/zpi1/brain/zupi"
	"github.com/robotalks/tbus/go/tbus"
)

// Runnable has a Run method
type Runnable interface {
	Run() error
}

// Controller is abstraction of the controlling logic
type Controller interface {
	Runnable
	// Connect conects the controller with target device
	Connect(tbus.Master) error
	// QueryInterface retrieves the controlling interface
	// out must be pointer to sub-interface of ControlInterface
	QueryInterface(out interface{}) error
}

// ControlInterface defines the base of controlling interface
type ControlInterface interface {
}

// ControlLogic is abstaction of certain robot capability
type ControlLogic interface {
	Runnable
}

// The following are commonly used controlling interfaces

// LEDCtrlIf controls LED
type LEDCtrlIf interface {
	ControlInterface
	SetOn(bool) error
}

// RtspCamCtrlIf controls the RTSP camera
type RtspCamCtrlIf interface {
	ControlInterface
	CameraCtl() *zupi.RtspCameraCtl
}

// CamSupportCtrlIf controls 2-axis (pan/tilt) camera support
type CamSupportCtrlIf interface {
	ControlInterface
	HorzServoCtl() *tbus.ServoCtl
	VertServoCtl() *tbus.ServoCtl
}

// Motor2WDCtrlIf controls left/right motors
type Motor2WDCtrlIf interface {
	ControlInterface
	Motors() (left, right *tbus.MotorCtl)
}

var (
	// ErrUnknownControlInterface is the possible return of Controller.QueryInterface
	ErrUnknownControlInterface = fmt.Errorf("unknown control interface")
)

// MustBuildLogic is a helper which panics when logic failed to build
func MustBuildLogic(l ControlLogic, err error) ControlLogic {
	if err != nil {
		panic(err)
	}
	return l
}
