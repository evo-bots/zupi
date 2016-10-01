package brain

import (
	"math"
	"time"

	"github.com/evo-bots/zupi/bots/zpi1/brain/zupi"
	log "github.com/mgutz/logxi/v1"
	"github.com/robotalks/tbus/go/tbus"
)

// CamSupportLogic controls the position of camera
type CamSupportLogic struct {
	ctrl   Controller
	events <-chan VisionEvent
	logger log.Logger

	camera    *zupi.RtspCameraCtl
	horzServo *tbus.ServoCtl
	vertServo *tbus.ServoCtl

	horzAngle int
	vertAngle int

	servoWaitWindow time.Duration
	servoWaitUntil  time.Time
}

// NewCamSupportLogic is logic constructor
func NewCamSupportLogic(ctrl Controller, sub <-chan VisionEvent) (*CamSupportLogic, error) {
	var camIf RtspCamCtrlIf
	var suppIf CamSupportCtrlIf
	err := ctrl.QueryInterface(&camIf)
	if err != nil {
		return nil, err
	}
	err = ctrl.QueryInterface(&suppIf)
	if err != nil {
		return nil, err
	}

	l := &CamSupportLogic{
		ctrl:            ctrl,
		events:          sub,
		logger:          log.New("camsupp"),
		camera:          camIf.CameraCtl(),
		horzServo:       suppIf.HorzServoCtl(),
		vertServo:       suppIf.VertServoCtl(),
		horzAngle:       90, // initial horizontal cam position
		vertAngle:       90, // initial vertical cam position
		servoWaitWindow: 500 * time.Millisecond,
	}

	return l, nil
}

// Run implements ControlLogic
func (l *CamSupportLogic) Run() error {
	camState, err := l.camera.GetState()
	if err != nil {
		return err
	}

	// initialize cam position
	l.horzServo.MoveTo(l.horzAngle)
	l.vertServo.MoveTo(l.vertAngle)

	for {
		event, ok := <-l.events
		if !ok {
			break
		}
		if time.Now().After(l.servoWaitUntil) {
			l.processObjects(camState.GetSpec(), event.Size, event.Objects)
		}
	}

	return nil
}

func (l *CamSupportLogic) processObjects(spec *zupi.CameraSpec, size Size, objects []VisionObject) {
	maxSize := 0
	var maxObj *VisionObject
	for i, obj := range objects {
		if obj.Type != "face" {
			continue
		}
		l.logger.Info("object", "type", obj.Type, "range", obj.Range)
		size := obj.Range.W * obj.Range.H
		if size > maxSize {
			maxObj = &objects[i]
			maxSize = size
		}
	}

	if maxObj != nil && spec != nil && spec.Focal > 0 {
		l.logger.Debug("max object", "range", maxObj.Range, "size", size,
			"sensor-w", spec.SensorWidth, "sensor-h", spec.SensorHeight,
			"focal", spec.Focal)

		midX := float64(maxObj.Range.X+(maxObj.Range.W>>1))*spec.SensorWidth/float64(size.W) - (spec.SensorWidth / 2)
		midY := float64(maxObj.Range.Y+(maxObj.Range.H>>1))*spec.SensorHeight/float64(size.H) - (spec.SensorHeight / 2)
		rotX := -int(math.Ceil(math.Atan2(midX, spec.Focal) / math.Pi * 180))
		rotY := int(math.Ceil(math.Atan2(midY, spec.Focal) / math.Pi * 180))

		l.logger.Debug("rotate", "mid-x", midX, "mid-y", midY, "x", rotX, "y", rotY)
		servoMove := false
		if rotX != 0 {
			if a := updateAngle(l.horzAngle, rotX); a != l.horzAngle {
				l.horzAngle = a
				l.horzServo.MoveTo(a)
				l.logger.Debug("horz servo", "angle", a)
				servoMove = true
			}
		}
		if rotY != 0 {
			if a := updateAngle(l.vertAngle, rotY); a != l.vertAngle {
				l.vertAngle = a
				l.vertServo.MoveTo(a)
				l.logger.Debug("vert servo", "angle", a)
				servoMove = true
			}
		}
		if servoMove {
			l.servoWaitUntil = time.Now().Add(l.servoWaitWindow)
		}
	}

}

func updateAngle(origin, delta int) (a int) {
	a = origin + delta
	if a < 0 {
		a = 0
	}
	if a > 180 {
		a = 180
	}
	return
}
