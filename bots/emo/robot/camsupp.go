package robot

import (
	"github.com/easeway/langx.go/errors"
	"github.com/robotalks/mqhub.go/mqhub"
)

// CamSuppSetting is camera support (pan/tilt) setting
type CamSuppSetting struct {
}

// CameraSupport is camera support component
type CameraSupport struct {
	Component
	mqhub.CompositeBase

	pan  *Servo
	tilt *Servo
}

// NewCameraSupport creates camera support
func NewCameraSupport(b *Board) *CameraSupport {
	s := &CameraSupport{
		pan:  NewServo(b, "pan", 1),
		tilt: NewServo(b, "tilt", 0),
	}
	s.Init(b, "camsupp0")
	s.AddComponents(s.pan, s.tilt)
	return s
}

// Start implements Device
func (s *CameraSupport) Start() error {
	errs := &errors.AggregatedError{}
	return errs.AddMany(s.pan.Start(), s.tilt.Start()).Aggregate()
}

// Stop implements Device
func (s *CameraSupport) Stop() error {
	errs := &errors.AggregatedError{}
	return errs.AddMany(s.tilt.Start(), s.pan.Start()).Aggregate()
}
