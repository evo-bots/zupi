package connector

import (
	"bytes"
	"fmt"
	"os"
	"os/exec"
	"text/template"
	"time"
)

var (
	// ErrOverrun indicate robot is already running
	ErrOverrun = fmt.Errorf("robot already running")
)

// RobotRunner runs robot with external process
type RobotRunner struct {
	Connector *Connector

	args    []*template.Template
	startCh chan startRequest
	exitCh  chan error
	killCh  chan chan bool
	cmd     *exec.Cmd
}

// RobotExit is robot exit event
type RobotExit struct {
	Err error
}

type startRequest struct {
	offer Offer
	errCh chan error
}

// NewRobotRunner creates a robot runner
func NewRobotRunner(conn *Connector, cmds []string) (*RobotRunner, error) {
	r := &RobotRunner{
		Connector: conn,
		startCh:   make(chan startRequest),
		exitCh:    make(chan error),
		killCh:    make(chan chan bool),
	}
	r.args = make([]*template.Template, len(cmds))
	for n, arg := range cmds {
		t, err := template.New("arg").Parse(arg)
		if err != nil {
			return nil, fmt.Errorf("parse arg[%d] failed: %s: %v", n, arg, err)
		}
		r.args[n] = t
	}
	return r, nil
}

// Run implements Runner
func (r *RobotRunner) Run() error {
	for {
		select {
		case req, ok := <-r.startCh:
			if !ok {
				return nil
			}
			req.errCh <- r.robotStart(&req.offer)
		case ch := <-r.killCh:
			ch <- r.robotKill()
		case e := <-r.exitCh:
			r.robotExit(e)
		}
	}
}

// Start starts the robot with offer
func (r *RobotRunner) Start(offer Offer) error {
	req := startRequest{
		offer: offer,
		errCh: make(chan error),
	}
	r.startCh <- req
	return <-req.errCh
}

// Kill terminates current running robot
func (r *RobotRunner) Kill() bool {
	ch := make(chan bool)
	r.killCh <- ch
	return <-ch
}

func (r *RobotRunner) robotKill() bool {
	if r.cmd != nil && r.cmd.Process != nil {
		r.cmd.Process.Signal(os.Interrupt)
		select {
		case <-time.After(5 * time.Second):
			r.cmd.Process.Kill()
			<-r.exitCh
		case <-r.exitCh:
			break
		}
		r.cmd = nil
		return true
	}
	return false
}

func (r *RobotRunner) robotStart(offer *Offer) (err error) {
	if r.cmd != nil {
		return ErrOverrun
	}
	vars := map[string]interface{}{
		"name":   r.Connector.Name,
		"host":   offer.Host,
		"port":   offer.Port,
		"remote": fmt.Sprintf("%s:%d", offer.Host, offer.Port),
	}

	args := make([]string, len(r.args))
	for n, t := range r.args {
		var a bytes.Buffer
		err = t.Execute(&a, vars)
		if err != nil {
			logger.Error("Arg error", "at", n, "err", err)
			break
		}
		args[n] = a.String()
	}
	if err != nil {
		return
	}

	logger.Info("Launch Robot", "cmd", args)

	cmd := exec.Command(args[0], args[1:]...)
	cmd.Env = os.Environ()
	cmd.Stdin = os.Stdin
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr

	err = cmd.Start()
	if err == nil {
		r.cmd = cmd
		go r.watch(cmd)
	}
	return err
}

func (r *RobotRunner) robotExit(err error) {
	r.cmd = nil
	r.Connector.Notify(&RobotExit{Err: err})
}

func (r *RobotRunner) watch(cmd *exec.Cmd) {
	r.exitCh <- cmd.Wait()
}
