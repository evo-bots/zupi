package main

import (
	"github.com/codingbrain/clix.go/flag"
)

var cliDef *flag.CliDef

func init() {
	cliDef = &flag.CliDef{
		Cli: &flag.Command{
			Name: "zpi1-brain",
			Desc: "zpi1 robot brain/connector",
			Commands: []*flag.Command{
				&flag.Command{
					Name: "brain",
					Desc: "Run as brain, waiting for robot to connect",
					Options: []*flag.Option{
						&flag.Option{
							Name:    "name",
							Alias:   []string{"N"},
							Desc:    "Brain name",
							Default: "zpi1",
						},
						&flag.Option{
							Name:    "accept",
							Alias:   []string{"a"},
							Desc:    "Name of robot to accept",
							Type:    "string",
							List:    true,
							Default: []string{"zpi1"},
						},
					},
				},
				&flag.Command{
					Name:  "connect",
					Alias: []string{"conn"},
					Desc:  "Connect a brain and host robot program",
					Options: []*flag.Option{
						&flag.Option{
							Name:    "name",
							Alias:   []string{"N"},
							Desc:    "Robot name",
							Default: "zpi1",
						},
						&flag.Option{
							Name:    "port",
							Alias:   []string{"P"},
							Desc:    "Server listening port",
							Type:    "integer",
							Default: 80,
						},
						&flag.Option{
							Name: "www-root",
							Desc: "Root directory containing web content, for development only",
						},
					},
					Arguments: []*flag.Option{
						&flag.Option{
							Name:     "program",
							Desc:     "Robot program",
							Required: true,
							Tags:     map[string]interface{}{"help-var": "PROGRAM"},
						},
					},
				},
				&flag.Command{
					Name:  "version",
					Alias: []string{"ver"},
					Desc:  "Show version",
				},
			},
		},
	}
	cliDef.Normalize()
}
