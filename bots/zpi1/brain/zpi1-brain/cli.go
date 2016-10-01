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
