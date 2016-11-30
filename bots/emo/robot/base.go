package robot

import (
	"os"

	"github.com/easeway/langx.go/errors"
	logxi "github.com/mgutz/logxi/v1"
	"github.com/robotalks/mqhub.go/mqhub"
)

var logger = logxi.NewLogger(logxi.NewConcurrentWriter(os.Stderr), "robot")

// Device defines the lifecycle methods of a device
type Device interface {
	Start() error
	Stop() error
}

// Errs converts error slice into aggregated error
func Errs(errs []error) error {
	aggregated := &errors.AggregatedError{}
	return aggregated.AddMany(errs...).Aggregate()
}

// Component is base for all components
type Component struct {
	mqhub.ComponentBase
	Board *Board

	endpoints []mqhub.Endpoint
}

// Init performs initialization
func (c *Component) Init(b *Board, id string, endpoints ...mqhub.Endpoint) {
	c.Board = b
	c.SetID(id)
	c.endpoints = endpoints
}

// Endpoints implements mqhub.Component
func (c *Component) Endpoints() []mqhub.Endpoint {
	return c.endpoints
}

// StreamMessage implements mqhub.Message with raw bytes
type StreamMessage []byte

// Value implements mqhub.Message
func (m StreamMessage) Value() (interface{}, bool) {
	return m, true
}

// IsState implements mqhub.Message
func (m StreamMessage) IsState() bool {
	return false
}

// As implements mqhub.Message
func (m StreamMessage) As(out interface{}) error {
	return nil
}

// Payload implements mqhub.EncodedPayload
func (m StreamMessage) Payload() ([]byte, error) {
	return m, nil
}
