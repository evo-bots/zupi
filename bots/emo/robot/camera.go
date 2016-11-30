package robot

import (
	"fmt"

	"github.com/blackjack/webcam"
	"github.com/robotalks/mqhub.go/mqhub"
)

// FourCC is V4L2 fourcc format
type FourCC uint32

// String returns the string represent of fourcc
func (cc FourCC) String() (s string) {
	for i := uint(0); i < 4; i++ {
		s += string(rune((cc >> (i * 8)) & 0xff))
	}
	return
}

// FourCC formats
const (
	FourCCMJPG FourCC = 0x47504a4d
)

// Camera is the camera component
type Camera struct {
	Component

	v4l2dev string
	image   mqhub.DataPoint
	state   CaptureState
	cam     *webcam.Webcam
}

// CaptureState indicates current capturing state
type CaptureState struct {
	Format string `json:"format"`
	Width  int    `json:"width"`
	Height int    `json:"height"`
	FourCC FourCC `json:"-"`
}

// NewCamera creates the camera
func NewCamera(b *Board) *Camera {
	c := &Camera{
		v4l2dev: b.Options.VideoCapDevice,
		image:   mqhub.DataPoint{Name: "still"},
	}
	c.Init(b, "cam0", &c.image)
	return c
}

// Start implements Device
func (c *Camera) Start() error {
	cam, err := webcam.Open(c.v4l2dev)
	if err != nil {
		return err
	}

	f, w, h, err := cam.SetImageFormat(
		webcam.PixelFormat(FourCCMJPG),
		uint32(c.Board.Options.VideoCapWidth),
		uint32(c.Board.Options.VideoCapHeight))
	if FourCC(f) != FourCCMJPG {
		cam.Close()
		return fmt.Errorf("video format %s not supported", FourCCMJPG)
	}

	if err = cam.StartStreaming(); err != nil {
		cam.Close()
		return err
	}

	c.cam = cam
	c.state.FourCC = FourCC(f)
	c.state.Width = int(w)
	c.state.Height = int(h)
	c.state.Format = c.state.FourCC.String()

	go c.streaming(cam)
	return nil
}

// Stop implements Device
func (c *Camera) Stop() error {
	if cam := c.cam; cam != nil {
		c.cam = nil
		cam.Close()
	}
	return nil
}

func (c *Camera) streaming(cam *webcam.Webcam) {
	for {
		err := cam.WaitForFrame(1)
		if err != nil {
			if _, ok := err.(*webcam.Timeout); !ok {
				break
			}
			continue
		}

		frame, _ := cam.ReadFrame()
		if frame == nil {
			continue
		}
		c.image.Update(StreamMessage(frame))
	}
}
