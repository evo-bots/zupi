package connector

import (
	"encoding/json"
	"fmt"
	"io"
	"net/http"

	rice "github.com/GeertJohan/go.rice"
)

// Server is API/Web server
type Server struct {
	Connector *Connector
	BindTo    string
	Port      int

	handler http.Handler
}

// NewServer creates a server
func NewServer(conn *Connector, port int, webRoot string) (*Server, error) {
	s := &Server{
		Connector: conn,
		Port:      port,
	}

	var fs http.FileSystem
	if webRoot != "" {
		fs = http.Dir(webRoot)
	} else {
		box, err := rice.FindBox("www")
		if err != nil {
			return nil, err
		}
		fs = box.HTTPBox()
	}

	mux := http.NewServeMux()
	mux.HandleFunc("/connect", s.handleConnect)
	mux.Handle("/", http.FileServer(fs))
	s.handler = mux
	return s, nil
}

// Run implements Runner
func (s *Server) Run() error {
	return http.ListenAndServe(fmt.Sprintf("%s:%d", s.BindTo, s.Port), s.handler)
}

func (s *Server) handleConnect(w http.ResponseWriter, r *http.Request) {
	var result interface{}
	var err error
	switch r.Method {
	case http.MethodGet:
		result, err = s.connectStatus()
	case http.MethodPut:
		result, err = s.connectConfig(r.Body)
	}
	w.Header().Add("Content-type", "application/json")
	if err != nil {
		encoded, _ := json.Marshal(map[string]interface{}{"error": err.Error()})
		w.WriteHeader(http.StatusInternalServerError)
		w.Write(encoded)
	} else if result != nil {
		encoded, _ := json.Marshal(result)
		w.Write(encoded)
	} else {
		w.WriteHeader(http.StatusNoContent)
		w.Write(nil)
	}
}

func (s *Server) connectStatus() (interface{}, error) {
	status := s.Connector.Status()
	return &status, nil
}

func (s *Server) connectConfig(r io.Reader) (interface{}, error) {
	var params Status
	err := json.NewDecoder(r).Decode(&params)
	if err != nil {
		return nil, err
	}
	return nil, s.Connector.Configure(params.Brains, params.Allowed)
}
