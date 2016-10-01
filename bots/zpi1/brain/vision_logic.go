package brain

import (
	"bufio"
	"encoding/json"
	"fmt"
	"os"
	"os/exec"
	"time"

	"github.com/evo-bots/zupi/bots/zpi1/brain/zupi"
	log "github.com/mgutz/logxi/v1"
)

// Point is a position
type Point struct {
	X int `json:"x"`
	Y int `json:"y"`
}

// Size is 2d size
type Size struct {
	W int `json:"w"`
	H int `json:"h"`
}

// Rect is a rectangle range
type Rect struct {
	Point
	Size
}

// VisionObject is a detected object
type VisionObject struct {
	Type  string `json:"type"`
	Range Rect   `json:"range"`
}

// VisionEvent is the detection event including the world and objects
type VisionEvent struct {
	Size    Size           `json:"size"`
	Objects []VisionObject `json:"objects"`
}

// VisionLogic applies face/object recognition
type VisionLogic struct {
	logger log.Logger
	ctrl   Controller
	camIf  RtspCamCtrlIf

	subscribers []chan VisionEvent
}

// NewVisionLogic makes a new VisionLogic
func NewVisionLogic(ctrl Controller) (*VisionLogic, error) {
	l := &VisionLogic{ctrl: ctrl, logger: log.New("vision")}
	err := ctrl.QueryInterface(&l.camIf)
	return l, err
}

// Subscribe subscribes vision processing events
func (l *VisionLogic) Subscribe() <-chan VisionEvent {
	ch := make(chan VisionEvent)
	l.subscribers = append(l.subscribers, ch)
	return ch
}

// Run implements ControlLogic
func (l *VisionLogic) Run() error {
	for {
		err := l.analyze()
		if err != nil {
			l.logger.Error("vision analyze error", "err", err)
		}
		time.Sleep(time.Second)
	}
}

func (l *VisionLogic) analyze() error {
	state, err := l.camIf.CameraCtl().SetState(&zupi.CameraState{
		Mode: zupi.CameraState_Video,
	})
	if err != nil || state.Rtsp == nil {
		return err
	}

	cmd := exec.Command("zpi1-vision", fmt.Sprintf("rtsp://%s:%d/unicast", state.Rtsp.Host, state.Rtsp.Port))
	l.logger.Info("vision connect", "url", cmd.Args[1])
	cmd.Env = os.Environ()
	cmd.Stderr = os.Stderr
	reader, err := cmd.StdoutPipe()
	if err != nil {
		return err
	}
	if err = cmd.Start(); err != nil {
		reader.Close()
		return err
	}

	scanner := bufio.NewScanner(reader)
	for scanner.Scan() {
		var event VisionEvent
		if json.Unmarshal([]byte(scanner.Text()), &event) == nil {
			for _, ch := range l.subscribers {
				ch <- event
			}
		}
	}
	return cmd.Wait()
}
