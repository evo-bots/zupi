package brain

import (
	"math"
	"math/rand"
	"time"

	"github.com/evo-bots/zupi/bots/zpi1/brain/zupi"
	log "github.com/mgutz/logxi/v1"
	"github.com/robotalks/tbus/go/tbus"
)

const (
	fineFactor      = 0.05
	servoDelay      = 40 * time.Millisecond
	servoShortDelay = 20 * time.Millisecond
	idleWindow      = 10 * time.Second
	idleDelta       = 5 * time.Second
	idleMovesMax    = 50
	idleMovesMin    = 10
	motorSpeed      = 200
	danceDuration   = 50 * time.Millisecond

	typeFace  = "face"
	typeSmile = "smile"
)

type servoRange struct {
	horzMin int
	horzMax int
	vertMin int
	vertMax int
}

// CamSupportLogic controls the position of camera
type CamSupportLogic struct {
	ctrl   Controller
	events <-chan VisionEvent
	logger log.Logger

	camera    *zupi.RtspCameraCtl
	horzServo *tbus.ServoCtl
	vertServo *tbus.ServoCtl
	motorL    *tbus.MotorCtl
	motorR    *tbus.MotorCtl

	horzAngle int
	vertAngle int
	ranges    []servoRange

	servoWaitUntil time.Time
	noCaptureTime  time.Time
	idleTimer      *time.Timer
	idleMoveHorz   int
	idleMoveVert   int
	idleMoves      int
	idleMoveState  int
	dancing        bool
	dancingTimer   *time.Timer
	dancingState   int
}

// NewCamSupportLogic is logic constructor
func NewCamSupportLogic(ctrl Controller, sub <-chan VisionEvent) (*CamSupportLogic, error) {
	var camIf RtspCamCtrlIf
	var suppIf CamSupportCtrlIf
	var motorIf Motor2WDCtrlIf
	err := ctrl.QueryInterface(&camIf)
	if err != nil {
		return nil, err
	}
	if err = ctrl.QueryInterface(&suppIf); err != nil {
		return nil, err
	}
	if err = ctrl.QueryInterface(&motorIf); err != nil {
		return nil, err
	}

	l := &CamSupportLogic{
		ctrl:      ctrl,
		events:    sub,
		logger:    log.New("camsupp"),
		camera:    camIf.CameraCtl(),
		horzServo: suppIf.HorzServoCtl(),
		vertServo: suppIf.VertServoCtl(),
		horzAngle: 90, // initial horizontal cam position
		vertAngle: 90, // initial vertical cam position
		ranges: []servoRange{
			{horzMin: 0, horzMax: 180, vertMin: 30, vertMax: 120},
			{horzMin: 40, horzMax: 140, vertMin: 50, vertMax: 100},
		},
	}
	l.motorL, l.motorR = motorIf.Motors()

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

	l.motorL.Stop()
	l.motorR.Stop()

	l.setIdle()

	loop := true
	for loop {
		if l.dancing {
			l.dance()
		} else {
			loop = l.findFace(camState.GetSpec())
		}

	}

	return nil
}

func (l *CamSupportLogic) dance() {
	switch l.dancingState {
	case 0:
		l.motorL.SetSpeed(motorSpeed)
		l.motorR.SetSpeed(-motorSpeed)
		if l.dancingTimer == nil {
			l.dancingTimer = time.NewTimer(danceDuration)
		} else {
			l.dancingTimer.Reset(danceDuration)
		}
		l.dancing = true
		l.dancingState++
	case 1:
		<-l.dancingTimer.C
		l.motorR.SetSpeed(motorSpeed)
		l.motorL.SetSpeed(-motorSpeed)
		l.dancingTimer.Reset(danceDuration)
		l.dancingState++
	case 2:
		<-l.dancingTimer.C
		l.motorL.Stop()
		l.motorR.Stop()
		l.dancing = false
		l.dancingState = 0
	}
}

func (l *CamSupportLogic) findFace(spec *zupi.CameraSpec) bool {
	select {
	case event, ok := <-l.events:
		if ok {
			l.handleEvent(spec, &event)
		} else {
			return false
		}
	case <-l.idleTimer.C:
		l.idleTimeout()
	}
	return true
}

func (l *CamSupportLogic) handleEvent(spec *zupi.CameraSpec, event *VisionEvent) {
	if time.Now().Before(l.servoWaitUntil) {
		return
	}
	if len(event.Objects) > 0 {
		l.processObjects(spec, event.Size, event.Objects)
		l.setIdle()
	}
}

