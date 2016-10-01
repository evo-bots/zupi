package connector

import (
	"bytes"
	"fmt"
	"html/template"
	"net"
	"os"
	"os/exec"
	"time"

	"github.com/evo-bots/zupi/bots/zpi1/brain"
	log "github.com/mgutz/logxi/v1"
)

var (
	// DefaultInterval is the interval sending broadcast
	DefaultInterval = time.Second

	logger = log.New("brain-connector")
)

// Connector is brain-connector to connect to brain
type Connector struct {
	Name       string
	Interval   time.Duration
	AllowNames []string
	Command    []string

	args []*template.Template
}

// NewConnector creates a connector
func NewConnector(name string, cmds []string) (*Connector, error) {
	c := &Connector{
		Name:       name,
		Interval:   DefaultInterval,
		AllowNames: []string{"*"},
		Command:    cmds,
	}

	c.args = make([]*template.Template, len(c.Command))
	for n, arg := range c.Command {
		t, err := template.New("arg").Parse(arg)
		if err != nil {
			return nil, fmt.Errorf("parse arg[%d] failed: %s: %v", n, arg, err)
		}
		c.args[n] = t
	}

	return c, nil
}

// Run starts the connector
func (c *Connector) Run() error {
	bcastAddr := &net.UDPAddr{IP: net.IPv4bcast, Port: brain.Port}
	conn, err := net.DialUDP("udp", nil, bcastAddr)
	if err != nil {
		return err
	}
	for {
		conn.Write([]byte(c.Name))

		conn.SetReadDeadline(time.Now().Add(c.Interval))
		var data []byte
		n, addr, err := conn.ReadFromUDP(data)
		if err != nil {
			log.Warn("Recv error", "err", err)
			continue
		}

		name := string(data[:n])
		allowed := false
		for _, nm := range c.AllowNames {
			if nm == "*" || nm == name {
				allowed = true
				break
			}
		}

		if !allowed {
			logger.Warn("Reject offer", "name", name, "from", addr)
			continue
		}

		logger.Info("Accept offer", "name", name, "from", addr)

		vars := map[string]interface{}{
			"name":   c.Name,
			"ip":     addr.IP,
			"port":   addr.Port,
			"remote": addr.String,
		}

		args := make([]string, len(c.args))
		for n, t := range c.args {
			var a bytes.Buffer
			err = t.Execute(&a, vars)
			if err != nil {
				logger.Error("Arg error", "at", n, "arg", c.Command[n], "err", err)
				break
			}
			args[n] = a.String()
		}
		if err != nil {
			continue
		}

		logger.Info("Launch Robot", "cmd", args)

		cmd := exec.Command(args[0], args[1:]...)
		cmd.Env = os.Environ()
		cmd.Stdin = os.Stdin
		cmd.Stdout = os.Stdout
		cmd.Stderr = os.Stderr

		err = cmd.Run()
		if err != nil {
			logger.Error("Execution error", "cmd", cmd.Args, "err", err)
		} else {
			logger.Info("Robot terminated")
		}
	}
}
