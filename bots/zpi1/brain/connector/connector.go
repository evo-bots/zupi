package connector

import (
	"os"
	"os/signal"

	log "github.com/mgutz/logxi/v1"
)

var (
	logger = log.New("brain-connector")
)

// Runner represents generic runnabler task
type Runner interface {
	Run() error
}

// Connector is brain-connector to connect to brain
type Connector struct {
	Name    string
	Brains  []string
	Allowed []string

	current Offer

	events  chan interface{}
	signals chan os.Signal

	discoverer *Discoverer
	botrunner  *RobotRunner
	server     *Server
}

// Status is current connector status
type Status struct {
	Name    string   `json:"name"`
	Brains  []string `json:"brains,omitempty"`
	Allowed []string `json:"allowed,omitempty"`
	Current *Offer   `json:"current,omitempty"`
}

// NewConnector creates a connector
func NewConnector(name string, port int, webRoot string, cmds []string) (c *Connector, err error) {
	c = &Connector{
		Name:    name,
		Brains:  []string{"zpi1", "zpi1.dev", "zpi1.custom1", "zpi1.custom2"},
		Allowed: []string{"zpi1", "zpi1.dev", "zpi1.custom1", "zpi1.custom2"},
		events:  make(chan interface{}),
		signals: make(chan os.Signal),
	}
	c.discoverer = NewDiscoverer(c)
	if c.botrunner, err = NewRobotRunner(c, cmds); err != nil {
		return nil, err
	}
	if c.server, err = NewServer(c, port, webRoot); err != nil {
		return nil, err
	}
	return c, err
}

// Run starts the connector
func (c *Connector) Run() (err error) {
	c.startRunner(c.discoverer)
	c.startRunner(c.botrunner)
	c.startRunner(c.server)
	signal.Notify(c.signals, os.Interrupt)
	loop := true
	for loop {
		select {
		case event, ok := <-c.events:
			if !ok {
				loop = false
				break
			}
			switch v := event.(type) {
			case *Offer:
				c.handleOffer(v)
			case *RobotExit:
				c.handleBotExit(v)
			case *request:
				c.handleRequest(v)
			case error:
				// runner start error
				err = v
				loop = false
			}
		case <-c.signals:
			loop = false
		}
	}
	c.botrunner.Kill()
	return
}

// Status retrieves the status
func (c *Connector) Status() (s Status) {
	s.Name = c.Name
	s.Brains = c.Brains
	s.Allowed = c.Allowed
	offer := c.current
	if c.running() {
		s.Current = &offer
	}
	return
}

// Configure updates the configuration
func (c *Connector) Configure(brains, allowed []string) error {
	return c.request(func() error {
		if brains != nil {
			c.Brains = brains
		}
		c.Allowed = allowed
		logger.Info("Configure", "brains", c.Brains, "allowed", c.Allowed)
		if c.running() {
			for _, name := range c.Allowed {
				if name == c.current.Name {
					return nil
				}
			}
			logger.Info("Terminate Robot", "reason", "brain no longer allowed")
			c.botrunner.Kill()
		}
		return nil
	})
}

// Notify sends event to connector
func (c *Connector) Notify(event interface{}) {
	c.events <- event
}

func (c *Connector) startRunner(r Runner) {
	go func() {
		err := r.Run()
		if err != nil {
			c.events <- err
		}
	}()
}

func (c *Connector) running() bool {
	return c.current.Name != ""
}

func (c *Connector) handleOffer(offer *Offer) {
	if !c.running() {
		for _, name := range c.Allowed {
			if name == offer.Name {
				logger.Info("Offer Accept", "offer", offer.String())
				err := c.botrunner.Start(*offer)
				if err == nil {
					logger.Info("Robot Started")
					c.current = *offer
					c.discoverer.Enable(false)
				} else {
					logger.Error("Robot Start Error", "err", err)
				}
				break
			}
		}
	}
}

func (c *Connector) handleBotExit(e *RobotExit) {
	logger.Info("Robot Exit", "name", c.current.Name, "err", e.Err)
	c.current.Name = ""
	c.discoverer.Enable(true)
}

func (c *Connector) handleRequest(req *request) {
	req.errCh <- req.fn()
}

type request struct {
	fn    func() error
	errCh chan error
}

func (c *Connector) request(fn func() error) error {
	req := &request{fn: fn, errCh: make(chan error)}
	c.events <- req
	return <-req.errCh
}