func (l *CamSupportLogic) processObjects(spec *zupi.CameraSpec, size Size, objects []VisionObject) {
	maxSize := 0
	maxType := ""
	maxIndex := -1
	smile := false
	var maxObj *VisionObject
	for i, obj := range objects {
		l.logger.Info("object", "type", obj.Type, "range", obj.Range)
		if obj.Type == typeSmile {
			if i == maxIndex+1 && maxType == typeFace {
				smile = true
			}
			continue
		}
		if obj.Type != typeFace && maxType == typeFace {
			continue
		}
		size := obj.Range.W * obj.Range.H
		if size > maxSize {
			maxObj = &objects[i]
			maxType = obj.Type
			maxIndex = i
			maxSize = size
			smile = false
		}
	}

	if maxObj != nil && spec != nil && spec.Focal > 0 {
		midX := float64(maxObj.Range.X+(maxObj.Range.W>>1))*spec.SensorWidth/float64(size.W) - (spec.SensorWidth / 2)
		midY := float64(maxObj.Range.Y+(maxObj.Range.H>>1))*spec.SensorHeight/float64(size.H) - (spec.SensorHeight / 2)

		l.logger.Debug("max object", "range", maxObj.Range, "size", size,
			"sensor-w", spec.SensorWidth, "sensor-h", spec.SensorHeight,
			"focal", spec.Focal,
			"midX", midX, "midY", midY)

		deltaHorz := 0
		deltaVert := 0
		if math.Abs(midX) > spec.SensorWidth*fineFactor {
			deltaHorz = 1
			if midX > 0 {
				deltaHorz = -1
			}
		}
		if math.Abs(midY) > spec.SensorHeight*fineFactor {
			deltaVert = 1
			if midY < 0 {
				deltaVert = -1
			}
		}
		moving := l.moveServo(deltaHorz, deltaVert, 0)
		if !moving && smile {
			l.logger.Info("cheers!")
			l.dance()
		}
	}
}

func (l *CamSupportLogic) idleTimeout() {
	if time.Now().After(l.servoWaitUntil) {
		moving := l.idleMoves > 0
		if moving {
			l.idleMoves--
		} else if l.idleMoveState == 0 {
			l.idleMoveHorz = rand.Intn(3) - 1
			l.idleMoveVert = rand.Intn(3) - 1
			l.idleMoveState = 1
		} else {
			l.idleMoveHorz = moveToCenter(l.horzAngle, 90)
			l.idleMoveVert = moveToCenter(l.vertAngle, 90)
			l.idleMoveState = 0
		}
		if !moving && (l.idleMoveHorz != 0 || l.idleMoveVert != 0) {
			l.idleMoves = rand.Intn(idleMovesMax-idleMovesMin) + idleMovesMin
		}
		l.moveServo(l.idleMoveHorz, l.idleMoveVert, 1)
	}

	if l.idleMoves > 0 {
		l.idleTimer.Reset(servoShortDelay)
	} else {
		l.setIdle()
	}
}

func (l *CamSupportLogic) setIdle() {
	d := time.Duration(float64(idleWindow) + (rand.Float64()-0.5)*float64(idleDelta))
	l.logger.Debug("idle duration", "d", d.String())
	if l.idleTimer != nil {
		l.idleTimer.Reset(d)
	} else {
		l.idleTimer = time.NewTimer(d)
	}
	l.idleMoves = 0
	l.idleMoveState = 0
}

func (l *CamSupportLogic) moveServo(deltaHorz, deltaVert, r int) (servoMove bool) {
	if a := updateAngle(l.horzAngle, deltaHorz, l.ranges[r].horzMin, l.ranges[r].horzMax); a != l.horzAngle {
		l.horzAngle = a
		l.horzServo.MoveTo(a)
		l.logger.Debug("horz servo", "angle", a)
		servoMove = true
	}
	if a := updateAngle(l.vertAngle, deltaVert, l.ranges[r].vertMin, l.ranges[r].vertMax); a != l.vertAngle {
		l.vertAngle = a
		l.vertServo.MoveTo(a)
		l.logger.Debug("vert servo", "angle", a)
		servoMove = true
	}
	if servoMove {
		l.servoWaitUntil = time.Now().Add(servoDelay)
	}
	return
}

func moveToCenter(current, mid int) int {
	if current > mid {
		return -1
	} else if current < mid {
		return 1
	}
	return 0
}

func updateAngle(origin, delta, min, max int) (a int) {
	a = origin + delta
	if a < min {
		a = min
	}
	if a > max {
		a = max
	}
	return
}
