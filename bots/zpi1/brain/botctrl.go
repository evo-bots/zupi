package brain

import (
	"fmt"

	"github.com/evo-bots/zupi/bots/zpi1/brain/zupi"
	log "github.com/mgutz/logxi/v1"
	"github.com/robotalks/tbus/go/tbus"
)

// Device IDs
const (
	DeviceLED    uint32 = 0x01
	DeviceMotorL uint32 = 0x10
	DeviceMotorR uint32 = 0x11
	DeviceCamP   uint32 = 0x20
	DeviceCamT   uint32 = 0x21
	DeviceCamera uint32 = 0x100
)

// DeviceCtl bits
const (
	CtlBitLED    = 0x01
	CtlBitMotorL = 0x02
	CtlBitMotorR = 0x04
	CtlBitCamP   = 0x08
	CtlBitCamT   = 0x10
	CtlBitCamera = 0x20

	CtlBits = CtlBitLED |
		CtlBitMotorL | CtlBitMotorR |
		CtlBitCamP | CtlBitCamT |
		CtlBitCamera
)

// BotCtrl is zpi1 robot controller implementation
type BotCtrl struct {
	logger log.Logger
	master tbus.Master

	// device controllers
	bus     *tbus.BusCtl
	led     *tbus.LEDCtl
	motors  []*tbus.MotorCtl
	camPan  *tbus.ServoCtl
	camTilt *tbus.ServoCtl
	camera  *zupi.RtspCameraCtl
}

// NewBotCtrl constructs a BotCtrl
func NewBotCtrl() *BotCtrl {
	c := &BotCtrl{
		logger: log.New("botctrl"),
		motors: make([]*tbus.MotorCtl, 2),
	}
	return c
}

// Connect implements Controller
func (c *BotCtrl) Connect(master tbus.Master) error {
	c.master = master
	c.bus = tbus.NewBusCtl(master)

	c.logger.Debug("bus enumeration")
	enum, err := c.bus.Enumerate()
	if err != nil {
		return err
	}

	devices := enum.GetDevices()
	bits := 0
	for _, info := range devices {
		switch info.ClassId {
		case tbus.LEDClassID:
			c.led = tbus.NewLEDCtl(master).SetAddress(info.DeviceAddress())
			bits |= CtlBitLED
			c.logger.Info("led", "addr", info.Address)
		case tbus.MotorClassID:
			switch info.DeviceId {
			case DeviceMotorL:
				c.motors[0] = tbus.NewMotorCtl(master).SetAddress(info.DeviceAddress())
				bits |= CtlBitMotorL
				c.logger.Info("motorL", "addr", info.Address)
			case DeviceMotorR:
				c.motors[1] = tbus.NewMotorCtl(master).SetAddress(info.DeviceAddress())
				bits |= CtlBitMotorR
				c.logger.Info("motorR", "addr", info.Address)
			}
		case tbus.ServoClassID:
			switch info.DeviceId {
			case DeviceCamP:
				c.camPan = tbus.NewServoCtl(master).SetAddress(info.DeviceAddress())
				bits |= CtlBitCamP
				c.logger.Info("camP", "addr", info.Address)
			case DeviceCamT:
				c.camTilt = tbus.NewServoCtl(master).SetAddress(info.DeviceAddress())
				bits |= CtlBitCamT
				c.logger.Info("camT", "addr", info.Address)
			}
		case zupi.RtspCameraClassID:
			switch info.DeviceId {
			case DeviceCamera:
				c.camera = zupi.NewRtspCameraCtl(master).SetAddress(info.DeviceAddress())
				bits |= CtlBitCamera
				c.logger.Info("camera", "addr", info.Address)
			}
		}
	}
	if bits != CtlBits {
		return fmt.Errorf("device map mismatch %02x (expect %02x)", bits, CtlBits)
	}
	return nil
}

// Run implements Controller
func (c *BotCtrl) Run() error {
	return nil
}

// QueryInterface implements Controller
func (c *BotCtrl) QueryInterface(out interface{}) error {
	switch out.(type) {
	case *LEDCtrlIf:
		*out.(*LEDCtrlIf) = c
	case *RtspCamCtrlIf:
		*out.(*RtspCamCtrlIf) = c
	case *CamSupportCtrlIf:
		*out.(*CamSupportCtrlIf) = c
	case *Motor2WDCtrlIf:
		*out.(*Motor2WDCtrlIf) = c
	default:
		return ErrUnknownControlInterface
	}
	return nil
}

// SetOn implements LEDCtrlIf
func (c *BotCtrl) SetOn(on bool) error {
	return c.led.SetOn(on)
}

// CameraCtl implements RtspCameraCtrlIf
func (c *BotCtrl) CameraCtl() *zupi.RtspCameraCtl {
	return c.camera
}

// HorzServoCtl implements CamSupportCtrlIf
func (c *BotCtrl) HorzServoCtl() *tbus.ServoCtl {
	return c.camPan
}

// VertServoCtl implements CamSupportCtrlIf
func (c *BotCtrl) VertServoCtl() *tbus.ServoCtl {
	return c.camTilt
}

// Motors implements Motor2WDCtrlIf
func (c *BotCtrl) Motors() (left, right *tbus.MotorCtl) {
	return c.motors[0], c.motors[1]
}
